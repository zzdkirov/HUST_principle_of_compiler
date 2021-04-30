# -
华科编译原理实验2017级实验4

生成目标代码

18级实验一，实验报告里面有相关内容

ast.c:创建抽象语法树对应词法语法分析,实验2，实验3

analysis.c 对应语义分析和中间代码生成，实验4，实验5，实验6

targetcode.c 对应目标代码生成实验8

实验7代码优化很好混

flex-2.5.4a

bison-2.4.1

gcc-6.3.0

//build

flex lex.l

bison -d parser.y

gcc -o ... lex.yy.c parser.tab.c parser.tab.h...

仅供参考，造福各位^_^
