#include "ast.hpp"

SymbolList BaseAST::symbol_list;
BlockInst BaseAST::block_inst;
std::vector<std::tuple<koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *>> BaseAST::loop_inst;

const void **BaseAST::vector_data(std::vector<void *> &vec)
{
    auto buffer = new const void *[vec.size()];
    std::copy(vec.begin(), vec.end(), buffer);

    return buffer;
}

char *BaseAST::string_data(std::string s)
{
    char *res = new char[s.size() + 1];
    res[s.copy(res, s.size())] = 0;

    return res;
}
