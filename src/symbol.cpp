#include <stdexcept>
#include "koopa.h"
#include "symbol.hpp"

void SymbolList::set_scope(std::vector<void *> *_scope)
{
    scope = _scope;

    return;
}

void SymbolList::new_scope(void *sc)
{
    scope->push_back(sc);

    return;
}

void SymbolList::new_block(void)
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

void SymbolList::delete_block(void)
{
    table.pop_back();

    return;
}
