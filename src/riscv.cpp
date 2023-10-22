#include "koopa.h"
#include "riscv.hpp"

static void visit_slice(const koopa_raw_slice_t *rs, std::string &res);

static void gen_riscv_func(koopa_raw_function_t kfunc, std::string &res)
{
    res += std::string(".globl ") + (kfunc->name + 1) + "\n";
    res += std::string(kfunc->name + 1) + ":\n";
    visit_slice(&kfunc->bbs, res);

    return;
}

static void gen_riscv_block(koopa_raw_basic_block_t kblk, std::string &res)
{
    visit_slice(&kblk->insts, res);

    return;
}

static void gen_riscv_value(koopa_raw_value_t kval, std::string &res)
{
    switch(kval->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
        res += std::to_string(kval->kind.data.integer.value);
        break;
    case KOOPA_RVT_RETURN:
        res += "li a0, ";
        gen_riscv_value(kval->kind.data.ret.value, res);
        res += "\nret\n";
        break;
    default:
        throw std::runtime_error("error: unknown kval.tag " + std::to_string(kval->kind.tag));
    }

    return;
}

static void visit_slice(const koopa_raw_slice_t *rs, std::string &res)
{
    for(int i = 0; i < (int)rs->len; i ++)
        switch(rs->kind)
        {
        case KOOPA_RSIK_FUNCTION:
            gen_riscv_func((koopa_raw_function_t)rs->buffer[i], res);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            gen_riscv_block((koopa_raw_basic_block_t)rs->buffer[i], res);
            break;
        case KOOPA_RSIK_VALUE:
            gen_riscv_value((koopa_raw_value_t)rs->buffer[i], res);
            break;
        }

    return;
}

std::string koopa2riscv(const koopa_raw_program_t *krp)
{
    std::string res;
    res.reserve(1U << 15);
    res += ".text\n";
    visit_slice(&krp->funcs, res);

    return res;
}
