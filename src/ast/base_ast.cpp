#include "../ast.hpp"

void *BaseAST::to_koopa(void)
{
    throw std::runtime_error("error: BaseAST cannot to_koopa()");
}

int BaseAST::value(void)
{
    throw std::runtime_error("error: BaseAST cannot value()");
}
