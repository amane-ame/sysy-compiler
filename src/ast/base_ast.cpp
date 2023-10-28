#include <stdexcept>
#include <vector>
#include "../ast.hpp"

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

koopa_raw_type_kind *BaseAST::array_data(std::vector<int> &sz, int pos)
{
    std::vector<koopa_raw_type_kind *> vec;

    for(int i = pos; i < (int)sz.size(); i ++)
        vec.push_back(new koopa_raw_type_kind{.tag = KOOPA_RTT_ARRAY, .data.array.len = (size_t)sz[i]});
    vec.back()->data.array.base = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32};
    for(int i = 0; i < (int)vec.size() - 1; i ++)
        vec[i]->data.array.base = vec[i + 1];

    return vec[0];
}

void *BaseAST::to_koopa(void)
{
    throw std::runtime_error("error: BaseAST cannot to_koopa()");
}

int BaseAST::value(void)
{
    throw std::runtime_error("error: BaseAST cannot value()");
}
