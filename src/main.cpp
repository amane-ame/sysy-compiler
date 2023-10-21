#include <fstream>
#include <iostream>
#include <memory>
#include "ast.hpp"
#include "koopa.h"

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[])
{
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    if(argc != 5)
        return 1;

    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];
  
    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
  
    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    std::unique_ptr<BaseAST> ast;
    yyparse(ast);
  
    // 输出解析得到的 AST, 其实就是个字符串
    std::cout << ast->to_string() << std::endl;

    std::unique_ptr<CompUnitAST> comp_ast((CompUnitAST *)ast.release());
    koopa_raw_program_t res = comp_ast->to_koopa_program();

    koopa_program_t kp;
    koopa_generate_raw_to_koopa(&res, &kp);

    char buffer[1U << 15];
    size_t sz = 1U << 15;
    koopa_dump_to_string(kp, buffer, &sz);

    std::cout << buffer << std::endl;
    std::ofstream yyout(output);
    yyout << buffer;

    return 0;
}
