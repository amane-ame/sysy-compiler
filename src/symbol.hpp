#pragma once

#include <map>
#include <string>
#include <vector>

struct LVal
{
    enum SymbolType
    {
        CONST,
        VAR,
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
