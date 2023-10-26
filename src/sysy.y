%code requires
{
    #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

std::vector<std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>> inst;

void add_inst(InstType inst_type, BaseAST *ast)
{
    inst.back().push_back(make_pair(inst_type, std::unique_ptr<BaseAST>(ast)));

    return;
}

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union
{
    std::string *str_val;
    int int_val;
    BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE
%token <str_val> IDENT UNARYOP MULOP ADDOP RELOP EQOP LANDOP LOROP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block IfExp
%type <ast_val> LVal Number
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit: FuncDef
{
    auto func = std::unique_ptr<BaseAST>($1);
    ast = std::unique_ptr<BaseAST>(new CompUnitAST(func));
};

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef: FuncType IDENT '(' ')' Block
{
    auto type = std::unique_ptr<BaseAST>($1);
    auto ident = std::unique_ptr<std::string>($2);
    auto block = std::unique_ptr<BaseAST>($5);
    $$ = new FuncDefAST(type, *ident, block);
};

// 同上, 不再解释
FuncType: INT
{
    $$ = new FuncTypeAST("int");
};

Block: '{'
{
    inst.push_back(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>());
}
BlockItems '}'
{
    $$ = new BlockAST(inst.back());
    inst.pop_back();
}
| '{' '}'
{
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> insts;
    $$ = new BlockAST(insts);
};

BlockItems: BlockItem | BlockItem BlockItems;
BlockItem: Decl | Stmt;

Stmt: RETURN Exp ';'
{
    auto val = std::unique_ptr<BaseAST>($2);
    add_inst(InstType::STMT, new ReturnAST(val));
}
| LVal '=' Exp ';'
{
    auto lval = std::unique_ptr<BaseAST>($1);
    auto exp = std::unique_ptr<BaseAST>($3);
    add_inst(InstType::STMT, new AssignmentAST(lval, exp));
}
| ';' | Exp ';' | Block
{
    add_inst(InstType::STMT, $1);
}
| IfExp Stmt
{
    auto exp = std::unique_ptr<BaseAST>($1);
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> true_insts;
    for(auto &inst : inst.back())
        true_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    inst.pop_back();
    add_inst(InstType::BRANCH, new BranchAST(exp, true_insts));
}
| IfExp Stmt ELSE
{
    inst.push_back(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>());
}
Stmt
{
    auto exp = std::unique_ptr<BaseAST>($1);
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> true_insts, false_insts;
    for(auto &inst : inst.rbegin()[1])
        true_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    for(auto &inst : inst.back())
        false_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    inst.erase(inst.end() - 2, inst.end());
    add_inst(InstType::BRANCH, new BranchAST(exp, true_insts, false_insts));
};

IfExp: IF '(' Exp ')'
{
    inst.push_back(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>());
    $$ = $3;
};

Decl: ConstDecl | VarDecl;

ConstDecl: CONST INT ConstDefList ';';
ConstDefList: ConstDef | ConstDefList ',' ConstDef
ConstDef: IDENT '=' Exp
{
    auto ident = std::unique_ptr<std::string>($1);
    auto exp = std::unique_ptr<BaseAST>($3);
    add_inst(InstType::CONSTDECL, new ConstDefAST(*ident, exp));
};

VarDecl: INT VarDefList ';';
VarDefList: VarDef | VarDefList ',' VarDef
VarDef: IDENT
{
    auto ident = std::unique_ptr<std::string>($1);
    add_inst(InstType::DECL, new VarDefAST(*ident));
}
| IDENT '=' Exp
{
    auto ident = std::unique_ptr<std::string>($1);
    auto exp = std::unique_ptr<BaseAST>($3);
    add_inst(InstType::DECL, new VarDefAST(*ident, exp));
};

LVal: IDENT
{
    auto ident = std::unique_ptr<std::string>($1);
    $$ = new LValAST(*ident);
};

Exp: LOrExp
{
    auto add_exp = std::unique_ptr<BaseAST>($1);
    $$ = new ExpAST(add_exp);
};

PrimaryExp: '(' Exp ')'
{
    auto exp = std::unique_ptr<BaseAST>($2);
    $$ = new PrimaryExpAST(exp);
}
| Number
{
    auto val = std::unique_ptr<BaseAST>($1);
    $$ = new PrimaryExpAST(val);
};
| LVal
{
    auto lval = std::unique_ptr<BaseAST>($1);
    $$ = new PrimaryExpAST(lval);
};

UnaryExp: PrimaryExp
{
    auto primary_exp = std::unique_ptr<BaseAST>($1);
    $$ = new UnaryExpAST(primary_exp);
}
| UNARYOP UnaryExp
{
    auto op = std::unique_ptr<std::string>($1);
    auto unary_exp = std::unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(*op, unary_exp);
}
| ADDOP UnaryExp
{
    auto op = std::unique_ptr<std::string>($1);
    auto unary_exp = std::unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(*op, unary_exp);
};

MulExp: UnaryExp
{
    auto unary_exp = std::unique_ptr<BaseAST>($1);
    $$ = new MulExpAST(unary_exp);
}
| MulExp MULOP UnaryExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new MulExpAST(left_exp, *op, right_exp);
};

AddExp: MulExp
{
    auto mul_exp = std::unique_ptr<BaseAST>($1);
    $$ = new MulExpAST(mul_exp);
}
| AddExp ADDOP MulExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new AddExpAST(left_exp, *op, right_exp);
};

RelExp: AddExp
{
    auto add_exp = std::unique_ptr<BaseAST>($1);
    $$ = new RelExpAST(add_exp);
}
| RelExp RELOP AddExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new RelExpAST(left_exp, *op, right_exp);
};

EqExp: RelExp
{
    auto rel_exp = std::unique_ptr<BaseAST>($1);
    $$ = new EqExpAST(rel_exp);
}
| EqExp EQOP RelExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new EqExpAST(left_exp, *op, right_exp);
};

LAndExp: EqExp
{
    auto eq_exp = std::unique_ptr<BaseAST>($1);
    $$ = new LAndExpAST(eq_exp);
}
| LAndExp LANDOP EqExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new LAndExpAST(left_exp, *op, right_exp);
};

LOrExp: LAndExp
{
    auto land_exp = std::unique_ptr<BaseAST>($1);
    $$ = new LOrExpAST(land_exp);
}
| LOrExp LOROP LAndExp
{
    auto left_exp = std::unique_ptr<BaseAST>($1);
    auto op = std::unique_ptr<std::string>($2);
    auto right_exp = std::unique_ptr<BaseAST>($3);
    $$ = new LOrExpAST(left_exp, *op, right_exp);
};

Number: INT_CONST
{
    $$ = new NumberAST($1);
};

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s)
{
    std::cerr << "error: " << s << std::endl;
    exit(0);
}
