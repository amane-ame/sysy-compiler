#pragma once

#include <memory>
#include <string>
#include <vector>
#include "block.hpp"
#include "koopa.h"
#include "symbol.hpp"

enum InstType
{
    CONSTDECL,
    DECL,
    STMT,
    BRANCH
};

// 所有 AST 的基类
class BaseAST
{
protected:
    static SymbolList symbol_list;
    static BlockInst block_inst;

public:
    virtual ~BaseAST(void) = default;
    virtual void *to_koopa(void);
    virtual int value(void);
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

    void *to_koopa(void);
};

class FuncTypeAST : public BaseAST
{
private:
    std::string ident;

public:
    FuncTypeAST(std::string _ident);

    void *to_koopa(void);
};

class BlockAST : public BaseAST
{
private:
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> insts;

public:
    BlockAST(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_insts);

    void *to_koopa(void);
};


class ReturnAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> ret_val;

public:
    ReturnAST(std::unique_ptr<BaseAST> &_ret_val);

    void *to_koopa(void);
};

class AssignmentAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;

public:
    AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp);

    void *to_koopa(void);
};

class BranchAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> exp;
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> true_insts;
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> false_insts;

public:
    BranchAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_true_insts);
    BranchAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_true_insts, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_false_insts);
    
    void *to_koopa(void);
};

class ConstDefAST : public BaseAST
{
private:
    std::string ident;
    std::unique_ptr<BaseAST> exp;

public:
    ConstDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp);

    void *to_koopa(void);
};

class VarDefAST : public BaseAST
{
private:
    std::string ident;
    std::unique_ptr<BaseAST> exp;

public:
    VarDefAST(std::string _ident);
    VarDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp);

    void *to_koopa(void);
};

class ExpAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> unary_exp;

public:
    ExpAST(std::unique_ptr<BaseAST> &_unary_exp);

    void *to_koopa(void);
    int value(void);
};

class LValAST : public BaseAST
{
friend class AssignmentAST;

private:
    std::string ident;

public:
    LValAST(std::string _ident);

    void *to_koopa(void);
    int value(void);
};

class PrimaryExpAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> next_exp;

public:
    PrimaryExpAST(std::unique_ptr<BaseAST> &_next_exp);

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
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

    void *to_koopa(void);
    int value(void);
};

class NumberAST : public BaseAST
{
private:
    int val;

public:
    NumberAST(int _val);

    void *to_koopa(void);
    int value(void);
};
