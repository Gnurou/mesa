/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/u_string.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_format.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_parse.h"

#include "freedreno_context.h"
#include "freedreno_util.h"

#include "ir3_shader.h"
#include "ir3_compiler.h"


static void
delete_variant(struct ir3_shader_variant *v)
{
	if (v->ir)
		ir3_destroy(v->ir);
	if (v->bo)
		fd_bo_del(v->bo);
	free(v);
}

/* for vertex shader, the inputs are loaded into registers before the shader
 * is executed, so max_regs from the shader instructions might not properly
 * reflect the # of registers actually used, especially in case passthrough
 * varyings.
 *
 * Likewise, for fragment shader, we can have some regs which are passed
 * input values but never touched by the resulting shader (ie. as result
 * of dead code elimination or simply because we don't know how to turn
 * the reg off.
 */
static void
fixup_regfootprint(struct ir3_shader_variant *v)
{
	if (v->type == SHADER_VERTEX) {
		unsigned i;
		for (i = 0; i < v->inputs_count; i++) {
			/* skip frag inputs fetch via bary.f since their reg's are
			 * not written by gpu before shader starts (and in fact the
			 * regid's might not even be valid)
			 */
			if (v->inputs[i].bary)
				continue;

			if (v->inputs[i].compmask) {
				int32_t regid = (v->inputs[i].regid + 3) >> 2;
				v->info.max_reg = MAX2(v->info.max_reg, regid);
			}
		}
		for (i = 0; i < v->outputs_count; i++) {
			int32_t regid = (v->outputs[i].regid + 3) >> 2;
			v->info.max_reg = MAX2(v->info.max_reg, regid);
		}
	} else if (v->type == SHADER_FRAGMENT) {
		/* NOTE: not sure how to turn pos_regid off..  but this could
		 * be, for example, r1.x while max reg used by the shader is
		 * r0.*, in which case we need to fixup the reg footprint:
		 */
		v->info.max_reg = MAX2(v->info.max_reg, v->pos_regid >> 2);
		if (v->frag_coord)
			debug_assert(v->info.max_reg >= 0); /* hard coded r0.x */
		if (v->frag_face)
			debug_assert(v->info.max_half_reg >= 0); /* hr0.x */
	}
}

/* wrapper for ir3_assemble() which does some info fixup based on
 * shader state.  Non-static since used by ir3_cmdline too.
 */
void * ir3_shader_assemble(struct ir3_shader_variant *v, uint32_t gpu_id)
{
	void *bin;

	bin = ir3_assemble(v->ir, &v->info, gpu_id);
	if (!bin)
		return NULL;

	if (gpu_id >= 400) {
		v->instrlen = v->info.sizedwords / (2 * 16);
	} else {
		v->instrlen = v->info.sizedwords / (2 * 4);
	}

	/* NOTE: if relative addressing is used, we set constlen in
	 * the compiler (to worst-case value) since we don't know in
	 * the assembler what the max addr reg value can be:
	 */
	v->constlen = MIN2(255, MAX2(v->constlen, v->info.max_const + 1));

	fixup_regfootprint(v);

	return bin;
}

static void
assemble_variant(struct ir3_shader_variant *v)
{
	struct fd_context *ctx = fd_context(v->shader->pctx);
	uint32_t gpu_id = v->shader->compiler->gpu_id;
	uint32_t sz, *bin;

	bin = ir3_shader_assemble(v, gpu_id);
	sz = v->info.sizedwords * 4;

	v->bo = fd_bo_new(ctx->dev, sz,
			DRM_FREEDRENO_GEM_CACHE_WCOMBINE |
			DRM_FREEDRENO_GEM_TYPE_KMEM);

	memcpy(fd_bo_map(v->bo), bin, sz);

	if (fd_mesa_debug & FD_DBG_DISASM) {
		struct ir3_shader_key key = v->key;
		DBG("disassemble: type=%d, k={bp=%u,cts=%u,hp=%u}", v->type,
			key.binning_pass, key.color_two_side, key.half_precision);
		ir3_shader_disasm(v, bin);
	}

	free(bin);

	/* no need to keep the ir around beyond this point: */
	ir3_destroy(v->ir);
	v->ir = NULL;
}

static struct ir3_shader_variant *
create_variant(struct ir3_shader *shader, struct ir3_shader_key key)
{
	struct ir3_shader_variant *v = CALLOC_STRUCT(ir3_shader_variant);
	const struct tgsi_token *tokens = shader->tokens;
	int ret;

	if (!v)
		return NULL;

	v->shader = shader;
	v->key = key;
	v->type = shader->type;

	if (fd_mesa_debug & FD_DBG_DISASM) {
		DBG("dump tgsi: type=%d, k={bp=%u,cts=%u,hp=%u}", shader->type,
			key.binning_pass, key.color_two_side, key.half_precision);
		tgsi_dump(tokens, 0);
	}

	ret = ir3_compile_shader_nir(shader->compiler, v, tokens, key);
	if (ret) {
		debug_error("compile failed!");
		goto fail;
	}

	assemble_variant(v);
	if (!v->bo) {
		debug_error("assemble failed!");
		goto fail;
	}

	return v;

fail:
	delete_variant(v);
	return NULL;
}

struct ir3_shader_variant *
ir3_shader_variant(struct ir3_shader *shader, struct ir3_shader_key key)
{
	struct ir3_shader_variant *v;

	/* some shader key values only apply to vertex or frag shader,
	 * so normalize the key to avoid constructing multiple identical
	 * variants:
	 */
	switch (shader->type) {
	case SHADER_FRAGMENT:
	case SHADER_COMPUTE:
		key.binning_pass = false;
		if (key.has_per_samp) {
			key.vsaturate_s = 0;
			key.vsaturate_t = 0;
			key.vsaturate_r = 0;
		}
		break;
	case SHADER_VERTEX:
		key.color_two_side = false;
		key.half_precision = false;
		key.rasterflat = false;
		if (key.has_per_samp) {
			key.fsaturate_s = 0;
			key.fsaturate_t = 0;
			key.fsaturate_r = 0;
		}
		break;
	}

	for (v = shader->variants; v; v = v->next)
		if (ir3_shader_key_equal(&key, &v->key))
			return v;

	/* compile new variant if it doesn't exist already: */
	v = create_variant(shader, key);
	if (v) {
		v->next = shader->variants;
		shader->variants = v;
	}

	return v;
}


void
ir3_shader_destroy(struct ir3_shader *shader)
{
	struct ir3_shader_variant *v, *t;
	for (v = shader->variants; v; ) {
		t = v;
		v = v->next;
		delete_variant(t);
	}
	free((void *)shader->tokens);
	free(shader);
}

struct ir3_shader *
ir3_shader_create(struct pipe_context *pctx, const struct tgsi_token *tokens,
		enum shader_t type)
{
	struct ir3_shader *shader = CALLOC_STRUCT(ir3_shader);
	shader->compiler = fd_context(pctx)->screen->compiler;
	shader->pctx = pctx;
	shader->type = type;
	shader->tokens = tgsi_dup_tokens(tokens);
	return shader;
}

static void dump_reg(const char *name, uint32_t r)
{
	if (r != regid(63,0))
		debug_printf("; %s: r%d.%c\n", name, r >> 2, "xyzw"[r & 0x3]);
}

static void dump_semantic(struct ir3_shader_variant *so,
		unsigned sem, const char *name)
{
	uint32_t regid;
	regid = ir3_find_output_regid(so, ir3_semantic_name(sem, 0));
	dump_reg(name, regid);
}

void
ir3_shader_disasm(struct ir3_shader_variant *so, uint32_t *bin)
{
	struct ir3 *ir = so->ir;
	struct ir3_register *reg;
	const char *type = (so->type == SHADER_VERTEX) ? "VERT" : "FRAG";
	uint8_t regid;
	unsigned i;

	for (i = 0; i < ir->ninputs; i++) {
		if (!ir->inputs[i]) {
			debug_printf("; in%d unused\n", i);
			continue;
		}
		reg = ir->inputs[i]->regs[0];
		regid = reg->num;
		debug_printf("@in(%sr%d.%c)\tin%d\n",
				(reg->flags & IR3_REG_HALF) ? "h" : "",
				(regid >> 2), "xyzw"[regid & 0x3], i);
	}

	for (i = 0; i < ir->noutputs; i++) {
		if (!ir->outputs[i]) {
			debug_printf("; out%d unused\n", i);
			continue;
		}
		/* kill shows up as a virtual output.. skip it! */
		if (is_kill(ir->outputs[i]))
			continue;
		reg = ir->outputs[i]->regs[0];
		regid = reg->num;
		debug_printf("@out(%sr%d.%c)\tout%d\n",
				(reg->flags & IR3_REG_HALF) ? "h" : "",
				(regid >> 2), "xyzw"[regid & 0x3], i);
	}

	for (i = 0; i < so->immediates_count; i++) {
		debug_printf("@const(c%d.x)\t", so->first_immediate + i);
		debug_printf("0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				so->immediates[i].val[0],
				so->immediates[i].val[1],
				so->immediates[i].val[2],
				so->immediates[i].val[3]);
	}

	disasm_a3xx(bin, so->info.sizedwords, 0, so->type);

	debug_printf("; %s: outputs:", type);
	for (i = 0; i < so->outputs_count; i++) {
		uint8_t regid = so->outputs[i].regid;
		ir3_semantic sem = so->outputs[i].semantic;
		debug_printf(" r%d.%c (%u:%u)",
				(regid >> 2), "xyzw"[regid & 0x3],
				sem2name(sem), sem2idx(sem));
	}
	debug_printf("\n");
	debug_printf("; %s: inputs:", type);
	for (i = 0; i < so->inputs_count; i++) {
		uint8_t regid = so->inputs[i].regid;
		ir3_semantic sem = so->inputs[i].semantic;
		debug_printf(" r%d.%c (%u:%u,cm=%x,il=%u,b=%u)",
				(regid >> 2), "xyzw"[regid & 0x3],
				sem2name(sem), sem2idx(sem),
				so->inputs[i].compmask,
				so->inputs[i].inloc,
				so->inputs[i].bary);
	}
	debug_printf("\n");

	/* print generic shader info: */
	debug_printf("; %s: %u instructions, %d half, %d full\n", type,
			so->info.instrs_count,
			so->info.max_half_reg + 1,
			so->info.max_reg + 1);

	/* print shader type specific info: */
	switch (so->type) {
	case SHADER_VERTEX:
		dump_semantic(so, TGSI_SEMANTIC_POSITION, "pos");
		dump_semantic(so, TGSI_SEMANTIC_PSIZE, "psize");
		break;
	case SHADER_FRAGMENT:
		dump_reg("pos (bary)", so->pos_regid);
		dump_semantic(so, TGSI_SEMANTIC_POSITION, "posz");
		dump_semantic(so, TGSI_SEMANTIC_COLOR, "color");
		/* these two are hard-coded since we don't know how to
		 * program them to anything but all 0's...
		 */
		if (so->frag_coord)
			debug_printf("; fragcoord: r0.x\n");
		if (so->frag_face)
			debug_printf("; fragface: hr0.x\n");
		break;
	case SHADER_COMPUTE:
		break;
	}

	debug_printf("\n");
}
