#pragma once

#include <map>
#include <string>
#include <vector>
#include "koopa.h"

struct LVal
{
    enum SymbolType
    {
        CONST,
        VAR
    } type;

    koopa_raw_value_t number;
};

class SymbolList
{
private:
    std::vector<void *> *scope;
    std::vector<std::map<std::string, LVal>> table;

public:
    void set_scope(std::vector<void *> *_scope);
    void new_scope(void *sc);

    void new_block(void);
    void delete_block(void);

    void add_symbol(const std::string &name, LVal koopa_item);
    LVal get_symbol(const std::string &name);
};
