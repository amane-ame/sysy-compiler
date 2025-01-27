#include <memory>
#include <string>
#include <vector>
#include "../ast.hpp"

ExpAST::ExpAST(std::unique_ptr<BaseAST> &_unary_exp)
{
    unary_exp = std::move(_unary_exp);

    return;
}

void *ExpAST::to_koopa(void)
{
    return unary_exp->to_koopa();
}

int ExpAST::value(void)
{
    return unary_exp->value();
}


LValAST::LValAST(std::string _ident) : ident(_ident)
{
    type = NUM;

    return;
}

LValAST::LValAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_idx_vec) : ident(_ident)
{
    type = ARRAY;
    for(auto &idx : _idx_vec)
        idx_vec.push_back(std::move(idx));

    return;
}

void *LValAST::left_value(void)
{
    if(type == NUM)
        return symbol_list.get_symbol(ident).number;

    koopa_raw_value_data *get;
    koopa_raw_value_t src = (koopa_raw_value_t)symbol_list.get_symbol(ident).number;

    if(src->ty->data.pointer.base->tag == KOOPA_RTT_POINTER)
    {
        koopa_raw_value_t src = (koopa_raw_value_t)symbol_list.get_symbol(ident).number;
        koopa_raw_value_data *load0 = new koopa_raw_value_data{src->ty->data.pointer.base, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = src}};
        block_inst.add_inst(load0);

        src = load0;
        for(auto &idx : idx_vec)
        {
            if(&idx == idx_vec.data())
                get = new koopa_raw_value_data{src->ty, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_PTR, .data.get_ptr.src = src, .data.get_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
            else
                get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
            block_inst.add_inst(get);
            src = get;
        }
    }
    else
        for(auto &idx : idx_vec)
        {
            get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
            block_inst.add_inst(get);
            src = get;
        }

    return get;
}

void *LValAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data();

    auto var = symbol_list.get_symbol(ident);
    if(var.type == LVal::CONST)
        return (void *)var.number;
    else if(var.type == LVal::VAR)
    {
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = (koopa_raw_value_t)var.number}};
        block_inst.add_inst(res);
    }
    else if(var.type == LVal::ARRAY)
    {
        bool load = false;
        koopa_raw_value_data *get;
        koopa_raw_value_data *src = (koopa_raw_value_data *)var.number;

        if(idx_vec.empty())
        {
            get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = 0}}}};
            block_inst.add_inst(get);
        }
        else
            for(auto &idx : idx_vec)
            {
                if(src->ty->data.pointer.base->data.array.base->tag == KOOPA_RTT_INT32)
                    load = true;
                get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
                block_inst.add_inst(get);
                src = get;
            }
        
        if(load)
        {
            res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = get}};
            block_inst.add_inst(res);
        }
        else if(src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
        {
            res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = 0}}}};
            block_inst.add_inst(res);
        }
        else
            res = src;
    }
    else if(var.type == LVal::POINTER)
    {
        bool load = false;
        koopa_raw_value_data *get;
        koopa_raw_value_data *src = (koopa_raw_value_data*)var.number;

        koopa_raw_value_data *load0 = new koopa_raw_value_data{src->ty->data.pointer.base, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = src}};
        block_inst.add_inst(load0);

        src = load0;
        for(auto &idx : idx_vec)
        {
            if(&idx == idx_vec.data())
                get = new koopa_raw_value_data{src->ty, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_PTR, .data.get_ptr.src = src, .data.get_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
            else
                get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = (koopa_raw_value_t)idx->to_koopa()}};
            block_inst.add_inst(get);

            src = get;
            if(get->ty->data.pointer.base->tag == KOOPA_RTT_INT32)
                load = true;
        }

        if(load)
        {
            res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = get}};
            block_inst.add_inst(res);
        }
        else if(src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
        {
            res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = 0}}}};
            block_inst.add_inst(res);
        }
        else
            res = src;
    }

    return res;
}

int LValAST::value(void)
{
    auto var = symbol_list.get_symbol(ident);

    if(var.type != LVal::CONST)
        throw std::runtime_error("error: LValAST must be LVal::CONST in symbol table");

    return ((koopa_raw_value_t)var.number)->kind.data.integer.value;
}

PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST> &_next_exp)
{
    next_exp = std::move(_next_exp);

    return;
}

void *PrimaryExpAST::to_koopa(void)
{
    return next_exp->to_koopa();
}

int PrimaryExpAST::value(void)
{
    return next_exp->value();
}

UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    next_exp = std::move(_primary_exp);

    return;
}

UnaryExpAST::UnaryExpAST(std::string _op, std::unique_ptr<BaseAST> &_unary_exp) : op(_op)
{
    type = OP;
    next_exp = std::move(_unary_exp);

    return;
}

UnaryExpAST::UnaryExpAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_rparams) : op(_ident)
{
    type = FUNCTION;
    for(auto &rparam : _rparams)
        rparams.push_back(std::move(rparam));

    return;
}

void *UnaryExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;
    koopa_raw_function_data_t *func = nullptr;
    std::vector<void *> params;

    switch(type)
    {
    case FUNCTION:
        func = (koopa_raw_function_data_t *)symbol_list.get_symbol(op).number;
        for(auto &rparam : rparams)
            params.push_back(rparam->to_koopa());
        res = new koopa_raw_value_data{func->ty->data.function.ret, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_CALL, .data.call.callee = func, .data.call.args = {vector_data(params), (unsigned)params.size(), KOOPA_RSIK_VALUE}}};

        block_inst.add_inst(res);
        break;
    case PRIMARY:
        res = (koopa_raw_value_data *)next_exp->to_koopa();
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

        auto &binary = res->kind.data.binary;
        if(op == "+")
            binary.op = KOOPA_RBO_ADD;
        else if(op == "-")
            binary.op = KOOPA_RBO_SUB;
        else if(op == "!")
            binary.op = KOOPA_RBO_EQ;
        binary.lhs = (koopa_raw_value_t)NumberAST(0).to_koopa();
        binary.rhs = (koopa_raw_value_t)next_exp->to_koopa();

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int UnaryExpAST::value(void)
{
    if(type == PRIMARY)
        return next_exp->value();

    int res = 0;
    if(op == "+")
        res = next_exp->value();
    else if(op == "-")
        res = -next_exp->value();
    else if(op == "!")
        res = !next_exp->value();

    return res;
}

MulExpAST::MulExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
MulExpAST::MulExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *MulExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

        auto &binary = res->kind.data.binary;
        if(op == "*")
            binary.op = KOOPA_RBO_MUL;
        else if(op == "/")
            binary.op = KOOPA_RBO_DIV;
        else if(op == "%")
            binary.op = KOOPA_RBO_MOD;
        binary.lhs = (koopa_raw_value_t)left_exp->to_koopa();
        binary.rhs = (koopa_raw_value_t)right_exp->to_koopa();

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int MulExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "*")
        res = left_exp->value() * right_exp->value();
    else if(op == "/")
        res = left_exp->value() / right_exp->value();
    else if(op == "%")
        res = left_exp->value() % right_exp->value();

    return res;
}

AddExpAST::AddExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
AddExpAST::AddExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *AddExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

        auto &binary = res->kind.data.binary;
        if(op == "+")
            binary.op = KOOPA_RBO_ADD;
        else if(op == "-")
            binary.op = KOOPA_RBO_SUB;
        binary.lhs = (koopa_raw_value_t)left_exp->to_koopa();
        binary.rhs = (koopa_raw_value_t)right_exp->to_koopa();

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int AddExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "+")
        res = left_exp->value() + right_exp->value();
    else if(op == "-")
        res = left_exp->value() - right_exp->value();

    return res;
}

RelExpAST::RelExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
RelExpAST::RelExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *RelExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

        auto &binary = res->kind.data.binary;
        if(op == "<")
            binary.op = KOOPA_RBO_LT;
        else if(op == "<=")
            binary.op = KOOPA_RBO_LE;
        else if(op == ">")
            binary.op = KOOPA_RBO_GT;
        else if(op == ">=")
            binary.op = KOOPA_RBO_GE;
        binary.lhs = (koopa_raw_value_t)left_exp->to_koopa();
        binary.rhs = (koopa_raw_value_t)right_exp->to_koopa();

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int RelExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "<")
        res = left_exp->value() < right_exp->value();
    else if(op == "<=")
        res = left_exp->value() <= right_exp->value();
    else if(op == ">")
        res = left_exp->value() > right_exp->value();
    else if(op == ">=")
        res = left_exp->value() >= right_exp->value();

    return res;
}

EqExpAST::EqExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
EqExpAST::EqExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *EqExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

        auto &binary = res->kind.data.binary;
        if(op == "==")
            binary.op = KOOPA_RBO_EQ;
        else if(op == "!=")
            binary.op = KOOPA_RBO_NOT_EQ;
        binary.lhs = (koopa_raw_value_t)left_exp->to_koopa();
        binary.rhs = (koopa_raw_value_t)right_exp->to_koopa();

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int EqExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "==")
        res = left_exp->value() == right_exp->value();
    else if(op == "!=")
        res = left_exp->value() != right_exp->value();

    return res;
}

static koopa_raw_value_data *to_bool(BlockInst *block_inst, koopa_raw_value_t exp, int op)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BINARY}};

    auto &binary = res->kind.data.binary;
    binary.op = op;
    binary.lhs = exp;
    binary.rhs = (koopa_raw_value_t)NumberAST(0).to_koopa();

    block_inst->add_inst(res);
    return res;
}

LAndExpAST::LAndExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
LAndExpAST::LAndExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *LAndExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        koopa_raw_value_data *temp_var = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}}, "%temp", {nullptr, 0, KOOPA_RSIK_TYPE}, {.tag = KOOPA_RVT_ALLOC}};
        koopa_raw_value_data *temp_store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = temp_var, .data.store.value = (koopa_raw_value_t)NumberAST(0).to_koopa()}};
        block_inst.add_inst(temp_var);
        block_inst.add_inst(temp_store);

        koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t{"%true", {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
        koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t{"%end", {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
        koopa_raw_value_data *branch = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BRANCH, .data.branch.cond = to_bool(&block_inst, (koopa_raw_value_t)left_exp->to_koopa(), KOOPA_RBO_NOT_EQ), .data.branch.true_bb = true_block, .data.branch.false_bb = end_block, .data.branch.true_args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.branch.false_args = {nullptr, 0, KOOPA_RSIK_VALUE}}};
        block_inst.add_inst(branch);

        block_inst.new_block(true_block);
        koopa_raw_value_data *right_store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = temp_var, .data.store.value = to_bool(&block_inst, (koopa_raw_value_t)right_exp->to_koopa(), KOOPA_RBO_NOT_EQ)}};
        block_inst.add_inst(right_store);
        block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = end_block}});

        block_inst.new_block(end_block);
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = temp_var}};

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int LAndExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "&&")
        res = left_exp->value() && right_exp->value();

    return res;
}

LOrExpAST::LOrExpAST(std::unique_ptr<BaseAST> &_primary_exp)
{
    type = PRIMARY;
    left_exp = std::move(_primary_exp);

    return;
}
LOrExpAST::LOrExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp) : op(_op)
{
    type = OP;
    left_exp = std::move(_left_exp);
    right_exp = std::move(_right_exp);

    return;
}

void *LOrExpAST::to_koopa(void)
{
    koopa_raw_value_data *res = nullptr;

    switch(type)
    {
    case PRIMARY:
        res = (koopa_raw_value_data *)left_exp->to_koopa();
        break;
    case OP:
        koopa_raw_value_data *temp_var = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}}, "%temp", {nullptr, 0, KOOPA_RSIK_TYPE}, {.tag = KOOPA_RVT_ALLOC}};
        koopa_raw_value_data *temp_store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = temp_var, .data.store.value = (koopa_raw_value_t)NumberAST(1).to_koopa()}};
        block_inst.add_inst(temp_var);
        block_inst.add_inst(temp_store);

        koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t{"%true", {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
        koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t{"%end", {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
        koopa_raw_value_data *branch = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BRANCH, .data.branch.cond = to_bool(&block_inst, (koopa_raw_value_t)left_exp->to_koopa(), KOOPA_RBO_EQ), .data.branch.true_bb = true_block, .data.branch.false_bb = end_block, .data.branch.true_args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.branch.false_args = {nullptr, 0, KOOPA_RSIK_VALUE}}};
        block_inst.add_inst(branch);

        block_inst.new_block(true_block);
        koopa_raw_value_data *right_store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = temp_var, .data.store.value = to_bool(&block_inst, (koopa_raw_value_t)right_exp->to_koopa(), KOOPA_RBO_NOT_EQ)}};
        block_inst.add_inst(right_store);
        block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = end_block}});

        block_inst.new_block(end_block);
        res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_LOAD, .data.load.src = temp_var}};

        block_inst.add_inst(res);
        break;
    }

    return res;
}

int LOrExpAST::value(void)
{
    if(type == PRIMARY)
        return left_exp->value();

    int res = 0;
    if(op == "||")
        res = left_exp->value() || right_exp->value();

    return res;
}

NumberAST::NumberAST(int _val) : val(_val)
{
    return;
}

void *NumberAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = val}};

    return res;
}

int NumberAST::value(void)
{
    return val;
}
