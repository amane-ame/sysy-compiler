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

static int blk_size(koopa_raw_basic_block_t kblk, bool &call)
{
    int sz = 0;

    for(int i = 0; i < (int)kblk->insts.len; i ++)
    {
        sz += inst_size((koopa_raw_value_t)kblk->insts.buffer[i]);
        if(((koopa_raw_value_t)kblk->insts.buffer[i])->kind.tag == KOOPA_RVT_CALL)
            call = true;
    }

    return sz;
}

static int func_size(koopa_raw_function_t kfunc, bool &call)
{
    int sz = 0;

    for(int i = 0; i < (int)kfunc->bbs.len; i ++)
        sz += blk_size((koopa_raw_basic_block_t)kfunc->bbs.buffer[i], call);
    sz += 4 * call;

    return sz;
}

class Stack
{
private:
    int reserve, cur;
    std::map<koopa_raw_value_t, int> addr;
    bool call;

public:
    void clear(int sz, bool _call)
    {
        reserve = cur = sz;
        call = _call;
        addr.clear();
        cur -= 4 * call;

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

    bool has_call(void)
    {
        return call;
    }
} stack;
static std::string current_ident;

static void load_reg(koopa_raw_value_t kval, std::string reg, std::string &res)
{
    if(kval->kind.tag == KOOPA_RVT_INTEGER)
        res += "\tli " + reg + ", " + std::to_string(kval->kind.data.integer.value) + "\n";
    else if(kval->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        res += "\tla t0, " + std::string(kval->name + 1) + "\n";
        res += "\tlw " + reg + ", 0(t0)\n";
    }
    else
        res += "\tlw " + reg + ", " + std::to_string(stack.fetch(kval)) + "(sp)\n";

    return;
}

static void value_global_alloc(koopa_raw_value_t kalloc, std::string &res)
{
    res += ".globl " + std::string(kalloc->name + 1) + "\n";
    res += std::string(kalloc->name + 1) + ":\n";
    if (kalloc->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT)
        res += "\t.zero 4\n";
    else
        res += "\t.word " + std::to_string(kalloc->kind.data.global_alloc.init->kind.data.integer.value) + "\n";

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
    std::string dest;

    res += "\n";
    if(kstore->dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        res += "\tla t1, " + std::string(kstore->dest->name + 1) + "\n";
        dest = "0(t1)";
    }
    else
        dest = std::to_string(stack.fetch(kstore->dest)) + "(sp)";

    if(kstore->value->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
    {
        if(kstore->value->kind.data.func_arg_ref.index < 8)
            res += "\tsw a" + std::to_string(kstore->value->kind.data.func_arg_ref.index) + ", " + dest + "\n";
        else
        {
            res += "\tlw t0, " + std::to_string((kstore->value->kind.data.func_arg_ref.index - 8) * 4) + "(sp)\n";
            res += "\tsw t0, " + dest + "\n";
        }
    }
    else
    {
        load_reg(kstore->value, "t0", res);
        res += "\tsw t0, " + dest + "\n";
    }

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

static void value_branch(const koopa_raw_branch_t *kbranch, std::string &res)
{
    static int magic;

    res += "\n";
    load_reg(kbranch->cond, "t0", res);
    // res += "\tbnez t0, " + current_ident + "_skip" + std::to_string(magic) + "\n";
    // res += "\tj " + current_ident + "_" + std::string(kbranch->true_bb->name + 1) + "\n";
    // res += current_ident + "_skip" + std::to_string(magic ++) + ":\n";
    // res += "\tj " + current_ident + "_" + std::string(kbranch->false_bb->name + 1) + "\n";
    res += "\tbnez t0, " + current_ident + "_" + std::string(kbranch->true_bb->name + 1) + "\n";
    res += "\tj " + current_ident + "_" + std::string(kbranch->false_bb->name + 1) + "\n";

    return;
}

static void value_jump(const koopa_raw_jump_t *kjump, std::string &res)
{
    res += "\n";
    res += "\tj " + current_ident + "_" + std::string(kjump->target->name + 1) + "\n";

    return;
}

static void value_call(const koopa_raw_call_t *kcall, int addr, std::string &res)
{
    res += "\n";
    for(int i = 0; i < std::min((int)kcall->args.len, 8); i++)
        load_reg((koopa_raw_value_t)kcall->args.buffer[i], "a" + std::string(1, '0' + i), res);

    bool call = false;
    int sz = func_size(kcall->callee, call);
    if(sz)
        sz = ((sz - 1) / 16 + 1) * 16;
    for(int i = 8; i < (int)kcall->args.len; i++)
    {
        load_reg((koopa_raw_value_t)kcall->args.buffer[i], "t0", res);
        res += "\tsw t0, " + std::to_string((i - 8) * 4 - sz) + "(sp)\n";
    }
    res += "\tcall " + std::string(kcall->callee->name + 1) + "\n";
    if (addr != -1)
        res += "\tsw a0, " + std::to_string(addr) + "(sp)\n";

    return;
}

static void value_return(const koopa_raw_return_t *kret, std::string &res)
{
    res += "\n";
    if(kret->value)
    {
        if(kret->value->kind.tag == KOOPA_RVT_INTEGER)
            res += "\tli a0, " + std::to_string(kret->value->kind.data.integer.value) + "\n";
        else
            res += "\tlw a0, " + std::to_string(stack.fetch(kret->value)) + "(sp)\n";
    }
    if(stack.has_call())
        res += "\tlw ra, " + std::to_string(stack.size() - 4) + "(sp)\n";
    if(stack.size())
        res += "\taddi sp, sp, " + std::to_string(stack.size()) + "\n";
    res += "\tret\n";

    return;
}

///////////////////////////////////////////////////////////////

static void visit_slice(const koopa_raw_slice_t *rs, std::string &res);

static void visit_func(koopa_raw_function_t kfunc, std::string &res)
{
    if(!kfunc->bbs.len)
        return;

    res += ".globl " + std::string(kfunc->name + 1) + "\n";
    res += std::string(kfunc->name + 1) + ":\n";

    bool call = false;
    int size = func_size(kfunc, call);
    if(size)
    {
        size = ((size - 1) / 16 + 1) * 16;
        res += "\taddi sp, sp, " + std::to_string(-size) + "\n";
    }
    if(call)
        res += "\tsw ra, " + std::to_string(size - 4) + "(sp)\n";
    stack.clear(size, call);
    current_ident = std::string(kfunc->name + 1);
    visit_slice(&kfunc->bbs, res);

    return;
}

static void visit_block(koopa_raw_basic_block_t kblk, std::string &res)
{
    res += "\n" + current_ident + "_" + std::string(kblk->name + 1) + ":\n";
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
    case KOOPA_RVT_GLOBAL_ALLOC:
        value_global_alloc(kval, res);
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
    case KOOPA_RVT_BRANCH:
        value_branch(&kval->kind.data.branch, res);
        break;
    case KOOPA_RVT_JUMP:
        value_jump(&kval->kind.data.jump, res);
        break;
    case KOOPA_RVT_CALL:
        value_call(&kval->kind.data.call, kval->ty->tag == KOOPA_RTT_UNIT ? -1 : addr, res);
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

    res += ".data\n";
    visit_slice(&krp->values, res);
    res += ".text\n";
    visit_slice(&krp->funcs, res);

    return res;
}
