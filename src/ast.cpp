#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "ast.hpp"

static const void **vector_data(std::vector<void *> &v)
{
    auto buffer = new const void *[v.size()];
    std::copy(v.begin(), v.end(), buffer);

    return buffer;
}

CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST> &_func_def)
{
    func_def = std::move(_func_def);

    return;
}

std::string CompUnitAST::to_string()
{
    return "CompUnitAST { " + func_def->to_string() + " }";
}

void *CompUnitAST::to_koopa()
{
    throw std::runtime_error("error: CompUnitAST cannot to_koopa");
}

koopa_raw_program_t CompUnitAST::to_koopa_program()
{
    std::vector<void *> funcs{func_def->to_koopa()};

    koopa_raw_program_t res;
    res.values = {nullptr, 0, KOOPA_RSIK_VALUE};
    res.funcs = {vector_data(funcs), (unsigned)funcs.size(), KOOPA_RSIK_FUNCTION};

    return res;
}

FuncDefAST::FuncDefAST(std::unique_ptr<BaseAST> &_func_type, const char *_ident, std::unique_ptr<BaseAST> &_block) : ident(_ident)
{
    func_type = std::move(_func_type);
    block = std::move(_block);

    return;
}

std::string FuncDefAST::to_string()
{
    return "FuncDefAST { " + func_type->to_string() + ", " + ident + ", " + block->to_string() + " }";
}

void *FuncDefAST::to_koopa()
{
    std::vector<void *> blocks{block->to_koopa()};
    koopa_raw_function_data_t *res = new koopa_raw_function_data_t();

    char *name = new char(ident.length() + 1);
    ("@" + ident).copy(name, sizeof(name));

    res->name = name;
    res->ty = new koopa_raw_type_kind_t{.tag = KOOPA_RTT_FUNCTION, .data.function.params = {nullptr, 0, KOOPA_RSIK_TYPE}, .data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa()};
    res->params = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->bbs = {vector_data(blocks), (unsigned)blocks.size(), KOOPA_RSIK_BASIC_BLOCK};

    return res;
}

FuncTypeAST::FuncTypeAST(const char *_name) : name(_name)
{
    return;
}

std::string FuncTypeAST::to_string()
{
    return "FuncTypeAST { " + name + " }";
}

void *FuncTypeAST::to_koopa()
{
    if(name == "int")
        return new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32};
    throw std::runtime_error("error: FuncType is " + name + " but not int");
}

BlockAST::BlockAST(std::unique_ptr<BaseAST> &_stmt)
{
    stmt = std::move(_stmt);
}

std::string BlockAST::to_string()
{
    return "BlockAST { " + stmt->to_string() + " }";
}

void *BlockAST::to_koopa()
{
    std::vector<void *> stmts{stmt->to_koopa()};
    koopa_raw_basic_block_data_t *res = new koopa_raw_basic_block_data_t();

    res->name = "%entry";
    res->params = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->used_by = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->insts = {vector_data(stmts), (unsigned)stmts.size(), KOOPA_RSIK_VALUE};

    return res;
}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &_ret_val)
{
    ret_val = std::move(_ret_val);

    return;
}

std::string StmtAST::to_string()
{
    return "StmtAST { return, " + ret_val->to_string() + " }";
}

void *StmtAST::to_koopa()
{
    koopa_raw_value_data *res = new koopa_raw_value_data();

    res->ty = new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT};
    res->name = nullptr;
    res->used_by = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->kind.tag = KOOPA_RVT_RETURN;
    res->kind.data.ret.value = (const koopa_raw_value_data*)ret_val->to_koopa();

    return res;
}

NumberAST::NumberAST(int _val) : val(_val)
{
    return;
}

std::string NumberAST::to_string()
{
    return "NumberAST { int " + std::to_string(val) + " }";
}

void *NumberAST::to_koopa()
{
    koopa_raw_value_data *res = new koopa_raw_value_data();

    res->ty = new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32};
    res->name = nullptr;
    res->used_by = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->kind.tag = KOOPA_RVT_INTEGER;
    res->kind.data.integer.value = val;

    return res;
}
