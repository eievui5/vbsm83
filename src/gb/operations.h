#pragma once

#include <stdint.h>
#include <stdio.h>

struct CPUReg;
struct CpuOpInfo;

// Constant information describing an operation. This can be used for things
// other than operations, such as jumps, writes, and reads.
typedef struct CpuOp {
    // Width of result in bytes.
    uint8_t result_width;
    // Width of lhs in bytes.
    uint8_t lhs_width;
    // Width of rhs in bytes. Only for binops.
    uint8_t rhs_width;
    // Does this operation accept a constant argument?
    bool is_const;
    // The register which the result of this operation in placed into.
    struct CPUReg* result_reg;
    // A NULL-terminated array of registers which are required by the operation.
    // This is usually the result register.
    struct CPUReg** required_regs;
    // A 0-terminated list of register widths. If any register could be used,
    // this array requests a certain size and expects an appropriate register to
    // be passed using the CpuOpInfo struct.
    const size_t* additional_regs;
    // The size of this operation in bytes.
    uint8_t bytes;
    // The speed of this operation in cycles.
    uint8_t cycles;
    // Compiles a CPU operation according to the operation info it was provided,
    // outputting assembly code.
    void (*compile)(FILE* out, struct CpuOpInfo* info);
} CpuOp;

// Describes how an operation should be compiled, namely the method and
// registers being used.
typedef struct CpuOpInfo {
    const CpuOp* operation;
    // Used as paremeters to the operation, to decide which registers are used.
    const struct CPUReg* registers[];
} CpuOpInfo;

extern const CpuOp* add_operations[];
