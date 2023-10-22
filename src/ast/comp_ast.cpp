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
    std::vector<void *> blocks{block->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN})};
    koopa_raw_function_data_t *res = new koopa_raw_function_data_t();

    char *name = new char(ident.size() + 2);
    ("@" + ident).copy(name, ident.size() + 1);

    res->name = name;
    res->ty = new koopa_raw_type_kind_t{.tag = KOOPA_RTT_FUNCTION, .data.function.params = {nullptr, 0, KOOPA_RSIK_TYPE}, .data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa({nullptr, 0, KOOPA_RSIK_UNKNOWN})};
    res->params = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->bbs = {vector_data(blocks), (unsigned)blocks.size(), KOOPA_RSIK_BASIC_BLOCK};

    return res;
}

FuncTypeAST::FuncTypeAST(const char *_name) : name(_name)
{
    return;
}

void *FuncTypeAST::to_koopa(koopa_raw_slice_t parent)
{
    if(name == "int")
        return new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32};
    throw std::runtime_error("error: FuncType is " + name + " but not int");
}

BlockAST::BlockAST(std::unique_ptr<BaseAST> &_stmt)
{
    stmt = std::move(_stmt);
}

void *BlockAST::to_koopa(koopa_raw_slice_t parent)
{
    std::vector<void *> stmts;
    koopa_raw_basic_block_data_t *res = new koopa_raw_basic_block_data_t();

    res->name = "%entry";
    res->params = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->used_by = {nullptr, 0, KOOPA_RSIK_VALUE};

    stmt->to_vector(stmts, {nullptr, 0, KOOPA_RSIK_UNKNOWN});
    res->insts = {vector_data(stmts), (unsigned)stmts.size(), KOOPA_RSIK_VALUE};

    return res;
}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &_ret_val)
{
    ret_val = std::move(_ret_val);

    return;
}

#include <cstdio>

void *StmtAST::to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent)
{
    koopa_raw_value_data *res = new koopa_raw_value_data();
    koopa_raw_slice_t child = {new const void *[1]{res}, 1, KOOPA_RSIK_VALUE};

    res->ty = new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT};
    res->name = nullptr;
    res->used_by = {nullptr, 0, KOOPA_RSIK_VALUE};
    res->kind.tag = KOOPA_RVT_RETURN;
    res->kind.data.ret.value = (const koopa_raw_value_data *)ret_val->to_vector(vec, child);
    vec.push_back(res);

    return res;
}
