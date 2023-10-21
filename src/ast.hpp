#pragma once

#include <memory>
#include <string>
#include "koopa.h"

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual std::string to_string() = 0;
    virtual void *to_koopa() = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> func_def;

public:
    CompUnitAST(std::unique_ptr<BaseAST> &_func_def);

    std::string to_string();
    void *to_koopa();
    koopa_raw_program_t to_koopa_program();
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

public:
    FuncDefAST(std::unique_ptr<BaseAST> &_func_type, const char *_ident, std::unique_ptr<BaseAST> &_block);

    std::string to_string();
    void *to_koopa();
};

class FuncTypeAST : public BaseAST
{
private:
    std::string name;

public:
    FuncTypeAST(const char *_name);

    std::string to_string();
    void *to_koopa();
};

class BlockAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> stmt;

public:
    BlockAST(std::unique_ptr<BaseAST> &_stmt);

    std::string to_string();
    void *to_koopa();
};

class StmtAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> ret_val;

public:
    StmtAST(std::unique_ptr<BaseAST> &_ret_val);

    std::string to_string();
    void *to_koopa();
};

class NumberAST : public BaseAST
{
private:
    int val;

public:
    NumberAST(int _val);

    std::string to_string();
    void *to_koopa();
};
