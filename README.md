# -
华科编译原理实验2017级实验4

生成目标代码
18级实验一，直接把.l .y文件翻译翻译就能过

ast.c创建抽象语法树对应词法语法分析\n
analysis.c对应语义分析和中间代码生成\n
targetcode.c对应目标代码生成\n
\n
代码优化很好混，代码不贴了

flex-2.5.4a\n
bison-2.4.1\n
gcc-6.3.0\n
\n
//build\n
flex lex.l\n
bison -d parser.y\n
==>lex.yy.c parser.tab.c parser.tab.h\n

gcc -o 即可
