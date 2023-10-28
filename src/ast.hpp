#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "koopa.h"
#include "table.hpp"

enum InstType
{
    CONSTDECL,
    DECL,
    ARRAYDECL,
    STMT,
    BRANCH,
    WHILE,
    BREAK,
    CONTINUE
};

// 所有 AST 的基类
class BaseAST
{
friend class BlockInst;

protected:
    static SymbolList symbol_list;
    static BlockInst block_inst;
    static std::vector<std::tuple<koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *, koopa_raw_basic_block_data_t *>> loop_inst;

    const void **vector_data(std::vector<void *> &vec);
    char *string_data(std::string s);
    koopa_raw_type_kind *array_data(std::vector<int> &sz, int pos);

public:
    virtual ~BaseAST(void) = default;
    virtual void *to_koopa(void);
    virtual int value(void);
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
private:
    std::vector<std::unique_ptr<BaseAST>> func_vec;
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> value_vec;

    void libfuncs(std::vector<void*> &funcs);

public:
    CompUnitAST(std::vector<std::unique_ptr<BaseAST>> &_func_vec, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_value_vec);

    koopa_raw_program_t to_koopa_program(void);
};

class FuncTypeAST : public BaseAST
{
private:
    std::string ident;

public:
    FuncTypeAST(std::string _ident);

    void *to_koopa(void);
};

class FuncFParamAST : public BaseAST
{
friend class FuncDefAST;

public:
    enum ParamType
    {
        INT,
        ARRAY
    } type;

private:
    std::string ident;
    int index;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;

public:
    FuncFParamAST(ParamType _type, std::string _ident, int _index);
    FuncFParamAST(ParamType _type, std::string _ident, int _index, std::vector<std::unique_ptr<BaseAST>> &_sz_exp);

    koopa_raw_type_kind *get_type(void);
    void *to_koopa(void);
};

class FuncDefAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::vector<std::unique_ptr<BaseAST>> fparams;
    std::unique_ptr<BaseAST> block;

public:
    FuncDefAST(std::unique_ptr<BaseAST> &_func_type, std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_fparams, std::unique_ptr<BaseAST> &_block);

    void *to_koopa(void);
};

class BlockAST : public BaseAST
{
friend class FuncDefAST;

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
    ReturnAST(void);
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

class WhileAST : public BaseAST
{
private:
    std::unique_ptr<BaseAST> exp;
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> body_insts;

public:
    WhileAST(std::unique_ptr<BaseAST> &_exp, std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_body_insts);

    void *to_koopa(void);
};

class BreakAST : public BaseAST
{
public:
    void *to_koopa(void);
};

class ContinueAST : public BaseAST
{
public:
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
friend class GlobalVarDefAST;

private:
    std::string ident;
    std::unique_ptr<BaseAST> exp;

public:
    VarDefAST(std::string _ident);
    VarDefAST(std::string _ident, std::unique_ptr<BaseAST> &_exp);

    void *to_koopa(void);
};

class GlobalVarDefAST : public BaseAST
{
private:
    std::string ident;
    std::unique_ptr<BaseAST> exp;

public:
    GlobalVarDefAST(std::unique_ptr<BaseAST> &vardef_ast);

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
    enum ValType
    {
        NUM,
        ARRAY
    } type;
    std::string ident;
    std::vector<std::unique_ptr<BaseAST>> idx_vec;

public:
    LValAST(std::string _ident);
    LValAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_idx_vec);

    void *left_value(void);
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
        OP,
        FUNCTION
    } type;
    std::string op;
    std::unique_ptr<BaseAST> next_exp;
    std::vector<std::unique_ptr<BaseAST>> rparams;

public:
    UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp);
    UnaryExpAST(std::string _op, std::unique_ptr<BaseAST> &_unary_exp);
    UnaryExpAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_rparams);

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

class InitValAST : public BaseAST
{
friend class GlobalArrayDefAST;

private:
    enum
    {
        EXP,
        ARRAY
    } type;
    bool is_const;
    std::unique_ptr<BaseAST> exp;
    std::vector<std::unique_ptr<BaseAST>> arr_vec;

    std::vector<koopa_raw_value_t> cache;

public:
    InitValAST(std::unique_ptr<BaseAST> &_exp);
    InitValAST(std::vector<std::unique_ptr<BaseAST>> &_arr_list);

    void sub_preprocess(std::vector<int> &pro, int align, std::vector<koopa_raw_value_t> &buf);
    void preprocess(const std::vector<int> &sz);

    koopa_raw_value_t index(int idx);
    koopa_raw_value_t sub_make_aggerate(std::vector<int> &sz, std::vector<int> &pro, int align, std::vector<koopa_raw_value_t> &buf, int pos);
    koopa_raw_value_t make_aggerate(std::vector<int> &sz);
};

class ArrayDefAST : public BaseAST
{
friend class GlobalArrayDefAST;

private:
    std::string ident;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;
    std::unique_ptr<BaseAST> init_val;

    koopa_raw_value_data *index(int i, std::vector<int> &pro, koopa_raw_value_data *src, int pos);

public:
    ArrayDefAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_exp);
    ArrayDefAST(std::string _ident, std::vector<std::unique_ptr<BaseAST>> &_exp, std::unique_ptr<BaseAST> &_init_val);

    void *to_koopa(void);
};

class GlobalArrayDefAST : public BaseAST
{
private:
    std::string ident;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;
    std::unique_ptr<BaseAST> init_val;

public:
    GlobalArrayDefAST(std::unique_ptr<BaseAST> &arraydef_ast);

    void *to_koopa(void);
};
