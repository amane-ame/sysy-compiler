#include <fstream>
#include <iostream>
#include <memory>
#include "ast.hpp"
#include "koopa.h"
#include "riscv.hpp"

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

    char buffer[1U << 15];
    size_t sz = 1U << 15;

    std::unique_ptr<CompUnitAST> comp_ast((CompUnitAST *)ast.release());
    koopa_raw_program_t krp = comp_ast->to_koopa_program();
    koopa_program_t kp;
    koopa_generate_raw_to_koopa(&krp, &kp);
    koopa_dump_to_string(kp, buffer, &sz);
    koopa_delete_program(kp);

    if(std::string(mode) == "-riscv")
    {
        koopa_program_t new_kp;
        koopa_parse_from_string(buffer, &new_kp);
        koopa_raw_program_builder_t kp_builder = koopa_new_raw_program_builder();
        koopa_raw_program_t new_krp = koopa_build_raw_program(kp_builder, new_kp);
        koopa_delete_program(new_kp);

        std::string riscv = koopa2riscv(&new_krp);
        buffer[riscv.copy(buffer, riscv.size())] = 0;
    }
    else if(std::string(mode) != "-koopa")
        throw std::runtime_error("error: unknown mode " + std::string(mode));

    std::cout << buffer;
    std::ofstream yyout(output);
    yyout << buffer;

    return 0;
}
