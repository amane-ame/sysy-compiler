#include <memory>
#include <string>
#include <vector>
#include "../ast.hpp"

InitValAST::InitValAST(std::unique_ptr<BaseAST> &_exp) : is_const(false)
{
    type = EXP;
    exp = std::move(_exp);

    return;
}

InitValAST::InitValAST(std::vector<std::unique_ptr<BaseAST>> &_arr_list) : is_const(false)
{
    type = ARRAY;
    for(auto &arr : _arr_list)
        arr_vec.emplace_back(std::move(arr));

    return;
}

void InitValAST::sub_preprocess(std::vector<int> &pro, int align, std::vector<koopa_raw_value_t> &buf)
{
    int target_size = buf.size() + pro[align];

    for(int i = 0; i < (int)arr_vec.size(); i ++)
    {
        InitValAST *t = (InitValAST *)arr_vec[i].get();
        if(t->type == EXP)
        {
            if(is_const)
                buf.push_back(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = t->exp->value()}});
            else
                buf.push_back((koopa_raw_value_t)t->exp->to_koopa());
        }
        else
        {
            int new_align_pos = align + 1;
            while(cache.size() % pro[new_align_pos] != 0)
                new_align_pos ++;
            t->sub_preprocess(pro, new_align_pos, buf);
        }
    }
    while((int)buf.size() < target_size)
        buf.push_back(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = 0}});

    return;
}

void InitValAST::preprocess(const std::vector<int> &sz)
{
    std::vector<int> pro(sz.size() + 1);

    pro[sz.size()] = 1;
    for(int i = (int)sz.size() - 1; i >= 0; i --)
        pro[i] = pro[i + 1] * sz[i];

    sub_preprocess(pro, 0, cache);
    return;
}

koopa_raw_value_t InitValAST::index(int idx)
{
    if(type == ARRAY)
        return cache[idx];
    else if(type == EXP)
        return (koopa_raw_value_t)exp->to_koopa();

    return nullptr;
}

koopa_raw_value_t InitValAST::sub_make_aggerate(std::vector<int> &sz, std::vector<int> &pro, int align, std::vector<koopa_raw_value_t> &buf, int pos)
{
    if(pro[align] == 1)
        return buf[pos];

    koopa_raw_value_data *res = new koopa_raw_value_data{array_data(sz, align), nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_AGGREGATE}};
    std::vector<void *> elems;

    for(int i = 0; i < sz[align]; i ++)
        elems.push_back((void *)sub_make_aggerate(sz, pro, align + 1, buf, pos + pro[align + 1] * i));
    res->kind.data.aggregate.elems = {vector_data(elems), (unsigned)elems.size(), KOOPA_RSIK_VALUE};

    return res;
}

koopa_raw_value_t InitValAST::make_aggerate(std::vector<int> &sz)
{
    std::vector<int> pro(sz.size() + 1);

    pro[sz.size()] = 1;
    for(int i = (int)sz.size() - 1; i >= 0; i --)
        pro[i] = pro[i + 1] * sz[i];

    return sub_make_aggerate(sz, pro, 0, cache, 0);
}

koopa_raw_value_data *ArrayDefAST::index(int i, std::vector<int> &pro, koopa_raw_value_data *src, int pos)
{
    if(pos >= (int)pro.size())
        return src;

    koopa_raw_value_data *get = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = src->ty->data.pointer.base->data.array.base}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GET_ELEM_PTR, .data.get_elem_ptr.src = src, .data.get_elem_ptr.index = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_INT32}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_INTEGER, .data.integer.value = i / pro[pos]}}}};
    block_inst.add_inst(get);

    return index(i % pro[pos], pro, get, pos + 1);
}

ArrayDefAST::ArrayDefAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_exp) : ident(_ident), init_val(nullptr)
{
    for(auto &exp : _exp)
        sz_exp.push_back(std::move(exp));

    return;
}
ArrayDefAST::ArrayDefAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_exp, std::unique_ptr<BaseAST> &_init_val) : ident(_ident)
{
    for(auto &exp : _exp)
        sz_exp.push_back(std::move(exp));
    init_val = std::move(_init_val);

    return;
}

void *ArrayDefAST::to_koopa(void)
{
    int total = 1;
    std::vector<int> sz;

    for(auto &exp : sz_exp)
    {
        int tmp = exp->value();
        total *= tmp;
        sz.push_back(tmp);
    }

    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = array_data(sz, 0)}, string_data("@" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_ALLOC}};
    block_inst.add_inst(res);
    symbol_list.add_symbol(ident, {LVal::ARRAY, res});

    if(init_val)
    {
        InitValAST *t = (InitValAST *)init_val.get();

        t->preprocess(sz);
        std::vector<int> pro(sz.size());
        pro.back() = 1;
        for(int i = sz.size() - 2; i >= 0; i --)
            pro[i] = pro[i + 1] * sz[i + 1];

        for(int i = 0; i < total; i ++)
            block_inst.add_inst(new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_UNIT}, nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_STORE, .data.store.value = t->index(i), .data.store.dest = index(i, pro, res, 0)}});
    }

    return res;
}

GlobalArrayDefAST::GlobalArrayDefAST(std::unique_ptr<BaseAST> &arraydef_ast)
{
    ArrayDefAST *arraydef = (ArrayDefAST *)arraydef_ast.get();

    ident = arraydef->ident;
    for(auto &exp : arraydef->sz_exp)
        sz_exp.push_back(std::move(exp));
    init_val = std::move(arraydef->init_val);

    return;
}

void *GlobalArrayDefAST::to_koopa(void)
{
    std::vector<int> sz;

    for(auto &exp : sz_exp)
        sz.push_back(exp->value());

    koopa_raw_value_data *res = new koopa_raw_value_data{new koopa_raw_type_kind{.tag = KOOPA_RTT_POINTER, .data.pointer.base = array_data(sz, 0)}, string_data("@" + ident), {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_GLOBAL_ALLOC}};
    symbol_list.add_symbol(ident, {LVal::ARRAY, res});

    if(init_val)
    {
        InitValAST *t = (InitValAST *)init_val.get();
        t->is_const = true;
        t->preprocess(sz);
        res->kind.data.global_alloc.init = t->make_aggerate(sz);
    }
    else
        res->kind.data.global_alloc.init = new koopa_raw_value_data{array_data(sz, 0), nullptr, {nullptr, 0, KOOPA_RSIK_VALUE}, {.tag = KOOPA_RVT_ZERO_INIT}};

    return res;
}
