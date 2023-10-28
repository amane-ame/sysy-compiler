#include <algorithm>
#include <stdexcept>
#include <vector>
#include "ast.hpp"
#include "koopa.h"
#include "table.hpp"

void SymbolList::new_scope(void)
{
    table.push_back(std::map<std::string, LVal>());

    return;
}

void SymbolList::add_symbol(const std::string &name, LVal koopa_item)
{
    table.back()[name] = koopa_item;

    return;
}

LVal SymbolList::get_symbol(const std::string &name)
{
    for(auto page = table.rbegin(); page != table.rend(); page ++)
        if(page->count(name))
            return (*page)[name];

    throw("error: cannot find " + name + " in symbol table");
}

void SymbolList::end_scope(void)
{
    table.pop_back();

    return;
}

void BlockInst::set_block(std::vector<void *> *_block)
{
    block = _block;

    return;
}

void BlockInst::new_block(koopa_raw_basic_block_data_t *basic)
{
    end_block();
    basic->insts.buffer = nullptr;
    block->push_back(basic);

    return;
}

void BlockInst::end_block(void)
{
    if(!block->empty())
    {
        for(auto &i : current)
        {
            koopa_raw_value_t t = (koopa_raw_value_t)i;
            if(t->kind.tag == KOOPA_RVT_BRANCH || t->kind.tag == KOOPA_RVT_RETURN || t->kind.tag == KOOPA_RVT_JUMP)
            {
                current.resize(&i - current.data() + 1);
                break;
            }
        }

        koopa_raw_basic_block_data_t *last = (koopa_raw_basic_block_data_t *)(*block->rbegin());
        if(!last->insts.buffer)
            last->insts = {BaseAST().vector_data(current), (unsigned)current.size(), KOOPA_RSIK_VALUE};
    }
    current.clear();

    return;
}

void BlockInst::add_inst(void *inst)
{
    current.push_back(inst);

    return;
}
