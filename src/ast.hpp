#pragma once

#include <memory>
#include <string>
#include <vector>
#include "koopa.h"

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST(void) = default;
    virtual void *to_koopa(koopa_raw_slice_t parent);
    virtual void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> func_def;

public:
    CompUnitAST(std::unique_ptr<BaseAST> &_func_def);

    koopa_raw_program_t to_koopa_program(void);
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

public:
    FuncDefAST(std::unique_ptr<BaseAST> &_func_type, std::string _ident, std::unique_ptr<BaseAST> &_block);

    void *to_koopa(koopa_raw_slice_t parent);
};

class FuncTypeAST : public BaseAST
{
private:
    std::string name;

public:
    FuncTypeAST(const char *_name);

    void *to_koopa(koopa_raw_slice_t parent);
};

class BlockAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> stmt;

public:
    BlockAST(std::unique_ptr<BaseAST> &_stmt);

    void *to_koopa(koopa_raw_slice_t parent);
};

class StmtAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> ret_val;

public:
    StmtAST(std::unique_ptr<BaseAST> &_ret_val);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};


class ExpAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> unary_exp;

public:
    ExpAST(std::unique_ptr<BaseAST> &_unary_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class PrimaryExpAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> next_exp;

public:
    PrimaryExpAST(std::unique_ptr<BaseAST> &_next_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class UnaryExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> next_exp;

public:
    UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    UnaryExpAST(std::string _op, std::unique_ptr<BaseAST> &_unary_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class MulExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

public:
    MulExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    MulExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class AddExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

public:
    AddExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    AddExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class RelExpAST : public BaseAST
{
public:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

    RelExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    RelExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class EqExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

public:
    EqExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    EqExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class LAndExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

public:
    LAndExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    LAndExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class LOrExpAST : public BaseAST
{
private:
    enum
    {
        PRIMARY,
        OP
    } type;
    std::string op;
    std::unique_ptr<BaseAST> left_exp;
    std::unique_ptr<BaseAST> right_exp;

public:
    LOrExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    LOrExpAST(std::unique_ptr<BaseAST> &_left_exp, std::string _op, std::unique_ptr<BaseAST> &_right_exp);

    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};

class NumberAST : public BaseAST
{
private:
    int val;

public:
    NumberAST(int _val);

    void *to_koopa(koopa_raw_slice_t parent);
    void *to_vector(std::vector<void *> &vec, koopa_raw_slice_t parent);
};
