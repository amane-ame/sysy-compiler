#include "../ast.hpp"

void *BaseAST::to_koopa(koopa_raw_slice_t parent)
{
    throw std::runtime_error("error: BaseAST cannot to_koopa");
}

void *BaseAST::to_vector(std::vector<void *> &v, koopa_raw_slice_t parent)
{
    throw std::runtime_error("error: BaseAST cannot to_vector");
}

int BaseAST::value(void)
{
    throw std::runtime_error("error: BaseAST cannot value");
}
