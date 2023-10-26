#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "../ast.hpp"

static const void **vector_data(std::vector<void *> &vec)
{
    auto buffer = new const void *[vec.size()];
    std::copy(vec.begin(), vec.end(), buffer);

    return buffer;
}

static char *string_data(std::string s)
{
    char *res = new char[s.size() + 1];
    res[s.copy(res, s.size())] = 0;

    return res;
}

CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST> &_func_def)
{
    func_def = std::move(_func_def);

    return;
}

koopa_raw_program_t CompUnitAST::to_koopa_program(void)
{
    std::vector<void *> funcs{func_def->to_koopa()};

    koopa_raw_program_t res;
    res.values = {nullptr, 0, KOOPA_RSIK_VALUE};
    res.funcs = {vector_data(funcs), (unsigned)funcs.size(), KOOPA_RSIK_FUNCTION};

    return res;
}

FuncDefAST::FuncDefAST(std::unique_ptr<BaseAST> &_func_type, std::string _ident, std::unique_ptr<BaseAST> &_block) : ident(_ident)
{
    func_type = std::move(_func_type);
    block = std::move(_block);

    return;
}

void *FuncDefAST::to_koopa(void)
{
    std::vector<void *> blocks;

    koopa_raw_basic_block_data_t *entry = new koopa_raw_basic_block_data_t{string_data("%entry_" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    block_inst.set_block(&blocks);
    block_inst.new_block(entry);
    block->to_koopa();
    block_inst.end_block();

    koopa_raw_function_data_t *res = new koopa_raw_function_data_t{new koopa_raw_type_kind_t{.tag = KOOPA_RTT_FUNCTION, .data.function.params = {nullptr, 0, KOOPA_RSIK_TYPE}, .data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa()}, string_data("@" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {vector_data(blocks), (unsigned)blocks.size(), KOOPA_RSIK_BASIC_BLOCK}};

    return res;
}

FuncTypeAST::FuncTypeAST(std::string _ident) : ident(_ident)
{
    return;
}

void *FuncTypeAST::to_koopa(void)
{
    if(ident == "int")
        return new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32};
    throw std::runtime_error("error: FuncType is " + ident + " but not int");
}

BlockAST::BlockAST(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_insts)
{
    for(auto &inst : _insts)
        insts.push_back(make_pair(inst.first, std::move(inst.second)));

    return;
}

void *BlockAST::to_koopa(void)
{
    symbol_list.new_scope();
    for(auto &inst : insts)
        inst.second->to_koopa();
    symbol_list.end_scope();

    return nullptr;
}

ReturnAST::ReturnAST(std::unique_ptr<BaseAST> &_ret_val)
{
    ret_val = std::move(_ret_val);

    return;
}

void *ReturnAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_RETURN, .data.ret.value = (const koopa_raw_value_data *)ret_val->to_koopa()}};

    block_inst.add_inst(res);

    return res;
}

AssignmentAST::AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp)
{
    lval = std::move(_lval);
    exp = std::move(_exp);

    return;
}

void *AssignmentAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_STORE, .data.store.value = (koopa_raw_value_t)exp->to_koopa(), .data.store.dest = symbol_list.get_symbol(((LValAST *)lval.get())->ident).number}};

    block_inst.add_inst(res);

    return nullptr;
}

BranchAST::BranchAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_true_insts)
{
    exp = std::move(_exp);
    for (auto &inst : _true_insts)
        true_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));

    return;
}

BranchAST::BranchAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_true_insts, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_false_insts)
{
    exp = std::move(_exp);
    for (auto &inst : _true_insts)
        true_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    for (auto &inst : _false_insts)
        false_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));

    return;
}

void *BranchAST::to_koopa(void)
{
    koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t{string_data("%true"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    koopa_raw_basic_block_data_t *false_block = new koopa_raw_basic_block_data_t{string_data("%false"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t{string_data("%end"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BRANCH, .data.branch.cond = (koopa_raw_value_t)exp->to_koopa(), .data.branch.true_bb = true_block, .data.branch.false_bb = false_block, .data.branch.true_args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.branch.false_args = {nullptr, 0, KOOPA_RSIK_VALUE}}};

    block_inst.add_inst(res);

    block_inst.new_block(true_block);
    symbol_list.new_scope();
    for(auto &inst : true_insts)
        inst.second->to_koopa();
    symbol_list.end_scope();
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = end_block}});

    block_inst.new_block(false_block);
    symbol_list.new_scope();
    for(auto &inst : false_insts)
        inst.second->to_koopa();
    symbol_list.end_scope();
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = end_block}});

    block_inst.new_block(end_block);

    return nullptr;
}


WhileAST::WhileAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_body_insts)
{
    for (auto &inst : _body_insts)
        body_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    exp = std::move(_exp);

    return;
}

void *WhileAST::to_koopa(void)
{
    koopa_raw_basic_block_data_t *while_entry = new koopa_raw_basic_block_data_t{string_data("%while_entry"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    koopa_raw_basic_block_data_t *while_body = new koopa_raw_basic_block_data_t{string_data("%while_body"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};
    koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t{string_data("%end"), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}, {}};

    loop_inst.push_back(std::make_tuple(while_entry, while_body, end_block));
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = while_entry}});
    block_inst.new_block(while_entry);
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_BRANCH, .data.branch.cond = (koopa_raw_value_t)exp->to_koopa(), .data.branch.true_bb = while_body, .data.branch.false_bb = end_block, .data.branch.true_args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.branch.false_args = {nullptr, 0, KOOPA_RSIK_VALUE}}};
    block_inst.add_inst(res);

    block_inst.new_block(while_body);
    symbol_list.new_scope();
    for(auto &inst : body_insts)
        inst.second->to_koopa();
    symbol_list.end_scope();
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = while_entry}});

    block_inst.new_block(end_block);
    loop_inst.pop_back();

    return nullptr;
}

void *BreakAST::to_koopa(void)
{
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = std::get<2>(loop_inst.back())}});

    return nullptr;
}

void *ContinueAST::to_koopa(void)
{
    block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_JUMP, .data.jump.args = {nullptr, 0, KOOPA_RSIK_VALUE}, .data.jump.target = std::get<0>(loop_inst.back())}});

    return nullptr;
}

ConstDefAST::ConstDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp) : ident(_ident)
{
    exp = std::move(_exp);

    return;
}

void *ConstDefAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = exp->value()}};
    
    symbol_list.add_symbol(ident, LVal{LVal::CONST, res});

    return res;
}


VarDefAST::VarDefAST(std::string _ident) : ident(_ident)
{
    exp = nullptr;

    return;
}

VarDefAST::VarDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp) : ident(_ident)
{
    exp = std::move(_exp);

    return;
}

void *VarDefAST::to_koopa(void)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}}, string_data("@" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_ALLOC}};

    block_inst.add_inst(res);
    symbol_list.add_symbol(ident, LVal{LVal::VAR, res});

    if(exp)
    {
        koopa_raw_value_data *store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = res, .data.store.value = (koopa_raw_value_t)exp->to_koopa()}};
        block_inst.add_inst(store);
    }
    
    return res;
}
