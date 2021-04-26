# -
华科编译原理实验2017级实验4

flex-2.5.4a
bison-2.4.1
gcc-6.3.0

//build
flex lex.l
bison -d parser.y
==>lex.yy.c parser.tab.c parser.tab.h

gcc -o 即可
