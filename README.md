# -
华科编译原理实验2017级实验4

生成目标代码
20级实验一，直接把.l .y文件翻译翻译就能过

ast.c创建抽象语法树对应词法语法分析
analysis.c对应语义分析



flex-2.5.4a
bison-2.4.1
gcc-6.3.0

//build
flex lex.l
bison -d parser.y
==>lex.yy.c parser.tab.c parser.tab.h

gcc -o 即可
