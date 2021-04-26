%error-verbose
%locations
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"

extern int yylineno;
extern char *yytext;
extern FILE *yyin;
void yyerror(const char* fmt, ...);
void display(struct node *,int);
%}

%union {
	int    type_int;
	float  type_float;
	char  type_char;
	char   type_id[32];
	struct node *ptr;
};

//  %type 定义非终结符
%type  <ptr> program ExtDefList Var LocalVar Func FuncDef Specifier FuncDec VarList VarName ArrName ArrayIni DimetList StructDef StructDec StructVarList FuncName ParamList ParamDec CompStat StatList Stat Exp Args For1 For2 For3 Void VarDec ArrDec StructNumber NumberList StrConst NumberVarList CallDimetList 

//% token 定义终结符
%token <type_int> INT_CONST              //指定INT的语义值是type_int，由词法分析得到的数值
%token <type_id> ID RELOP DATATYPE VOID  //指定ID,RELOP 的语义值是type_id，由词法分析得到的标识符字符串
%token <type_float> FLOAT_CONST 
%token <type_char> CHAR_CONST   
%token <type_id> STRING_CONST 

%token LP RP LB RB LC RC SEMI COMMA DOT //用bison对该文件编译时，带参数-d，生成的exp.tab.h中给这些单词进行编码，可在lex.l中包含parser.tab.h使用这些单词种类码
%token PLUS MINUS MUL DIV ASSIGN AND OR NOT IF ELSE WHILE RETURN STRUCT FOR BREAK CONTINUE ASSIGN_PLUS INC ASSIGN_MINUS DEC ASSIGN_MUL ASSIGN_DIV MOD ASSIGN_MOD RSHFIT ASSIGN_RSHFIT LSHFIT ASSIGN_LSHFIT 

%right ASSIGN ASSIGN_PLUS ASSIGN_MINUS ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD ASSIGN_RSHFIT ASSIGN_LSHFIT
%left OR
%left AND
%left RELOP
%left RSHFIT LSHFIT
%left PLUS MINUS
%left MUL DIV MOD
%right UMINUS NOT INC DEC
%nonassoc LOWER_THAN_RB
%left LP RP LB RB 
%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%%
/*程序作为入口，无需语法节点*/
program: ExtDefList    { display($1,0);ExtDefList($1);}     /*显示语法树,第一次实验不需要语义分析*/
         ; 
		 
/*外部定义序列语法节点为一序列语法节点，其孩子为具体的五类定义*/
ExtDefList: {$$=NULL;}
          | Var ExtDefList {$$=mknode(EXTLIST,$1,$2,NULL,NULL,yylineno);}
          | FuncDec ExtDefList {$$=mknode(EXTLIST,$1,$2,NULL,NULL,yylineno);}
          | Func ExtDefList {$$=mknode(EXTLIST,$1,$2,NULL,NULL,yylineno);}
		  | StructDec ExtDefList {$$=mknode(EXTLIST,$1,$2,NULL,NULL,yylineno);}
		  | StructDef ExtDefList {$$=mknode(EXTLIST,$1,$2,NULL,NULL,yylineno);}
          ;
		  
/*局部变量节点，其节点类型为VAR，父节点为外部定义序列或语句序列,子树分别为变量类型和变量序列*/
Var:     Specifier VarList SEMI   {$$=mknode(VAR,$1,$2,NULL,NULL,yylineno);}   //该结点对应一个外部变量声明
         | error SEMI   {$$=NULL; }
         ;
		 
/*函数定义节点，节点类型为FUNCDEF，父节点为外部定义序列，子树分别为函数类型，函数定义，函数体（复合语句）*/
Func:  Specifier FuncDef CompStat   {$$=mknode(FUNCDEF,$1,$2,$3,NULL,yylineno);}
		| Void FuncDef CompStat   {$$=mknode(FUNCDEF,$1,$2,$3,NULL,yylineno);}
		;

/*函数声明节点，节点类型为FUNCDEC，父节点为外部定义序列，子树分别为函数类型，函数定义*/
FuncDec: Specifier FuncDef SEMI  {$$=mknode(FUNCDEC,$1,$2,NULL,NULL,yylineno);}
		 | Void FuncDef SEMI     {$$=mknode(FUNCDEC,$1,$2,NULL,NULL,yylineno);}
		 
		 ;
		 
/*变量或函数类型节点，节点类型为DATATYPE，父节点为变量或函数声明定义，无子树，节点存储变量的类型（int/float/char）*/
Specifier:  DATATYPE {$$=mknode(DATATYPE,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
         ;

/*Void节点，增加了VOID*/
Void: VOID  {$$=mknode(DATATYPE,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"void");}
		 ;
				
/*变量序列节点，节点类型为VARLIST，父节点为（局部或外部）变量，子树分为多种情况*/
VarList:  VarDec {$$=mknode(VARLIST,$1,NULL,NULL,NULL,yylineno);}
		  |ArrDec {$$=mknode(VARLIST,$1,NULL,NULL,NULL,yylineno);}
		  |VarDec COMMA VarList {$$=mknode(VARLIST,$1,$3,NULL,NULL,yylineno);}
		  |ArrDec COMMA VarList {$$=mknode(VARLIST,$1,$3,NULL,NULL,yylineno);}
		 ;

VarDec:   /*单一的变量未初始化，则Varlist节点的子节点唯一为变量名节点*/
		  VarName  {$$=mknode(VARDEC,$1,NULL,NULL,NULL,yylineno);}
		  
		  /*单一变量初始化，则Varlist节点的子节点1为变量名节点，子节点2为初始值表达式节点*/
		  | VarName ASSIGN Exp {$$=mknode(VARDEC,$1,$3,NULL,NULL,yylineno);}
		 ;
		   
ArrDec:   /*单一的数组未初始化，则Varlist节点的子节点1为数组名节点，子节点2为数组长度节点*/
		  ArrName DimetList {$$=mknode(ARRDEC,$1,$2,NULL,NULL,yylineno);}
		  
		  /*单一的数组初始化，则Varlist节点的子节点1为数组名节点，子节点2为数组长度节点，子节点3位数组初始化序列节点*/
		  | ArrName DimetList ASSIGN LC ArrayIni RC {$$=mknode(ARRDEC,$1,$2,$5,NULL,yylineno);}
		  
		  | ArrName DimetList ASSIGN StrConst {$$=mknode(ARRDEC,$1,$2,$4,NULL,yylineno);}
		 ;

/*多维数组维度序列*/
DimetList: LB Exp RB {$$=mknode(DIMELIST,$2,NULL,NULL,NULL,yylineno);}
		  | LB Exp RB DimetList {$$=mknode(DIMELIST,$2,$4,NULL,NULL,yylineno);}
		 ;
		 
/*变量名节点，节点类型为VARNAME，父节点为变量序列，子树为存储VARNAME的标识符节点*/
VarName:  ID {$$=mknode(VARNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
         ;

/*数组名节点，节点类型为ARRNAME，父节点为变量序列，子树为存储ARRNAME的标识符节点*/
ArrName:  ID {$$=mknode(ARRNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
		 ;
		 
/*数组初始化序列节点，节点类型为ARRINITIAL，父节点为变量序列，子树为参数序列，即表达式序列*/
ArrayIni: LC ArrayIni RC {$$=mknode(ARRINITIAL,$2,NULL,NULL,NULL,yylineno);}
		  | LC ArrayIni RC COMMA ArrayIni {$$=mknode(ARRINITIAL,$2,$5,NULL,NULL,yylineno);}
		  | Args {$$=mknode(ARRINITIAL,$1,NULL,NULL,NULL,yylineno);}
		 ;

/*结构体声明*/
StructDec: STRUCT ID LC NumberList RC SEMI {$$=mknode(STRUCTDEC,$4,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$2);}
		 ;
		 
NumberList: Specifier NumberVarList SEMI {$$=mknode(NUMBERLIST,$1,$2,NULL,NULL,yylineno);}
			|Specifier NumberVarList SEMI NumberList{$$=mknode(NUMBERLIST,$1,$2,$4,NULL,yylineno);}
		 ;
		 
NumberVarList: ID {$$=mknode(VARNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
			   | ID COMMA NumberVarList {$$=mknode(VARNAME,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
			   ;
			   
/*结构变量定义*/
StructDef: STRUCT ID StructVarList SEMI {$$=mknode(STRUCTDEF,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$2);}
		 ;
		 
StructVarList: ID {$$=mknode(STRUCTNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
			  | ID COMMA StructVarList {$$=mknode(STRUCTNAME,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
			  ;

		 
/*函数头（名，参数序列）节点，节点类型为FUNCHEAD，父节点为函数定义节点，子树1为函数名节点，子树2位参数列表节点，可以支持无参函数*/
FuncDef:  FuncName LP ParamList RP   {$$=mknode(FUNCHEAD,$1,$3,NULL,NULL,yylineno);}
		|FuncName LP RP   {$$=mknode(FUNCHEAD,$1,NULL,NULL,NULL,yylineno);}
		|FuncName LP Void RP   {$$=mknode(FUNCHEAD,$1,NULL,NULL,NULL,yylineno);}
         ;

/*变量名节点，节点类型为FUNCNAME，父节点为变量序列，子树为存储FUNCNAME的标识符节点*/
FuncName: ID {$$=mknode(FUNCNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
		 ;

/*形参声明序列节点，节点类型为PARAMLIST，父节点为函数头结点，子树为形参声明或形参声明序列*/
ParamList: ParamDec  {$$=mknode(PARAMLIST,$1,NULL,NULL,NULL,yylineno);}
         | ParamDec COMMA  ParamList  {$$=mknode(PARAMLIST,$1,$3,NULL,NULL,yylineno);}
        ;
		
/*形参声明节点，节点类型为PARAM，父节点为形参声明序列节点，子树为形参类型和标识符，或数组相关定义*/
ParamDec: Specifier VarName  {$$=mknode(PARAM,$1,$2,NULL,NULL,yylineno);}
		  |Specifier ArrName LB Exp RB {$$=mknode(PARAM,$1,$2,$4,NULL,yylineno);}
         ;

/*复合语句节点，节点类型为COMPSTAT，父节点为函数定义节点或其它合适位置节点，子树为语句序列节点*/
CompStat: LC StatList RC    {$$=mknode(COMPSTAT,$2,NULL,NULL,NULL,yylineno);}
       ;

/*语句序列节点，节点类型为STATLIST，父节点为复合语句节点，子树为变量节点*/
StatList: LocalVar {$$=mknode(STATLIST,$1,NULL,NULL,NULL,yylineno);}
		  |LocalVar StatList {$$=mknode(STATLIST,$1,$2,NULL,NULL,yylineno);}
		  |Stat {$$=mknode(STATLIST,$1,NULL,NULL,NULL,yylineno);}
		  |Stat StatList {$$=mknode(STATLIST,$1,$2,NULL,NULL,yylineno);}
		  ;
		  
/*局部变量节点，其节点类型为LOCALVAR，父节点为语句序列,子树分别为变量类型和变量序列*/
LocalVar: Specifier VarList SEMI   {$$=mknode(LOCALVAR,$1,$2,NULL,NULL,yylineno);}   //该结点对应一个局部变量声明
		 | STRUCT ID StructVarList SEMI {$$=mknode(LOCALSTRUCTDEF,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$2);}
         ;
		 
		   
/*语句节点，其节点类型多样，父节点为语句序列,子树多样*/
Stat:   Exp SEMI {$$=mknode(EXP,$1,NULL,NULL,NULL,yylineno);}
      | CompStat {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
      | RETURN Exp SEMI {$$=mknode(RETURN,$2,NULL,NULL,NULL,yylineno);}
	  | RETURN SEMI {$$=mknode(RETURN,NULL,NULL,NULL,NULL,yylineno);}
      | IF LP Exp RP Stat %prec LOWER_THEN_ELSE {$$=mknode(IF,$3,$5,NULL,NULL,yylineno);}
      | IF LP Exp RP Stat ELSE Stat {$$=mknode(IF,$3,$5,$7,NULL,yylineno);}
      | WHILE LP Exp RP Stat {$$=mknode(WHILE,$3,$5,NULL,NULL,yylineno);}
	  | BREAK SEMI {$$=mknode(BREAK,NULL,NULL,NULL,NULL,yylineno);}
	  | CONTINUE SEMI {$$=mknode(CONTINUE,NULL,NULL,NULL,NULL,yylineno);}
	  | FOR LP For1 SEMI For2 SEMI For3 RP Stat {$$=mknode(FOR,$3,$5,$7,$9,yylineno);}
	  | SEMI {$$=NULL;}
	  | error SEMI {$$=NULL; }
      ;

For1: Specifier VarList {$$=mknode(FOR1,$1,$2,NULL,NULL,yylineno);}
	  |Exp {$$=mknode(FOR1,$1,NULL,NULL,NULL,yylineno);}
	  ;
	  
For2: Exp {$$=mknode(FOR2,$1,NULL,NULL,NULL,yylineno);}
	  ;
	  
For3: Exp {$$=mknode(FOR3,$1,NULL,NULL,NULL,yylineno);}
	  ;

Exp:    Exp ASSIGN Exp {$$=mknode(ASSIGN,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"=");}
      | Exp ASSIGN_PLUS Exp {$$=mknode(ASSIGN_PLUS,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"+=");}
      | Exp ASSIGN_MINUS Exp {$$=mknode(ASSIGN_MINUS,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"-=");}
      | Exp ASSIGN_MUL Exp {$$=mknode(ASSIGN_MUL,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"*=");}
      | Exp ASSIGN_DIV Exp {$$=mknode(ASSIGN_DIV,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"/=");}
      | Exp ASSIGN_MOD Exp {$$=mknode(ASSIGN_MOD,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"%=");}
	  | Exp ASSIGN_LSHFIT Exp   {$$=mknode(ASSIGN_LSHFIT,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"<<=");}
      | Exp ASSIGN_RSHFIT Exp   {$$=mknode(ASSIGN_RSHFIT,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,">>=");}
      | Exp AND Exp   {$$=mknode(AND,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"&&");}
      | Exp OR Exp    {$$=mknode(OR,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"||");}
      | Exp RELOP Exp {$$=mknode(RELOP,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,$2);}  //词法分析关系运算符号自身值保存在$2中
      | Exp PLUS Exp  {$$=mknode(PLUS,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"+");}
      | Exp MINUS Exp {$$=mknode(MINUS,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"-");}
      | Exp MUL Exp  {$$=mknode(MUL,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"*");}
      | Exp DIV Exp   {$$=mknode(DIV,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"/");}
      | Exp MOD Exp   {$$=mknode(MOD,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"/");}
      | Exp LSHFIT Exp   {$$=mknode(LSHFIT,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,"<<");}
      | Exp RSHFIT Exp   {$$=mknode(RSHFIT,$1,$3,NULL,NULL,yylineno);strcpy($$->type_id,">>");}
      | LP Exp RP     {$$=$2;}
	  | Exp INC  {$$=mknode(INC,$1,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"++");}
      | INC Exp  {$$=mknode(INC,NULL,$2,NULL,NULL,yylineno);strcpy($$->type_id,"++");}
      | Exp DEC  {$$=mknode(DEC,$1,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"--");}
      | DEC Exp  {$$=mknode(DEC,NULL,$2,NULL,NULL,yylineno);strcpy($$->type_id,"--");}
      | MINUS Exp %prec UMINUS   {$$=mknode(MINUS,$2,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"-");}
      | NOT Exp       {$$=mknode(NOT,$2,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"!");}
      | ID LP Args RP {$$=mknode(FUNCCALL,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID LP RP      {$$=mknode(FUNCCALL,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID CallDimetList  {$$=mknode(ARRAYCALL,$2,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID            {$$=mknode(VARCALL,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
	  | StrConst      {$$=$1}					
	  | ID DOT StructNumber {$$=mknode(STRUCTCALL,$3,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | INT_CONST     {$$=mknode(INT_CONST,NULL,NULL,NULL,NULL,yylineno);$$->type_int=$1;}
      | FLOAT_CONST   {$$=mknode(FLOAT_CONST,NULL,NULL,NULL,NULL,yylineno);$$->type_float=$1;}  
      | CHAR_CONST    {$$=mknode(CHAR_CONST,NULL,NULL,NULL,NULL,yylineno);$$->type_char=$1;}
      ;

StrConst: STRING_CONST {$$=mknode(STRING_CONST,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}

/*多维数组调用序列*/
CallDimetList: LB Exp RB {$$=mknode(CALLDIMELIST,$2,NULL,NULL,NULL,yylineno);}
		  | LB Exp RB CallDimetList {$$=mknode(CALLDIMELIST,$2,$4,NULL,NULL,yylineno);}
	  
	     
StructNumber: ID {$$=mknode(STRUCTNAME,NULL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
		 ;

Args:    Exp COMMA Args    {$$=mknode(ARGS,$1,$3,NULL,NULL,yylineno);}
       | Exp               {$$=mknode(ARGS,$1,NULL,NULL,NULL,yylineno);}
       ;
       
%%

int main(int argc, char *argv[]){
	yyin=fopen(argv[1],"r");
	if (!yyin) return;
	yylineno=1;
	yyparse();
	return 0;
	}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}
