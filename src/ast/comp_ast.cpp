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
    s.copy(res, s.size());

    return res;
}

CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST> &_func_def)
{
    func_def = std::move(_func_def);

    return;
}

koopa_raw_program_t CompUnitAST::to_koopa_program(void)
{
    std::vector<void *> funcs{func_def->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN})};

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

void *FuncDefAST::to_koopa(koopa_raw_slice_t parent)
{
    std::vector<void *> blocks{block->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN})}, insts;
    koopa_raw_function_data_t *res = new koopa_raw_function_data_t();

    res->name = string_data("@" + ident);
    res->ty = new koopa_raw_type_kind_t{.tag = KOOPA_RTT_FUNCTION, .data.function.params = {nullptr, 0, KOOPA_RSIK_TYPE}, .data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN})};
    res->params = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->bbs = {vector_data(blocks), (unsigned)blocks.size(), KOOPA_RSIK_BASIC_BLOCK};

    koopa_raw_basic_block_data_t *entry = new koopa_raw_basic_block_data_t{string_data("%entry_" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {nullptr, 0, KOOPA_RSIK_VALUE}};
    block->to_vector(insts, {nullptr, 0, KOOPA_RSIK_UNKNOWN});
    entry->insts = {vector_data(insts), (unsigned)insts.size(), KOOPA_RSIK_VALUE};
    symbol_list.set_scope(&blocks);
    symbol_list.new_scope(entry);

    return res;
}

FuncTypeAST::FuncTypeAST(std::string _ident) : ident(_ident)
{
    return;
}

void *FuncTypeAST::to_koopa(koopa_raw_slice_t parent)
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

void *BlockAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    symbol_list.new_block();
    for(auto &inst : insts)
        switch(inst.first)
        {
        case CONSTDECL:
            inst.second->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN});
            break;
        case DECL:
            inst.second->to_vector(vec, {nullptr, 0, KOOPA_RSIK_UNKNOWN});
            break;
        case STMT:
            inst.second->to_vector(vec, {nullptr, 0, KOOPA_RSIK_UNKNOWN});
            break;
        }
    symbol_list.delete_block();

    return nullptr;
}

ReturnAST::ReturnAST(std::unique_ptr<BaseAST> &_ret_val)
{
    ret_val = std::move(_ret_val);

    return;
}

void *ReturnAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_RETURN}};
    koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};
    
    res->kind.data.ret.value = (const koopa_raw_value_data *)ret_val->to_vector(vec, child);
    vec.push_back(res);

    return res;
}

AssignmentAST::AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp)
{
    lval = std::move(_lval);
    exp = std::move(_exp);

    return;
}

void *AssignmentAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_STORE}};
    koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

    res->kind.data.store.value = (koopa_raw_value_t)exp->to_vector(vec, child);
    res->kind.data.store.dest = (koopa_raw_value_t)lval->to_koopa(child);
    vec.push_back(res);

    return nullptr;
}

ConstDefAST::ConstDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp) : ident(_ident)
{
    exp = std::move(_exp);

    return;
}

void *ConstDefAST::to_koopa(koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, parent, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = exp->value()}};
    
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

void *VarDefAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data();
    koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

    res->name = string_data("@" + ident);
    res->ty = new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}};
    res->used_by = parent;
    res->kind.tag = KOOPA_RVT_ALLOC;

    vec.push_back(res);
    symbol_list.add_symbol(ident, LVal{LVal::VAR, res});

    if(exp)
    {
        koopa_raw_value_data *store = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_UNKNOWN}, {.tag = KOOPA_RVT_STORE, .data.store.dest = res, .data.store.value = (koopa_raw_value_t)exp->to_vector(vec, child)}};
        vec.push_back(store);
    }
    
    return res;
}
