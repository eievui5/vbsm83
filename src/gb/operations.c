#include <stdio.h>

#include "statements.h"
#include "operations.h"

/*
 * add a, r8
 */

static CPUReg* add_a_r8_rregs[] = { &a_reg, NULL };
static const size_t add_a_r8_aregs[] = { 1, 0 };
static void compile_add_a_r8(FILE* out, CpuOpInfo* info) {
    fprintf(out, "    add a, %s\n", info->registers[0]->name);
}

static const CpuOp add_a_r8 = {
    .result_width = 1,
    .lhs_width = 1,
    .rhs_width = 1,

    .result_reg = &a_reg,
    .required_regs = add_a_r8_rregs,
    .additional_regs = add_a_r8_aregs,
    .bytes = 1,
    .cycles = 1,
    .compile = &compile_add_a_r8,
};

/*
 * add hl, r16
 */

static CPUReg* add_hl_r16_rregs[] = { &hl_reg, NULL };
static const size_t add_hl_r16_aregs[] = { 1, 0 };
static void compile_add_hl_r16(FILE* out, CpuOpInfo* info) {
    fprintf(out, "    add hl, %s\n", info->registers[0]->name);
}

static const CpuOp add_hl_r16 = {
    .result_width = 2,
    .lhs_width = 2,
    .rhs_width = 2,

    .result_reg = &hl_reg,
    .required_regs = add_hl_r16_rregs,
    .additional_regs = add_hl_r16_aregs,
    .bytes = 1,
    .cycles = 2,
    .compile = &compile_add_hl_r16,
};

/*
 * Operation pools
 */

const CpuOp* add_operations[] = { &add_a_r8, &add_hl_r16, NULL};
