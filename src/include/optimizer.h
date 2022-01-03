#include "statements.h"

void parse_opt_flag(const char* arg);
void generate_local_vars(Function* func);
void generate_basic_blocks(Function* func);
void remove_unused_blocks(Function* func);
void optimize_ir(Declaration** decls);
