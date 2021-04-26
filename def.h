#ifndef __DEF_H__
#define __DEF_H__

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"

#include "parser.tab.h"

#define MAXLENGTH 300
#define DX sizeof(int)

//DX为返回地址

enum node_kind{EXTLIST,VAR,VARDEC,ARRDEC,LOCALVAR,FUNCDEF,FUNCDEC,FUNCHEAD,VARLIST,VARNAME,PARAMLIST,PARAM,ARRNAME,ARRLTH,DIMELIST,STRUCTNAME,
ARRINITIAL,STRUCTDEF,STRUCTDEC,LOCALSTRUCTDEF,NUMBERLIST,STRUCTCALL,FUNCNAME,COMPSTAT,STATLIST,EXP,FOR1,FOR2,FOR3,FUNCCALL,ARRAYCALL,CALLDIMELIST
,VARCALL,ARGS};

enum IR_kind{FUNC_IR=34,PARAM_IR,ASSIGN_IR,ARG_IR,CALL_IR,ARR_IR,STRUCT_IR,RETURN_IR,ID_IR,IF_IR,LABEL_IR,GOTO_IR,
EQ_IR,NE_IR,GT_IR,GE_IR,LT_IR,LE_IR,END_IR};


//以下语法树结点类型、三地址结点类型等定义仅供参考，实验时一定要根据自己的理解来定义


struct node {

	enum node_kind kind;               //结点类型
	union {
		  char type_id[33];       //由标识符生成的叶结点
		  int type_int;           //由整常数生成的叶结点
		  char type_char;         //由字符常数生成的叶节点
		  float type_float;       //由浮点常数生成的叶结点
	      };
    struct node *ptr[4];        //子节点
    struct node *parent;		//父节点
    int pos;                     //语法单位所在位置行号
	int type;                      //用以标识表达式结点的类型
	int num;                      //计数器，可以用来统计形参个数
	char Etrue[15],Efalse[15];		//对布尔表达式的翻译时，真假转移目标的标号
	char Snext[15];               //结点对应语句S执行后的下一条语句位置标号
	struct codenode *code;          //该结点中间代码链表头指针
	int offset;                     //偏移量
	int place;                     //存放（临时）变量在符号表的位置序号
    int width;                     //占数据字节数
    };



struct symbol{
    char name[33];   //变量或函数名
    int level;        //层号，复合语句复合时指示变量是否重复
    char alias[10];     //别名
    int type;         //变量类型或函数返回值类型，-2:void 0：int，1：char，2：float
    int paramnum;
	//对函数适用，记录形式参数个数，对于成员变量，指示其结构体的声明下标，对数组，指示其维数，对结构变量，指向其声明下标
	//对参数，指示其函数头的下标
    char flag;       //符号标记，函数定义：'F' 函数原型：'D'  变量：'V'  参数：'P' 数组：'A' 结构名：'S' 结构变量名：'B'
					 //成员变量：'N'（不支持成员数组） 临时变量：'T'
	int offset;     //对于其他变量是偏移量，全局变量相对于data段首址偏移，局部变量相对栈帧偏移，
	//对于结构声明，是结构体长度，对于函数定义，是所有局部变量之和
    int local;
};

//符号表

struct symbol_table{
    struct symbol symbols[ MAXLENGTH ];
    int index;//index指向的是下一个写入符号表的位置
};

struct opn{
    int kind;                  //标识操作的类型
    int type;                  //标识操作数的类型
    int const_int;      //整常数值，立即数
    char id[33];        //变量或临时变量的别名或标号字符串
    int level;                 //变量的层号，0表示是全局变量，数据保存在静态数据区
    int offset;                 //变量单元偏移量，或函数在符号表的定义位置序号，目标代码生成时用
    //对于函数定义而言，这个相当于栈帧的大小，在result存放
};
struct codenode {   //三地址TAC代码结点,采用单链表存放中间语言代码
        int  op;
        char flag;
        struct opn opn1;
        struct opn opn2;
        struct opn result;
        struct codenode  *next,*prior;
};

struct registerSymbol{
    char tempvar[10];
    int reg;
    int addrreg;
    int flag;//0:非地址，1:存储有地址
};
//实验一函数
struct node *mknode(int kind,struct node *first, struct node *second, struct node *third, struct node *forth, int pos );

//实验二函数
void errormsg(int No,int line, char *msg1, char *msg2);
int localRedec(char* name,int LV);
int redec(char* name,char type,int master);
int Exp(struct node* T);
void LocalVar(struct node *T);
void Stat(struct node *T);
void CompoundStat(struct node *T);
void StructDef(struct node *T);
void StructDec(struct node *T);
void FuncDec(struct node *T);
void FuncDef(struct node *T);
void VarDef(struct node *T);
void ExtDefList(struct node *T);
int fillSymbolTable(char *name,char* alias,int level, int type, char flag,int local,int offset);
int searchTarget(char *name,char flag);
int parammatch(int dec,int def);

//实验三函数
int getresult(struct node* T);
struct codenode *genIR(int op,struct opn opn1,struct opn opn2,struct opn result);
struct codenode *merge(int num,...);
struct codenode *genGoto(char *label);
struct codenode *genLabel(char *label);

//实验四函数
void push(int rd);
void pop(int rd);
void ble(int rd,int rt,char* s);
void blt(int rd,int rt,char* s);
void bge(int rd,int rt,char* s);
void bgt(int rd,int rt,char* s);
void bne(int rd,int rt,char* s);
void beq(int rd,int rt,char* s);
void jal(char* s);
void j(char* s);
void jr(int rd);
void or(int rd,int rs,int rt);
void and(int rd,int rs,int rt);
void srlv(int rd,int rs,int rt);
void sllv(int rd,int rs,int rt);
void sub(int rd,int rs,int rt);
void add(int rd,int rs,int rt);
void subi(int rd,int rs,int imm);
void addi(int rd,int rs,int imm);
void label(char* s);
void sw(int rd,int rt,int offset,int base);
void lw(int rd,int rt,int offset,int base);
int getBaseBlock(struct codenode* root);
int getopnum(char* tvar);
void revokeRegister(int reg);
int allocRegister();
int getframesize(struct codenode* root,char* s);
void generateTargetCode(struct codenode* root);
void recordcode();



#endif // DEF_H_INCLUDED
