#include <map>
#include <stdexcept>
#include <string>
#include "koopa.h"
#include "riscv.hpp"

static int inst_size(koopa_raw_value_t kval)
{
    switch(kval->ty->tag)
    {
    case KOOPA_RTT_UNIT:
        return 0;
    case KOOPA_RTT_INT32:
    case KOOPA_RTT_POINTER:
        return 4;
    default:
        throw std::runtime_error("error: unknown kval.tag " + std::to_string(kval->ty->tag));
    }

    return 0;
}

static int blk_size(koopa_raw_basic_block_t kblk)
{
    int sz = 0;

    for(int i = 0; i < (int)kblk->insts.len; i ++)
        sz += inst_size((koopa_raw_value_t)kblk->insts.buffer[i]);

    return sz;
}

static int func_size(koopa_raw_function_t kfunc)
{
    int sz = 0;

    for(int i = 0; i < (int)kfunc->bbs.len; i ++)
        sz += blk_size((koopa_raw_basic_block_t)kfunc->bbs.buffer[i]);

    return sz;
}

class Stack
{
private:
    int reserve, cur;
    std::map<koopa_raw_value_t, int> addr;

public:
    void clear(int sz)
    {
        reserve = cur = sz;
        addr.clear();

        return;
    }

    int size(void)
    {
        return reserve;
    }

    int fetch(koopa_raw_value_t kval)
    {
        if(addr.count(kval))
            return addr[kval];

        int t = inst_size(kval);
        if(!t)
            return 0;
        addr[kval] = (cur -= t);

        return cur;
    }
} stack;

static void load_reg(koopa_raw_value_t kval, std::string reg, std::string &res)
{
    if(kval->kind.tag == KOOPA_RVT_INTEGER)
        res += "\tli " + reg + ", " + std::to_string(kval->kind.data.integer.value) + "\n";
    else
        res += "\tlw " + reg + ", " + std::to_string(stack.fetch(kval)) + "(sp)\n";

    return;
}

static void value_load(const koopa_raw_load_t *kload, int addr, std::string &res)
{
    res += "\n";
    load_reg(kload->src, "t0", res);
    res += "\tsw t0, " + std::to_string(addr) + "(sp)\n";

    return;
}

static void value_store(const koopa_raw_store_t *kstore, std::string &res)
{
    res += "\n";
    load_reg(kstore->value, "t0", res);
    res += "\tsw t0, " + std::to_string(stack.fetch(kstore->dest)) += "(sp)\n";

    return;
}

static void value_binary(const koopa_raw_binary_t *kbinary, int addr, std::string &res)
{
    res += "\n";
    load_reg(kbinary->lhs, "t0", res);
    load_reg(kbinary->rhs, "t1", res);

    switch(kbinary->op)
    {
    case KOOPA_RBO_NOT_EQ:
        res += "\txor t0, t0, t1\n";
        res += "\tsnez t0, t0\n";
        break;
    case KOOPA_RBO_EQ:
        res += "\txor t0, t0, t1\n";
        res += "\tseqz t0, t0\n";
        break;
    case KOOPA_RBO_GT:
        res += "\tsgt t0, t0, t1\n";
        break;
    case KOOPA_RBO_LT:
        res += "\tslt t0, t0, t1\n";
        break;
    case KOOPA_RBO_GE:
        res += "\tslt t0, t0, t1\n";
        res += "\txori t0, t0, 1\n";
        break;
    case KOOPA_RBO_LE:
        res += "\tsgt t0, t0, t1\n";
        res += "\txori t0, t0, 1\n";
        break;
    case KOOPA_RBO_ADD:
        res += "\tadd t0, t0, t1\n";
        break;
    case KOOPA_RBO_SUB:
        res += "\tsub t0, t0, t1\n";
        break;
    case KOOPA_RBO_MUL:
        res += "\tmul t0, t0, t1\n";
        break;
    case KOOPA_RBO_DIV:
        res += "\tdiv t0, t0, t1\n";
        break;
    case KOOPA_RBO_MOD:
        res += "\trem t0, t0, t1\n";
        break;
    case KOOPA_RBO_AND:
        res += "\tand t0, t0, t1\n";
        break;
    case KOOPA_RBO_OR:
        res += "\tor t0, t0, t1\n";
        break;
    case KOOPA_RBO_XOR:
        res += "\txor t0, t0, t1\n";
        break;
    case KOOPA_RBO_SHL:
        res += "\tsll t0, t0, t1\n";
        break;
    case KOOPA_RBO_SHR:
        res += "\tsrl t0, t0, t1\n";
        break;
    case KOOPA_RBO_SAR:
        res += "\tsra t0, t0, t1\n";
        break;
    }
    res += "\tsw t0, " + std::to_string(addr) += "(sp)\n";

    return;
}

static void value_return(const koopa_raw_return_t *kret, std::string &res)
{
    res += "\n";
    if(kret->value->kind.tag == KOOPA_RVT_INTEGER)
        res += "\tli a0, " + std::to_string(kret->value->kind.data.integer.value) + "\n";
    else
        res += "\tlw a0, " + std::to_string(stack.fetch(kret->value)) + "(sp)\n";
    res += "\taddi sp, sp, " + std::to_string(stack.size()) + "\n";
    res += "\tret\n";

    return;
}



static void visit_slice(const koopa_raw_slice_t *rs, std::string &res);

static void visit_func(koopa_raw_function_t kfunc, std::string &res)
{
    res += std::string(".globl ") + (kfunc->name + 1) + "\n";
    res += std::string(kfunc->name + 1) + ":\n";

    int sz = func_size(kfunc);
    sz = ((sz - 15) / 16 + 1) * 16;
    res += "\taddi sp, sp, " + std::to_string(sz) + "\n";
    stack.clear(sz);
    visit_slice(&kfunc->bbs, res);

    return;
}

static void visit_block(koopa_raw_basic_block_t kblk, std::string &res)
{
    visit_slice(&kblk->insts, res);

    return;
}

static void visit_value(koopa_raw_value_t kval, std::string &res)
{
    int addr = stack.fetch(kval);

    switch(kval->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
        res += std::to_string(kval->kind.data.integer.value);
        break;
    case KOOPA_RVT_ALLOC:
        break;
    case KOOPA_RVT_LOAD:
        value_load(&kval->kind.data.load, addr, res);
        break;
    case KOOPA_RVT_STORE:
        value_store(&kval->kind.data.store, res);
        break;
    case KOOPA_RVT_BINARY:
        value_binary(&kval->kind.data.binary, addr, res);
        break;
    case KOOPA_RVT_RETURN:
        value_return(&kval->kind.data.ret, res);
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
            visit_func((koopa_raw_function_t)rs->buffer[i], res);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            visit_block((koopa_raw_basic_block_t)rs->buffer[i], res);
            break;
        case KOOPA_RSIK_VALUE:
            visit_value((koopa_raw_value_t)rs->buffer[i], res);
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
