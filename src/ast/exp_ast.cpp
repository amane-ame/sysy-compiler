#include <memory>
#include <string>
#include <vector>
#include "../ast.hpp"

ExpAST::ExpAST(std::unique_ptr<BaseAST> &_unary_exp)
{
    unary_exp = std::move(_unary_exp);

    return;
}

void *ExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    return unary_exp->to_vector(vec, parent);
}


NumberAST::NumberAST(int _val) : val(_val)
{
    return;
}

void *NumberAST::to_koopa(koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = val}};

    return res;
}

void *NumberAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    return this->to_koopa(parent);
}

PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST> &_next_exp)
{
    next_exp = std::move(_next_exp);
}

void *PrimaryExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    return next_exp->to_vector(vec, parent);
}

UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    next_exp = std::move(_primary_exp);

    return;
}
UnaryExpAST::UnaryExpAST(std::string _op, std::unique_ptr<BaseAST> &_unary_exp)
{
    type = OP;
    op = std::string(_op);
    next_exp = std::move(_unary_exp);

    return;
}

void *UnaryExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)next_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "+")
            binary.op = KOOPA_RBO_ADD;
        else if(op == "-")
            binary.op = KOOPA_RBO_SUB;
        else if(op == "!")
            binary.op = KOOPA_RBO_EQ;
        binary.lhs = (koopa_raw_value_t)NumberAST(0).to_vector(vec, child);
        binary.rhs = (koopa_raw_value_t)next_exp->to_vector(vec, child);

        vec.push_back(res);
        break;
    }

    return res;
}

MulExpAST::MulExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
MulExpAST::MulExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *MulExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "*")
            binary.op = KOOPA_RBO_MUL;
        else if(op == "/")
            binary.op = KOOPA_RBO_DIV;
        else if(op == "%")
            binary.op = KOOPA_RBO_MOD;
        binary.lhs = (koopa_raw_value_t)left_exp->to_vector(vec, child);
        binary.rhs = (koopa_raw_value_t)right_exp->to_vector(vec, child);

        vec.push_back(res);
        break;
    }

    return res;
}

AddExpAST::AddExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
AddExpAST::AddExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *AddExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "+")
            binary.op = KOOPA_RBO_ADD;
        else if(op == "-")
            binary.op = KOOPA_RBO_SUB;
        binary.lhs = (koopa_raw_value_t)left_exp->to_vector(vec, child);
        binary.rhs = (koopa_raw_value_t)right_exp->to_vector(vec, child);

        vec.push_back(res);
        break;
    }

    return res;
}

RelExpAST::RelExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
RelExpAST::RelExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *RelExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "<")
            binary.op = KOOPA_RBO_LT;
        else if(op == "<=")
            binary.op = KOOPA_RBO_LE;
        else if(op == ">")
            binary.op = KOOPA_RBO_GT;
        else if(op == ">=")
            binary.op = KOOPA_RBO_GE;
        binary.lhs = (koopa_raw_value_t)left_exp->to_vector(vec, child);
        binary.rhs = (koopa_raw_value_t)right_exp->to_vector(vec, child);

        vec.push_back(res);
        break;
    }

    return res;
}

EqExpAST::EqExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
EqExpAST::EqExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *EqExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "==")
            binary.op = KOOPA_RBO_EQ;
        else if(op == "!=")
            binary.op = KOOPA_RBO_NOT_EQ;
        binary.lhs = (koopa_raw_value_t)left_exp->to_vector(vec, child);
        binary.rhs = (koopa_raw_value_t)right_exp->to_vector(vec, child);

        vec.push_back(res);
        break;
    }

    return res;
}

static void *to_bool(std::vector<void *> &vec, koopa_raw_slice_t parent, koopa_raw_value_t exp)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
    koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

    auto &binary = res->kind.data.binary;
    binary.op = KOOPA_RBO_NOT_EQ;
    binary.lhs = exp;
    binary.rhs = (koopa_raw_value_t)NumberAST(0).to_vector(vec, child);

    vec.push_back(res);
    return res;
}

LAndExpAST::LAndExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
LAndExpAST::LAndExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *LAndExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "&&")
            binary.op = KOOPA_RBO_AND;
        binary.lhs = (koopa_raw_value_t)to_bool(vec, child, (koopa_raw_value_t)left_exp->to_vector(vec, child));
        binary.rhs = (koopa_raw_value_t)to_bool(vec, child, (koopa_raw_value_t)right_exp->to_vector(vec, child));

        vec.push_back(res);
        break;
    }

    return res;
}

LOrExpAST::LOrExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
LOrExpAST::LOrExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp)
{
    type = OP;
    left_exp = std::move(_left_exp);
    op = std::string(_op);
    right_exp = std::move(_right_exp);

    return;
}

void *LOrExpAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_vector(vec, parent);
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_BINARY}};
        koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

        auto &binary = res->kind.data.binary;
        if(op == "||")
            binary.op = KOOPA_RBO_OR;
        binary.lhs = (koopa_raw_value_t)to_bool(vec, child, (koopa_raw_value_t)left_exp->to_vector(vec, child));
        binary.rhs = (koopa_raw_value_t)to_bool(vec, child, (koopa_raw_value_t)right_exp->to_vector(vec, child));

        vec.push_back(res);
        break;
    }

    return res;
}
