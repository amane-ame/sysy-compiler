#include "ast.hpp"

SymbolList BaseAST::symbol_list;
BlockInst BaseAST::block_inst;
std::vector<std::tuple<koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *>> BaseAST::loop_inst;
