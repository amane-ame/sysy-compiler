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
        VAR,
        ARRAY,
        POINTER,
        FUNCTION
    } type;

    void *number;
};

class SymbolList
{
private:
    std::vector<std::map<std::string, LVal>> table;

public:
    void new_scope(void);
    void end_scope(void);

    void add_symbol(const std::string &name, LVal koopa_item);
    LVal get_symbol(const std::string &name);
};

class BlockInst
{
private:
    std::vector<void *> current, *block;

public:
    void set_block(std::vector<void *> *_block);
    void new_block(koopa_raw_basic_block_data_t *basic);
    void end_block(void);
    void add_inst(void *inst);
};
