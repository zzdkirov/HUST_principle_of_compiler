#include "def.h"

/*中间代码生成，去掉浮点和字符型变量,所有代码均只涉及到int型变量，常量，数组，函数，结构*/
/*加入了0号语义错误，即使用了非int型变量*/

/*外部变量：要添加偏移量，对于节点添加节点的位宽*/

/*局部变量：同外部变量*/

/*语句：翻译对应的语句*/

/*表达式：参考表达式处理，自顶向下处理优先级表达式*/

/*数组，由于只支持一维数组，对于下标而言，将其视为一个临时变量，处理数组的中间代码，即偏移量加临时变量（偏移量）双加，也视为，这与变量一致*/

/*结构，同数组理，将结构体的调用视为临时变量，由结构体变量的偏移量加上偏移量而得*/

/*个人的做法是，先将所有的中间代码生成，然后对于复合语句进行连接，对外部函数定义再进行连接，*/

/*temp变量在指代变量（执行赋值写入时）指代的是变量地地址*/

/*
实验二内容：
（1）使用未定义的变量；
（2）调用未定义或未声明的函数；
（3）在同一作用域，名称的重复定义（如变量名、函数名、结构类型名以及结构体成员名等）。
为更清楚说明语义错误，这里也可以拆分成几种类型的错误，如变量重复定义、函数重复定义、结构体成员名重复等；
（4）对非函数名采用函数调用形式；
（5）对函数名采用非函数调用形式访问；
（6）函数调用时参数个数不匹配，如实参表达式个数太多、或实参表达式个数太少；
（7）函数调用时实参和形参类型不匹配；
（8）对非数组变量采用下标变量的形式访问；
（9）数组变量的下标不是整型表达式；
（10）对非结构变量采用成员选择运算符“.”；
（11）结构成员不存在；
（12）赋值号左边不是左值表达式；
（13）对非左值表达式进行自增、自减运算；
（14）对结构体变量进行自增、自减运算；
（15）类型不匹配。如数组名与结构变量名间的运算，需要指出类型不匹配错误；有些需要根据定义的语言的语义自行进行界定，比如：32+'A'，10*12.3，如果使用强类型
规则，则需要报错，如果按C语言的弱类型规则，则是允许这类运算的，但需要在后续阶段需要进行类型转换，类型统一后再进行对应运算；
（16）函数返回值类型与函数定义的返回值类型不匹配；
（17）函数没有返回语句（当函数返回值类型不是void时）；
（18）break语句不在循环语句或switch语句中；
（19）continue语句不在循环语句中；
*/
int NoIRgenerate;

int ifincir;	//计数器指示是否在循环中

int LV = 0;   //层号

int func_size; //1个函数的活动记录大小

int functype;//当前函数的类型

int ifreturn;

int nowfunc=-1;//指示当前函数的下标，用于参数变量判断

struct symbol_table symbolTable;

int globaloff;//全局变量偏移量

int localoff=4;//局部变量偏移量，栈帧开头是ebp，因此局部变量偏移量初始就为4

int nowfuncparanum;//统计当前函数参数值

int funcwidth=0;

struct opn op1;
struct opn op2;
struct opn result;

struct codenode* Intermidiate;

int expresultindex;//记录表达式结果的对应符号（或变量）在符号表下标，对于表达式而言，这个一定是个临时变量，因为对应的是寄存器

struct codenode* root=NULL;

struct codenode* target=NULL;

//#define DISPSYMBOL
//显示符号表
void dispsymbol()
{
#ifdef DISPSYMBOL
    int i = 0;
    char *name;
    char symboltype[20];
    int lv;
    char type[10];
    short scope;
    char *master;
    printf("%s\t%s\t%s\t%s\t%s\t%s\n", "符号名","符号类型", "层 号", "类  型", "作用域","所  属");
    for (i = 0; i < symbolTable.index; i++)
    {
        name=symbolTable.symbols[i].name;
        switch(symbolTable.symbols[i].flag)
        {
            case 'D':
                strcpy(symboltype,"FuncDeclare");
                master=NULL;
                break;
            case 'F':
                strcpy(symboltype,"FuncDefine");
                master=NULL;
                break;
            case 'V':
                strcpy(symboltype,"Variable");
                master=NULL;
                break;
            case 'A':
                strcpy(symboltype,"ArrayVariable");
                master=NULL;
                break;
            case 'B':
                strcpy(symboltype,"StructVar");
                master=NULL;
                break;
            case 'S':
                strcpy(symboltype,"StructDec");
                master=NULL;
                break;
            case 'P':
                strcpy(symboltype,"FuncParam");
                master=symbolTable.symbols[symbolTable.symbols[i].paramnum].name;
                break;
            case 'N':
                strcpy(symboltype,"StructMumber");
                master=symbolTable.symbols[symbolTable.symbols[i].paramnum].name;
                break;
            default:
                break;
        }
        lv=symbolTable.symbols[i].level;
        switch(symbolTable.symbols[i].type)
        {
            case -2:
                strcpy(type,"void");
                break;
            case -1:
                strcpy(type,"struct");
                break;
            case 0:
                strcpy(type,"int");
                break;
            case 1:
                strcpy(type,"char");
                break;
            case 2:
                strcpy(type,"float");
                break;
            default:
                break;
        }
        scope=symbolTable.symbols[i].local;
        printf("%s",name);
        printf("\t%s",symboltype);
        printf("\t%d",lv);
        printf("\t%s",type);
        if(scope)
            printf("\t%s","Local");
        else
            printf("\t%s","Global");

        if(master)
            printf("\t%s",master);
        else
            printf("\t%s","\\");
        putchar('\n');

    }
#endif // DISPSYMBOL
}
#define DISPINTERMIDIATE

void displayIR(struct codenode* code)
{
#ifdef DISPINTERMIDIATE
    while(code!=NULL)
    {
        switch(code->op)
        {
            case FUNC_IR://函数定义,result.offset存储了函数栈帧大小
                printf("Function %s\n",code->result.id);
                break;
            case PARAM_IR://参数声明
                printf("Parameter %s\n",code->result.id);
                break;
            case ASSIGN_IR://赋值运算（区别于ASSIGN这个是temp=op1=op2，另一个是temp=op(var/const)）
                printf("%s = %s = %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case ARG_IR://传参
                printf("Arg %s\n",code->result.id);
                break;
            case CALL_IR://函数调用
                if(code->result.kind)
                    printf("%s = Call %s\n",code->result.id,code->opn2.id);
                else
                    printf("Call %s\n",code->opn2.id);
                break;
            case ARR_IR://数组调用
                printf("%s = %s[%s]\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case STRUCT_IR://结构调用
                printf("%s = %s.%s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case RETURN_IR://返回语句，一般带有传值给temp，result.offset存储了函数参数总字节数
                if(code->result.id)
                    printf("Return %s\n",code->result.id);
                else
                    printf("Return\n");
                break;
            /*
            case ASSIGN_ARR://数组写
                printf("%s[%s] = %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case ASSIGN_STRUCT://结构写
                printf("%s.%s = %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            */
            case IF_IR://格式if result==0 j op1 else j op2
                printf("IF %s == 0 Goto %s\n",code->result.id,code->opn1.id);
                break;
            case LABEL_IR://标号语句
                printf("Lable %s\n",code->result.id);
                break;
            case GOTO_IR://跳转语句
                printf("Goto %s\n",code->result.id);
                break;
            case EQ_IR://对应beq
                printf("If %s == %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case NE_IR://对应bne
                printf("If %s != %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case GT_IR://对应bgtz
                printf("If %s > %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case GE_IR://对应bgez
                printf("If %s >= %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case LT_IR://对应bltz
                printf("If %s < %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case LE_IR://对应blez
                printf("If %s <= %s Goto %s\n",code->opn1.id,code->opn2.id,code->result.id);
                break;
            case ASSIGN://前文已述，常量赋值
                if(code->opn2.kind==INT_CONST)
                    printf("%s = %d\n",code->result.id,code->opn2.const_int);
                else
                    printf("%s = %s\n",code->result.id,code->opn2.id);
                break;
            case PLUS://下述格式统一temp=op1+op2，temp为临时变量
                printf("%s = %s + %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case MINUS:
                printf("%s = %s - %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case MUL:
                printf("%s = %s * %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case DIV:
                printf("%s = %s / %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case MOD:
                printf("%s = %s % %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case LSHFIT:
                printf("%s = %s << %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case RSHFIT:
                printf("%s = %s >> %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case AND:
                printf("%s = %s && %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case OR:
                printf("%s = %s || %s\n",code->result.id,code->opn1.id,code->opn2.id);
                break;
            case END_IR:
                return ;
            default:
                break;
        }
        code=code->next;
    }
#endif // DISPINTERMIDIATE
}

//连接字符串
char *strcats(char *s1, char *s2)
{
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/*别名，标签，临时变量名*/
char *createAlias()
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcats("v", s);
}
char *createLabel() {
    static int no=1;
    char s[10];
    itoa(no++,s,10);
    return strcats("Lable",s);
}

//生成一个临时变量的名字
char *createTemp()
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcats("temp", s);
}

/*引入addr变量的目的是解决数组和结构体的赋值问题，同时也将变量归类，addr被视为是temp变量*/
char* createAddr()
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcats("addr", s);
}

/*临时变量0拥有统一的名字：temp0，对应于mips的0号寄存器，这里是一个方法*/



//处理外部变量定义列表
void ExtDefList(struct node *T)
{
    int start=0;
    while(T)
	{
		switch(T->ptr[0]->kind)
		{
			case VAR:
			{
				//decide type and varlist,including var and array
				VarDef(T->ptr[0]);
				break;
			}
			case FUNCDEF:
			{
				FuncDef(T->ptr[0]);
				root=merge(2,root,T->ptr[0]->code);
				if(start==0)
                {
                    start=1;
                    root->flag='H';
                }
				break;
			}
			case FUNCDEC:
			{
				FuncDec(T->ptr[0]);
				break;
			}
			case STRUCTDEC:
			{
				StructDec(T->ptr[0]);
				break;
			}
			case STRUCTDEF:
			{
				StructDef(T->ptr[0]);
				break;
			}
			default:
				break;
		}
		T=T->ptr[1];

	}
	dispsymbol();
	root=merge(2,root,genIR(END_IR,op1,op2,result));
	if(NoIRgenerate==0)
    {
        displayIR(root);
        putchar('\n');
        //DAGoptimize();
        //displayIR(target);
        generateTargetCode(root);
    }

    else
        printf("Semantic error occured, please check again\n");
}

/*变量和数组不能和除成员变量，参数外的任何外部定义重名*/
void VarDef(struct node *T)
{
	int type;
	struct node *temp;
	int basewidth;

	if(!strcmp(T->ptr[0]->type_id,"int"))
		type=0;
	else if(!strcmp(T->ptr[0]->type_id,"char"))
		type=1;
	else if(!strcmp(T->ptr[0]->type_id,"float"))
		type=2;

    if(type!=0)
    {
        errormsg(0,T->pos,T->ptr[0]->type_id,"Function type should be int, temporarily");
    }
    basewidth=4;//默认为int

    int dim=basewidth;

	while(T->ptr[1])
	{
		T=T->ptr[1];
		if(T->ptr[0]->kind==VARDEC)
		{
			temp=T->ptr[0];
			if(redec(temp->ptr[0]->type_id,'V',0))//除结构成员变量外不能有变量重名
			{
			    temp->ptr[0]->width=basewidth;
                temp->ptr[0]->offset=globaloff;
                globaloff+=basewidth;
				fillSymbolTable(temp->ptr[0]->type_id,createAlias(),LV,type,'V',0,temp->ptr[0]->offset);
				if(temp->ptr[1])
                {//由于内容的限制，对于变量的初始化，由于不直接生成目标代码，最终写入内存的内容无法直接构成。外部变量的初始化需要单独考虑
                    if(Exp(temp->ptr[1])!=0)//这里收紧了语言的规范，只要不是int型变量就算错
					{
						errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Variable initializer types do not match or initializer element is not constant");
					}
                }
			}
			else
			{
				errormsg(1,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of variables");
			}
		}
		else//ARRDEC
		{
			temp=T->ptr[0];
			if(redec(temp->ptr[0]->type_id,'A',0))//除结构成员变量外不能有变量重名
			{
			    struct node *temp1=temp->ptr[1];//维度
			    temp->ptr[0]->offset=globaloff;
				fillSymbolTable(temp->ptr[0]->type_id,createAlias(),LV,type,'A',0,temp->ptr[0]->offset);
				symbolTable.symbols[symbolTable.index].paramnum=0;
				while(temp1)
                {
                    if(Exp(temp1->ptr[0])!=0)//一个一个查数组维度，必须为常量
					{
						errormsg(2,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Array length is not Int_Const");
					}
					else
                    {
                        dim=dim*getresult(temp1->ptr[0]);
                        symbolTable.symbols[symbolTable.index-1].paramnum++;//维数记录增加
                    }
                    temp1=temp1->ptr[1];
                    if(temp1!=NULL)
                    {
                        errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Array dimension just support 1 dim, temporarily");
                    }
                }
				/*此处查数组初始化，个人认为必要性不大，太复杂了，这里留作后续处理吧*/
				/*二更，数组初始化取消*/
			}
			else
			{
				errormsg(1,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of arrays");
			}
			temp->ptr[0]->width=dim;
			globaloff+=dim;
		}
	}

}
/*函数原型不能和除成员变量和参数和函数声明和函数原型外的变量重名*/
void FuncDec(struct node *T)
{
	int type;
	struct node *temp;
	struct node *ttemp;

	int i,j;

	if(!strcmp(T->ptr[0]->type_id,"int"))
		type=0;
	else if(!strcmp(T->ptr[0]->type_id,"char"))
		type=1;
	else if(!strcmp(T->ptr[0]->type_id,"float"))
		type=2;
	else if(!strcmp(T->ptr[0]->type_id,"void"))
		type=3;

    if(type==1||type==2)
    {
        errormsg(0,T->pos,T->ptr[0]->type_id,"Function type should not be char or float, temporarily");
    }

	temp=T->ptr[1];//指针指向funcdef

	if(redec(temp->ptr[0]->type_id,'D',0))
	{
		symbolTable.symbols[symbolTable.index].paramnum=0;//此时对应于符号表的即将写入的位置，将其参数数量写为0
		i=symbolTable.index;
		fillSymbolTable(temp->ptr[0]->type_id,NULL,LV,type,'D',0,0);//填符号表，对函数原型，只在语义分析阶段有作用，因此无需别名
		ttemp=temp->ptr[1];//指向paralist
		int ttype;
		while(ttemp)//参数列表
		{
			if(redec(ttemp->ptr[0]->ptr[1]->type_id,'P',i))
			{
				if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"int"))
					ttype=0;
				else if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"char"))
					ttype=1;
				else if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"float"))
					ttype=2;

                if(ttype!=0)
                {
                    errormsg(0,ttemp->ptr[0]->ptr[1]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Function param type should be int, temporarily");
                }

				if(ttemp->ptr[0]->ptr[1]->kind==VARNAME)
				{
					fillSymbolTable(ttemp->ptr[0]->ptr[1]->type_id,NULL,LV+1,ttype,'P',0,0);//函数原型可以不用参与中间代码生成
					symbolTable.symbols[symbolTable.index-1].paramnum=i;
					symbolTable.symbols[i].paramnum++;//统计参数数量
				}
				else
				{
					//此处为开发时的BUG，默认定义其实是函数参数列表并不支持数组
					errormsg(3,ttemp->ptr[1]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Function Paramater do not support array, temporarily");
					/*
					symbolTable.symbols[symboltable.index].paramnum=1;//函数参数只支持一维数组
					fillSymbolTable(ttemp->ptr[1]->type_id,LV,ttype,'P');

					if(Exp(ttemp->ptr[2]))//如果表达式没有问题
					{
						//没问题
					}
					else
					{
						//函数参数列表表达式有问题
					}




					*/
				}
			}
			else
			{
				errormsg(4,temp->ptr[0]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Redeclaration of function paramaters");
			}
			ttemp=ttemp->ptr[1];
		}
		j=searchTarget(temp->ptr[0]->type_id,'F');
		if(j!=-1)
        {
            switch(parammatch(i,j))
            {
                case 0:
                    break;
                case 1:
                    errormsg(5,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's type are not match");
                    break;
                case 2:
                    errormsg(6,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's paramater num are not match");
                    break;
                case 3:
                    errormsg(7,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's paramater type are not match");
                    break;
                default:
                    break;
            }
        }
	}
	else
	{
		errormsg(8,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of functions");
	}

}

void FuncDef(struct node *T)
{
	int type;
	struct node *temp;
	struct node *ttemp;
	int i,j;
    struct codenode* tempnode=NULL;
    nowfuncparanum=0;

	if(!strcmp(T->ptr[0]->type_id,"int"))
		type=0;
	else if(!strcmp(T->ptr[0]->type_id,"char"))
		type=1;
	else if(!strcmp(T->ptr[0]->type_id,"float"))
		type=2;
	else if(!strcmp(T->ptr[0]->type_id,"void"))
		type=3;

    if(type!=0)
    {
        errormsg(0,T->pos,temp->ptr[0]->type_id,"Function type should be int, temporarily");
    }

	functype=type;

	temp=T->ptr[1];//指针指向funchead

	if(redec(temp->ptr[0]->type_id,'F',0))
	{
		symbolTable.symbols[symbolTable.index].paramnum=0;//此时对应于符号表的即将写入的位置，将其参数数量写为0
		i=symbolTable.index;
		fillSymbolTable(temp->ptr[0]->type_id,NULL,LV,type,'D',0,0);//填符号表
		ttemp=temp->ptr[1];//指向paralist
		int ttype;

		int baseoffset=8;//这里的初始值就是函数参数相对于函数新栈帧的偏移量（按理而言应该是正数，理解为负数也可）
		//另外，上面还有返回地址
		//因为栈最后一个存储的是返回地址

		while(ttemp)//参数列表
		{
			if(redec(ttemp->ptr[0]->ptr[1]->type_id,'P',i))
			{
				if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"int"))
					ttype=0;
				else if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"char"))
					ttype=1;
				else if(!strcmp(ttemp->ptr[0]->ptr[0]->type_id,"float"))
					ttype=2;

                if(ttype!=0)
                {
                    errormsg(0,ttemp->ptr[0]->ptr[1]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Function param type should be int, temporarily");
                }

				if(ttemp->ptr[0]->ptr[1]->kind==VARNAME)
				{
				    ttemp->ptr[0]->ptr[1]->offset=baseoffset;
				    //对函数参数的处理是一个序列，函数参数的偏移量是相对于函数头的即新函数的栈帧的偏移量
				    //需要加上返回地址的偏移地址，为方便mips32的使用，int型数据的恒定值就是4
				    fillSymbolTable(ttemp->ptr[0]->ptr[1]->type_id,createAlias(),LV+1,ttype,'P',1,ttemp->ptr[0]->ptr[1]->offset);
				    //此处将函数形参当做局部变量的作用域为1
					symbolTable.symbols[symbolTable.index-1].paramnum=i;

					symbolTable.symbols[i].paramnum++;//统计参数数量
					nowfuncparanum++;//统计参数个数后续生成中间代码用
					ttemp->ptr[0]->ptr[1]->width=4;//参数长度恒定为sizeof(int);
				    result.kind=ID_IR;
                    strcpy(result.id, symbolTable.symbols[symbolTable.index-1].alias);
                    symbolTable.symbols[symbolTable.index-1].offset=-baseoffset;
                    result.offset=-baseoffset;
                    result.level=LV+1;//参数视为局部变量
				    ttemp->ptr[0]->ptr[1]->code=genIR(PARAM_IR,op1,op2,result);
				    tempnode=merge(2,tempnode,ttemp->ptr[0]->ptr[1]->code);
				    baseoffset+=4;
				}
				else
				{
					//此处为开发时的BUG，默认定义其实是函数参数列表并不支持数组
					errormsg(3,ttemp->ptr[1]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Function Paramater do not support array, temporarily");
					/*
					fillSymbolTable(ttemp->ptr[1]->type_id,LV,ttype,'P');
					symbolTable.symbols[symboltable.index-1].paramnum=1;//函数参数只支持一维数组
					if(Exp(ttemp->ptr[2]))//如果表达式没有问题
					{
						//没问题
					}
					else
					{
						//函数参数列表表达式有问题
					}
					*/
				}
			}
			else
			{
				errormsg(4,temp->ptr[0]->pos,ttemp->ptr[0]->ptr[1]->type_id,"Redeclaration of function paramaters");
			}
			ttemp=ttemp->ptr[1];
		}

		j=searchTarget(temp->ptr[0]->type_id,'D');
		if(j!=-1)
        {
            switch(parammatch(i,j))
            {
                case 0:
                    break;
                case 1:
                    errormsg(5,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's type are not match");
                    break;
                case 2:
                    errormsg(6,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's paramater num are not match");
                    break;
                case 3:
                    errormsg(7,T->pos,temp->ptr[0]->type_id,"Funcdec and funcdef's paramater type are not match");
                    break;
                default:
                    break;
            }
        }

	}
	else
	{
		errormsg(8,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of functions");
	}
	localoff=4;
	funcwidth=0;
	ifreturn=0;
	nowfunc=i;
	LV++;
	CompoundStat(T->ptr[2]);//检查复合语句
	LV--;
	if(ifreturn!=1)
    {
        errormsg(9,temp->ptr[0]->pos,temp->ptr[0]->type_id,"No return statment of non-void function");
    }
	nowfunc=-1;
	T->width=funcwidth;
	strcpy(result.id,temp->ptr[0]->type_id);//函数名
	result.offset=funcwidth;
	T->code=genIR(FUNC_IR,op1,op2,result);//FUNCTION HEAD
	T->code=merge(2,T->code,tempnode);//PARAM
	T->code=merge(2,T->code,T->ptr[2]->code);//Compound Stat
}

void StructDec(struct node *T)
{
	struct node *temp;
	int i;
	int master;//结构体对应于符号表的节点，对于后续判断成员变量的重复


	if(redec(T->type_id,'S',0))
	{
		master=fillSymbolTable(T->type_id,NULL,LV,0,'S',0,0);//填符号表
		//处理成员变量列表
		temp=T->ptr[0];//指向成员列表
		int type;
		while(temp)
		{
			if(!strcmp(temp->ptr[0]->type_id,"int"))
				type=0;
			else if(!strcmp(temp->ptr[0]->type_id,"char"))
				type=1;
			else if(!strcmp(temp->ptr[0]->type_id,"float"))
				type=2;

            if(type!=0)
            {
                errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Struct member type should be int, temporarily");
            }

			struct node *ttemp;
			int number;

			int baseoffset=0;

			ttemp=temp->ptr[1];
			while(ttemp)
			{
				if(redec(ttemp->type_id,'N',master))
				{
				    ttemp->offset=baseoffset;//偏移量
				    ttemp->width=4;//int类型长度
				    symbolTable.symbols[master].offset+=ttemp->width;//统计结构体长度
				    fillSymbolTable(ttemp->type_id,NULL,LV,type,'N',0,baseoffset);
				    symbolTable.symbols[symbolTable.index-1].paramnum=master;
				    baseoffset+=4;
				}
				else
				{
					//出现错误，成员变量重名
					errormsg(10,ttemp->pos,temp->ptr[0]->type_id,"Redeclaration of member variables");
				}
				ttemp=ttemp->ptr[0];
			}
			temp=temp->ptr[2];
		}

	}
	else
	{
		errormsg(11,T->pos,T->type_id,"Redeclaration of struct");
	}

}

void StructDef(struct node *T)
{
	struct node *temp;
	int i;
	int master;//结构体声明对应于符号表的节点，指定结构变量对应的结构体

	master=searchTarget(T->type_id,'S');//找结构体名，只能是结构声明
	if(master>=0)
	{
		temp=T->ptr[0];
		while(temp)
		{
			if(redec(temp->type_id,'B',0))
			{
			    temp->offset=globaloff;
				fillSymbolTable(temp->type_id,createAlias(),LV,-1,'B',0,temp->offset);
				//刚刚写入的符号，对应的结构体声明符号表的位置
				symbolTable.symbols[symbolTable.index-1].paramnum=master;
				globaloff+=4;
			}
			else
			{
				errormsg(12,temp->pos,temp->type_id,"Redefinition of struct variables");
			}
			temp=temp->ptr[0];
		}
	}
	else
	{
		errormsg(13,T->pos,T->type_id,"No such declaration of structs");
	}
}


/*进入复合语句记录进入时符号表的记录，出来的时候全部删掉*/
void CompoundStat(struct node *T)
{
	int record=symbolTable.index;//记录此时符号表表尾的位置
	struct node *temp=T->ptr[0];//指向statlist第一个
	while(temp)
	{
		switch(temp->ptr[0]->kind)
		{
			case LOCALVAR:
			case LOCALSTRUCTDEF:
				LocalVar(temp->ptr[0]);
				T->code=merge(2,T->code,temp->ptr[0]->code);
				break;
			default:
			{
				Stat(temp->ptr[0]);
				T->code=merge(2,T->code,temp->ptr[0]->code);//将语句对应的中间代码连接到复合语句上
				break;
			}
		}
		temp=temp->ptr[1];
	}
	dispsymbol();
	//退出复合语句时符号表要相应的去除
	symbolTable.index=record;

}

void Stat(struct node *T)
{
	switch(T->kind)
	{
	    /*表达式语句的中间代码序列放在表达式语法树的根上*/
		case EXP:
		    {
                if(Exp(T->ptr[0])==-1)//EXP按照其他表达式的规范，应该是从对应的运算符节点开始分析，这里是实验一上面的一个失误问题
                    errormsg(14,T->pos,"Expression statement","Semantic error");
                T->code=merge(2,T->code,T->ptr[0]->code);
                //EXP还是要注意修改一下，关键是返回值的规范。
                //EXP的中间代码，在EXP的树顶操作符上

                break;
		    }
        /*复合语句的中间代码序列放在复合语句的根上*/
		case COMPSTAT://复合语句中间代码
		    {
                LV++;
                CompoundStat(T);
                LV--;
                break;
		    }
        /*return语句的中间代码序列放在RETURN节点上*/
		case RETURN://在此判断函数匹配问题，
		{

			if(T->ptr[0])
			{
			    ifreturn=1;
				if(Exp(T->ptr[0])%3!=functype)
				{
					errormsg(15,T->pos,"return","values do not match function type");
				}
				result.kind=ID_IR;
                strcpy(result.id,symbolTable.symbols[expresultindex].alias);
                T->code=merge(2,T->code,T->ptr[0]->code);
			}
			else
			{
				if(functype!=-2)//若当前函数返回值类型不为void
				{
					errormsg(15,T->pos,"return","values do not match function type");
				}
				result.kind=0;
			}


            result.offset=nowfuncparanum*4;
            /*实验四限制，所有函数返回值均为Int，因此，这方面可以如此简化，直接将偏移量写入result节点*/

            /*此处需要修改，即expresultindex标识的就是表达式的值对应的临时变量，因此对于这里没必要弄一个赋值语句*/
            /*格式return result或return (result.kind==0)*/
            T->code=merge(2,T->code,genIR(RETURN_IR,op1,op2,result));

			break;
		}

		/*if语句的中间代码序列放在IF上*/
		/*对于表达式，申请一个新的变量，判断是否等于0*/
		/*if语句结构：exp中间代码序列，truelable，true语句（复合语句）序列，falsetable，（可选）false语句（复合语句序列），后续语句*/
		case IF:
		    {
                if(Exp(T->ptr[0])<0)
                    errormsg(14,T->pos,"if","Condition expression semantic error");
                int result1=expresultindex;
                Stat(T->ptr[1]);
                if(T->ptr[2])
                    Stat(T->ptr[2]);

                /*if语句的处理，需要针对于正确和错误的标号，*/
                strcpy(T->Etrue,createLabel());  //条件为真的跳转位置（其实没有啥用），这相当于一个存储标号的临时变量
                strcpy(T->Efalse,createLabel());//条件为假的跳转位置

                T->code=T->ptr[0]->code;//这是表达式的代码

                /*if用于比较表达式的值是否为0，是0跳转到false，是1跳转到true*/
                /*获得表达式的值在前面有过*/

                result.kind=ID_IR;
                strcpy(result.id,symbolTable.symbols[result1].alias);

                op1.kind=ID_IR;
                strcpy(op1.id,T->Efalse);
                struct codenode* tempnode=genIR(IF_IR,op1,op2,result);
                T->code=merge(2,T->code,tempnode);//if result==0 goto op1
                //IF条件判断的中间代码，视为result对应的变量是否等于0然后跳转到op1或者op2
                //这样的处理方法与指导书完全不同，原因有二，其一是对于C语言子集定义的不同，其二是对于诸如if(a+2)这样的表达式，调用布尔表达式处理完全错误
                /*加true标签，true语句序列，false标签，false语句序列，结尾标签*/

                T->code=merge(2,T->code,T->ptr[1]->code);//true语句序列
                tempnode->next->flag='H';//条件转移语句后面的一条语句
                char endlable[10];

                strcpy(endlable,createLabel());

                T->code=merge(2,T->code,genGoto(endlable));

                T->code=merge(2,T->code,genLabel(T->Efalse));

                if(T->ptr[2])
                {
                    //false语句序列
                    T->code=merge(2,T->code,T->ptr[2]->code);
                }
                T->code=merge(2,T->code,genLabel(endlable));

                break;

		    }
        /*while语句的中间代码序列放在WHILE上*/
        /*对于表达式，申请一个新的变量，判断是否等于0*/
		/*while语句结构：开始label，exp中间代码序列，truelable，true语句（复合语句）序列，falsetable，后续语句*/
		case WHILE:
		    {
                ifincir++;
                if(Exp(T->ptr[0])<0)
                    errormsg(14,T->pos,"while","Condition expression semantic error");
                int result1=expresultindex;
                Stat(T->ptr[1]);
                ifincir--;

                strcpy(T->Etrue,createLabel());
                strcpy(T->Efalse,createLabel());

                char startlable[10];
                strcpy(startlable,createLabel());


                T->code=genLabel(startlable);//while的开始标签

                T->code=merge(2,T->code,T->ptr[0]->code);

                result.kind=ID_IR;
                strcpy(result.id,symbolTable.symbols[result1].alias);//比较值

                op1.kind=ID_IR;
                strcpy(op1.id,T->Efalse);//假，即结束
                struct codenode* tempnode=genIR(IF_IR,op1,op2,result);
                T->code=merge(2,T->code,tempnode);//条件判断

                T->code=merge(2,T->code,T->ptr[1]->code);//语句序列
                tempnode->next->flag='H';
                T->code=merge(2,T->code,genGoto(startlable));//跳转到条件判断

                T->code=merge(2,T->code,genLabel(T->Efalse));//结束位置

                break;
		    }
        /*break语句的中间代码序列放在BREAK上*/
		case BREAK:
		    {
                if(ifincir==0)
                    errormsg(15,T->pos,"break","Not in circle");
                struct node* temp=T;
                //break语句的处理，将上溯寻找while语句或for语句的Efalse的label，写入goto即可。
                while(temp->parent->kind!=WHILE&&temp->parent->kind!=FOR)
                {
                    temp=temp->parent;
                }

                //temp指向的是break最近的循环节点
                //插入跳转到结束的goto语句即可

                T->code=genGoto(temp->Efalse);


                break;
		    }
        /*continue语句的中间代码序列放在CONTINUE上*/
		case CONTINUE:
		    {
                if(ifincir==0)
                    errormsg(16,T->pos,"continue","Not in circle");

                struct node* temp=T;
                //continue语句的处理，将上溯寻找while语句或for语句的开头的label，写入goto即可。
                while(temp->parent->kind!=WHILE&&temp->parent->kind!=FOR)
                {
                    temp=temp->parent;
                }

                //temp指向的是for最近的循环节点
                //插入跳转到开始的goto语句即可
                char startlable[10];
                //循环语句的节点对应的第一个中间代码一定是标号
                strcpy(startlable,temp->code->result.id);
                T->code=genGoto(startlable);
                break;
		    }
        /*for语句的中间代码序列放在FOR上*/
		case FOR:
		{
			ifincir++;
			int exporvar=0;
			int record=symbolTable.index;
			if(T->ptr[0]->ptr[0]->kind==DATATYPE)
			{
				//for内局部变量
				struct node *Temp=T->ptr[0];
				int type;
				struct node *temp;

                int basewidth=0;

				if(!strcmp(Temp->ptr[0]->type_id,"int"))
					type=0;
				else if(!strcmp(Temp->ptr[0]->type_id,"char"))
					type=1;
				else if(!strcmp(Temp->ptr[0]->type_id,"float"))
					type=2;

                basewidth=4;

                if(type!=0)
                {
                    errormsg(0,T->pos,temp->ptr[0]->type_id,"Struct member type should be int, temporarily");
                }

				while(Temp->ptr[1])
				{
					Temp=Temp->ptr[1];
					if(Temp->ptr[0]->kind==VARDEC)
					{
						int i;
						temp=Temp->ptr[0];

						if(localRedec(temp->ptr[0]->type_id,LV+1))//除结构成员变量外不能有变量重名
						{
							//for内局部变量，应该是层次+1，这样亦可兼容对应于复合语句的情况
							temp->ptr[0]->width=basewidth;
                            funcwidth+=basewidth;
                            temp->ptr[0]->offset=localoff;
                            localoff+=basewidth;
							i=fillSymbolTable(temp->ptr[0]->type_id,createAlias(),LV+1,type,'V',1,temp->ptr[0]->offset);
							if(temp->ptr[1])
							{
								if(Exp(temp->ptr[1])%3!=type)
									errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Variable initialization types do not match");
                                T->code=merge(2,T->code,temp->ptr[1]->code);
                                int result1=expresultindex;
                                op2.kind=ID_IR;
                                strcpy(op2.id,symbolTable.symbols[result1].alias);

                                result.kind=ID_IR;
                                strcpy(result.id,symbolTable.symbols[i].alias);
                                result.offset=temp->ptr[0]->offset;
                                result.level=LV;
                                //局部变量初始化的赋值，result(var)=op2(temp)，应对应于一条store指令
                                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));
                                //for1为局部变量
							}
						}
						else
						{
							errormsg(1,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of variables");
						}
					}
					else//ARRDEC
					{
						errormsg(17,T->pos,"for","Not support array definition");
						/*
						int i;
						temp=Temp->ptr[0];
						if(redec(temp->ptr[0]->type_id,'A',0))//除结构成员变量外不能有变量重名
						{
							struct node *temp1=temp->ptr[1];//维度
							fillSymbolTable(temp->ptr[0]->type_id,LV,type,'A',1);
							symbolTable.symbols[symboltable.index].paramnum=0;
							while(temp1)
							{
								if(Exp(temp1->ptr[0])!=1)//一个一个查数组维度，必须为常量
								{
									errormsg(temp->ptr[0]->pos,temp->ptr[0]->type_id,"数组长度不为整常量");
								}
								temp1=temp1->ptr[1];
								symbolTable.symbols[symboltable.index].paramnum++;//维数记录增加
							}
						}
						else
						{
							errormsg(temp->ptr[0]->pos,temp->ptr[0]->type_id,"数组变量名重复");
						}*/
					}
				}
			}
			else
			{
				if(Exp(T->ptr[0]->ptr[0])<0)
					errormsg(14,T->pos,"for","Condition Expression 1 semantic error");
                T->code=T->ptr[0]->code;//for1为表达式
			}

			if(Exp(T->ptr[1]->ptr[0])<0)
				errormsg(14,T->pos,"for","Condition Expression 2 semantic error");
            int result1=expresultindex;
			if(Exp(T->ptr[2]->ptr[0])<0)
				errormsg(14,T->pos,"for","Condition Expression 3 semantic error");
			Stat(T->ptr[3]);

			strcpy(T->Etrue,createLabel());
            strcpy(T->Efalse,createLabel());

            char startlable[10];
            strcpy(startlable,createLabel());




            T->code=merge(2,T->code,genLabel(startlable));//for中条件判断的开始标签

            T->code=merge(2,T->code,T->ptr[1]->ptr[0]->code);

            result.kind=ID_IR;
            strcpy(result.id,symbolTable.symbols[result1].alias);//比较值

            op1.kind=ID_IR;
            strcpy(op1.id,T->Efalse);//假，即结束
            struct codenode* tempnode=genIR(IF_IR,op1,op2,result);
            T->code=merge(2,T->code,tempnode);//条件判断

            T->code=merge(2,T->code,genLabel(T->Etrue));//正确标签
            tempnode->next->flag='H';
            T->code=merge(2,T->code,T->ptr[3]->code);//语句序列

            T->code=merge(2,T->code,T->ptr[2]->ptr[0]->code);//for3

            T->code=merge(2,T->code,genGoto(startlable));

            T->code=merge(2,T->code,genLabel(T->Efalse));//结束位置

			//不管有无，释放局部变量
			symbolTable.index=record;
			ifincir--;
			break;
		}
		default:
			break;
	}
}


/*函数局部变量的长度总和应该写入对应函数的偏移值的位置，方便调用时使用*/
/*函数局部变量长度的计算，应该写入总和*/
void LocalVar(struct node *T)
{
    struct node* Troot=T;//此处为一个失误，把函数参数值改变了
	if(T->kind==LOCALVAR)
	{
		int type;
		struct node *temp;
		int basewidth=0;
		int dim=1;

		if(!strcmp(T->ptr[0]->type_id,"int"))
			type=0;
		else if(!strcmp(T->ptr[0]->type_id,"char"))
			type=1;
		else if(!strcmp(T->ptr[0]->type_id,"float"))
			type=2;

        basewidth=4;

        if(type!=0)
        {
            errormsg(0,T->pos,temp->ptr[0]->type_id,"Local variable type should be int, temporarily");
        }

        int i=0;

		while(T->ptr[1])
		{
			T=T->ptr[1];
			if(T->ptr[0]->kind==VARDEC)
			{
				temp=T->ptr[0];
				if(localRedec(temp->ptr[0]->type_id,LV))//除结构成员变量外不能有变量重名
				{
				    temp->ptr[0]->width=basewidth;
				    funcwidth+=basewidth;
                    temp->ptr[0]->offset=localoff;
                    localoff+=basewidth;
					i=fillSymbolTable(temp->ptr[0]->type_id,createAlias(),LV,type,'V',1,temp->ptr[0]->offset);
					if(temp->ptr[1])
					{
						if(Exp(temp->ptr[1])%3!=type)
							errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Variable initialization types do not match");

                        Troot->code=merge(2,Troot->code,temp->ptr[1]->code);
                        int result1=expresultindex;
                        op2.kind=ID_IR;
                        strcpy(op2.id,symbolTable.symbols[result1].alias);

                        result.kind=ID_IR;
                        strcpy(result.id,symbolTable.symbols[i].alias);
                        result.offset=temp->ptr[0]->offset;
                        result.level=LV;
                        //局部变量初始化的赋值，result(var)=op2(temp)，应对应于一条store指令
                        Troot->code=merge(2,Troot->code,genIR(ASSIGN,op1,op2,result));
					}
				}
				else
				{
					errormsg(1,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of variables");
				}
			}
			else//ARRDEC
			{
				temp=T->ptr[0];
				if(localRedec(temp->ptr[0]->type_id,LV))//除结构成员变量外不能有变量重名
				{
				    int index=0;
					struct node *temp1=temp->ptr[1];//维度
					temp->ptr[0]->offset=localoff;
					index=fillSymbolTable(temp->ptr[0]->type_id,createAlias(),LV,type,'A',1,temp->ptr[0]->offset);
					symbolTable.symbols[index].paramnum=0;
					while(temp1)
					{
						if(Exp(temp1->ptr[0])!=0)//一个一个查数组维度，必须为常量
						{
							errormsg(2,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Array length is not Int_Const");
						}
						else
                        {
                            dim=dim*getresult(temp1->ptr[0]);
                            symbolTable.symbols[index].paramnum++;//维数记录增加
                        }
						temp1=temp1->ptr[1];
                        if(temp1!=NULL)
                        {
                            errormsg(0,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Array dimension just support 1 dim, temporarily");
                        }
					}
					/*此处查表达式初始化*/
				}
				else
				{
					errormsg(1,temp->ptr[0]->pos,temp->ptr[0]->type_id,"Redefinition of arrays");
				}
                temp->ptr[0]->width=dim;
                funcwidth+=dim;
                localoff+=dim;
			}
		}
	}
	else
	{
		struct node *temp;
		int master;//结构体声明对应于符号表的节点，指定结构变量对应的结构体

		master=searchTarget(T->type_id,'S');//找结构体名，只能是结构声明
		if(master)
		{
			temp=T->ptr[0];
			while(temp)
			{
				if(localRedec(temp->type_id,LV))
				{
				    temp->offset=localoff;
				    temp->width=symbolTable.symbols[master].offset;
				    funcwidth+=temp->width;
					fillSymbolTable(temp->type_id,createAlias(),LV,-1,'B',1,temp->offset);
					//刚刚写入的符号，对应的结构体声明符号表的位置
					symbolTable.symbols[symbolTable.index-1].paramnum=master;
					localoff+=temp->width;//偏移量增加
				}
				else
				{
					errormsg(12,temp->pos,temp->type_id,"Redefinition of struct variables");
				}
				temp=temp->ptr[0];
			}
		}
		else
		{
			errormsg(13,T->pos,T->type_id,"No such declaration of structs");
		}
	}
}

/*返回计算结果， -2:void -1:error 0:int const 1:char const 2:float const 3:int var 4:char var 5:float var*/
/*表达式返回结果的判断，对于求值表达式，出现<0即表示出错了，因为void类型不能作为值，除了Stat内Exp只有在-1时出错*/
/*表达式分三类，第一类是变量调用及计算，这类表达式的结果需写入某寄存器（即某临时变量）内， 如语句3+4即便没有赋值也要写入寄存器*/
/*第二类是常量调用，将常量写入寄存器内，常量的写入可以用addi*/
/*第三类是函数调用，返回结果写入对应的传值寄存器，这也是写入寄存器，但是不是写入通用寄存器*/
int Exp(struct node* T)
{
	switch(T->kind)
	{
		//左值的意义基本上是单走一个varcall和arraycall，为方便后续实验还是将这些分开写为好
		case ASSIGN:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL&&T->ptr[0]->kind!=STRUCTCALL)
			{
				errormsg(18,T->pos,"Assign","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Assign","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Assign","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign","lvalue and rvalue do not match");
				return -1;
			}

			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//存储变量1
			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_PLUS:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_plus","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Assign_plus","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Assign_plus","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_plus","lvalue and rvalue do not match");
				return -1;
			}

			/*+=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(PLUS,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_MINUS:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_minus","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Assign_minus","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Assign_minus","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_minus","lvalue and rvalue do not match");
				return -1;
			}

			/*-=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(MINUS,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_MUL:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_mul","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Assign_mul","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Assign_mul","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_mul","lvalue and rvalue do not match");
				return -1;
			}

			/* *=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(MUL,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_DIV:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_div","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Assign_div","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Assign_div","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_div","lvalue and rvalue do not match");
				return -1;
			}

			/* /=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(DIV,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_MOD:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_mod","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;

			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Assign_mod","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Assign_mod","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_mod","lvalue and rvalue do not match");
				return -1;
			}

			/*%=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(MOD,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_LSHFIT:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_lshift","Inappropriate lvalue");
				return -1;
			}
            int lvalue=Exp(T->ptr[0]);
            int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Assign_lshift","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Assign_lshift","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_lshift","lvalue and rvalue do not match");
				return -1;
			}

			/*<<=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(LSHFIT,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case ASSIGN_RSHFIT:
		{
			if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
			{
				errormsg(18,T->pos,"Assign_rshift","Inappropriate lvalue");
				return -1;
			}
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;

			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Assign_rshift","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Assign_rshift","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Assign_rshift","lvalue and rvalue do not match");
				return -1;
			}

			/*>>=的中间代码，第一个临时变量存储加法结果，第二个临时变量存储运算结果*/

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op1.offset=symbolTable.symbols[result1].offset;
			op1.level=symbolTable.symbols[result1].level;
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(RSHFIT,op1,op2,result));

			//处理赋值过程
			op2.kind=ID_IR;
			strcpy(op2.id,result.id);

			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);
			T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
			/*这里的意思是，tempX=op1=op2*/
			/*tempX并无实际意义，但是还是有需要的，如if语句*/

			return lvalue;
			break;
		}
		case AND:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"And","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"And","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"And","lvalue and rvalue do not match");
				return -1;
			}
			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(AND,op1,op2,result));

			return lvalue;
			break;
		}
		case OR:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Or","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(21,T->pos,"Or","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Or","lvalue and rvalue do not match");
				return -1;
			}

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(OR,op1,op2,result));

			return lvalue;
			break;
		}
		case RELOP:
		{
		    //关系算符包括
		    //==,!=,>,>=,<,<=，结果只有两个，0和1
		    //但最终临时变量的值存放的是比较器的输出结果，
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Relop","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Relop","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Relop","lvalue and rvalue do not match");
				return -1;
			}

			int relationop=0;

			if(!strcmp(T->type_id,"=="))
            {
                relationop=EQ_IR;//if的基本盘就是判断是否为0跳转
            }
            else if(!strcmp(T->type_id,"!="))
            {
                relationop=NE_IR;
            }
            else if(!strcmp(T->type_id,">="))
            {
                relationop=GE_IR;
            }
            else if(!strcmp(T->type_id,">"))
            {
                relationop=GT_IR;
            }
            else if(!strcmp(T->type_id,"<="))
            {
                relationop=LE_IR;
            }
            else if(!strcmp(T->type_id,"<"))
            {
                relationop=LT_IR;
            }

            //代码结构：exp,exp,op(tempvar),#temp=0,goto labend,labtrue,#temp=1,labend

            //子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//创建标签：true,false,end

			strcpy(T->Etrue,createLabel());
			char tempend[15];
			strcpy(tempend,createLabel());





			result.kind=ID_IR;
			strcpy(result.id,T->Etrue);

            op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			//某操作决定是否goto result
			struct codenode* tempnode=genIR(relationop,op1,op2,result);
			T->code=merge(2,T->code,tempnode);

            //temp=0

			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);

            op2.kind=INT_CONST;
            op2.const_int=0;
            result.kind=ID_IR;
            strcpy(result.id,tempvar);
            T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));
            tempnode->next->flag='H';

            T->code=merge(2,T->code,genGoto(tempend));

            T->code=merge(2,T->code,genLabel(T->Etrue));

            //temp=1
            op2.kind=INT_CONST;
            op2.const_int=1;
            result.kind=ID_IR;
            strcpy(result.id,tempvar);
            T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

            T->code=merge(2,T->code,genLabel(tempend));

			return lvalue;
			break;
		}
		case PLUS:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Plus","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Plus","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Plus","lvalue and rvalue do not match");
				return -1;
			}

			//子树的代码
			T->code=merge(2,T->code,T->ptr[0]->code);
			T->code=merge(2,T->code,T->ptr[1]->code);

			//处理加法过程
			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,tempvar);

			op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op2.kind=ID_IR;
			strcpy(op2.id,symbolTable.symbols[result2].alias);

			T->code=merge(2,T->code,genIR(PLUS,op1,op2,result));

			return lvalue;
			break;
		}
		case MINUS:
		{
			if(T->ptr[1])
			{
				int lvalue=Exp(T->ptr[0]);
				int result1=expresultindex;
                int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
                int result2=expresultindex;
				if(lvalue<0)
				{
					errormsg(19,T->pos,"Minus","lvalue error");
					return -1;
				}
				if(rvalue<0)
				{
					errormsg(20,T->pos,"Minus","rvalue error");
					return -1;
				}
				if(lvalue%3!=rvalue%3)
				{
					errormsg(21,T->pos,"Minus","lvalue and rvalue do not match");
					return -1;
				}

				//子树的代码
                T->code=merge(2,T->code,T->ptr[0]->code);
                T->code=merge(2,T->code,T->ptr[1]->code);

                //处理加法过程
                char tempvar[10];
                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=ID_IR;
                strcpy(op1.id,symbolTable.symbols[result1].alias);
                op2.kind=ID_IR;
                strcpy(op2.id,symbolTable.symbols[result2].alias);

                T->code=merge(2,T->code,genIR(MINUS,op1,op2,result));

				return lvalue;
			}
			else
			{
				int rvalue=Exp(T->ptr[0]);
				int result1=expresultindex;
				if(rvalue<0)
				{
					errormsg(20,T->pos,"Neg","rvalue error");
					return -1;
				}

				//子树的代码
                T->code=merge(2,T->code,T->ptr[0]->code);

                //处理加法过程
                char tempvar[10];
                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=INT_CONST;
                op1.const_int=0;
                op2.kind=ID_IR;
                strcpy(op2.id,symbolTable.symbols[result1].alias);

                //此处特殊之处在于，mips没有neg指令，因此只能用0减
                T->code=merge(2,T->code,genIR(MINUS,op1,op2,result));

				return rvalue;
			}
			break;
		}
		case MUL:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Mul","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Mul","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Mul","lvalue and rvalue do not match");
				return -1;
			}

			//子树的代码
            T->code=merge(2,T->code,T->ptr[0]->code);
            T->code=merge(2,T->code,T->ptr[1]->code);

            //处理加法过程
            char tempvar[10];
            strcpy(tempvar,createTemp());
            expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
            result.kind=ID_IR;
            strcpy(result.id,tempvar);

            op1.kind=ID_IR;
            strcpy(op1.id,symbolTable.symbols[result1].alias);
            op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[result2].alias);

            T->code=merge(2,T->code,genIR(MUL,op1,op2,result));

			return lvalue;
			break;
		}
		case DIV:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0)
			{
				errormsg(19,T->pos,"Div","lvalue error");
				return -1;
			}
			if(rvalue<0)
			{
				errormsg(20,T->pos,"Div","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Div","lvalue and rvalue do not match");
				return -1;
			}

			//子树的代码
            T->code=merge(2,T->code,T->ptr[0]->code);
            T->code=merge(2,T->code,T->ptr[1]->code);

            //处理加法过程
            char tempvar[10];
            strcpy(tempvar,createTemp());
            expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
            result.kind=ID_IR;
            strcpy(result.id,tempvar);

            op1.kind=ID_IR;
            strcpy(op1.id,symbolTable.symbols[result1].alias);
            op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[result2].alias);

            T->code=merge(2,T->code,genIR(DIV,op1,op2,result));

			return lvalue;
			break;
		}
		case MOD:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Mod","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Mod","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Mod","lvalue and rvalue do not match");
				return -1;
			}


			//子树的代码
            T->code=merge(2,T->code,T->ptr[0]->code);
            T->code=merge(2,T->code,T->ptr[1]->code);

            //处理加法过程
            char tempvar[10];
            strcpy(tempvar,createTemp());
            expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
            result.kind=ID_IR;
            strcpy(result.id,tempvar);

            op1.kind=ID_IR;
            strcpy(op1.id,symbolTable.symbols[result1].alias);
            op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[result2].alias);

            T->code=merge(2,T->code,genIR(MOD,op1,op2,result));

			return lvalue;
			break;
		}
		case LSHFIT:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Lshift","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Lshift","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Lshift","lvalue and rvalue do not match");
				return -1;
			}


			//子树的代码
            T->code=merge(2,T->code,T->ptr[0]->code);
            T->code=merge(2,T->code,T->ptr[1]->code);

            //处理加法过程
            char tempvar[10];
            strcpy(tempvar,createTemp());
            expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
            result.kind=ID_IR;
            strcpy(result.id,tempvar);

            op1.kind=ID_IR;
            strcpy(op1.id,symbolTable.symbols[result1].alias);
            op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[result2].alias);

            T->code=merge(2,T->code,genIR(LSHFIT,op1,op2,result));

			return lvalue;
			break;
		}
		case RSHFIT:
		{
			int lvalue=Exp(T->ptr[0]);
			int result1=expresultindex;
			int rvalue=Exp(T->ptr[1]);//赋值运算最终值为
			int result2=expresultindex;
			if(lvalue<0||lvalue%3==2)
			{
				errormsg(19,T->pos,"Rshift","lvalue error");
				return -1;
			}
			if(rvalue<0||rvalue%3==2)
			{
				errormsg(20,T->pos,"Rshift","rvalue error");
				return -1;
			}
			if(lvalue%3!=rvalue%3)
			{
				errormsg(21,T->pos,"Rshift","lvalue and rvalue do not match");
				return -1;
			}


			//子树的代码
            T->code=merge(2,T->code,T->ptr[0]->code);
            T->code=merge(2,T->code,T->ptr[1]->code);

            //处理加法过程
            char tempvar[10];
            strcpy(tempvar,createTemp());
            expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
            result.kind=ID_IR;
            strcpy(result.id,tempvar);

            op1.kind=ID_IR;
            strcpy(op1.id,symbolTable.symbols[result1].alias);
            op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[result2].alias);

            T->code=merge(2,T->code,genIR(RSHFIT,op1,op2,result));

			return lvalue;
			break;
		}
		case INC:
		{
			int value;
			if(T->ptr[0])
			{
				if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
				{
					errormsg(22,T->pos,"Inc","Inappropriate opreation value");
					return -1;
				}
				value=Exp(T->ptr[0]);
				int result1=expresultindex;
				if(value<0)
				{
					errormsg(23,T->pos,"Inc","Opreation value error");
					return -1;
				}

                char tempvar[10];

				//将自增视为+=1的操作，由于mips不存在INC指令，所以这样的操作是完全等价的
				//子树的代码
                T->code=merge(2,T->code,T->ptr[0]->code);

                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);

                //后缀自增通过增加临时变量解决

                op2.kind=ID_IR;
                strcpy(op2.id,symbolTable.symbols[result1].alias);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

                //处理加法过程

                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=ID_IR;
                strcpy(op1.id,symbolTable.symbols[result1].alias);
                op1.offset=symbolTable.symbols[result1].offset;
                op1.level=symbolTable.symbols[result1].level;


                op2.kind=INT_CONST;
                op2.const_int=1;
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                //常量的使用要先创建临时变量
                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

                op2.kind=ID_IR;
                strcpy(op2.id,result.id);


                T->code=merge(2,T->code,genIR(PLUS,op1,op2,result));

                //处理赋值过程
                op2.kind=ID_IR;
                strcpy(op2.id,result.id);

                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);
                T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
                /*这里的意思是，tempX=op1=op2*/
                /*tempX并无实际意义，但是还是有需要的，如if语句*/

			}
			else if(T->ptr[1])
			{
				if(T->ptr[1]->kind!=VARCALL&&T->ptr[1]->kind!=ARRAYCALL)
				{
					errormsg(22,T->pos,"Inc","Inappropriate opreation value");
					return -1;
				}
				value=Exp(T->ptr[1]);
				int result1=expresultindex;
				if(value<0)
				{
					errormsg(23,T->pos,"Inc","Opreation value error");
					return -1;
				}
				//后缀自增的处理，传出当前临时变量对应的值即可，在缀上自增，唯二一个运算前传值的运算符
				//仍然需要临时变量存储
				//将自增视为+=1的操作，由于mips不存在INC指令，所以这样的操作是完全等价的
				//子树的代码
                T->code=merge(2,T->code,T->ptr[1]->code);


                //处理加法过程



                char tempvar[10];
                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=ID_IR;
                strcpy(op1.id,symbolTable.symbols[result1].alias);
                op1.offset=symbolTable.symbols[result1].offset;
                op1.level=symbolTable.symbols[result1].level;
                op2.kind=INT_CONST;
                op2.const_int=1;

                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                //常量的使用要先创建临时变量
                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

                op2.kind=ID_IR;
                strcpy(op2.id,result.id);


                T->code=merge(2,T->code,genIR(PLUS,op1,op2,result));

                //处理赋值过程
                op2.kind=ID_IR;
                strcpy(op2.id,result.id);

                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);
                T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
                /*这里的意思是，tempX=op1=op2*/
                /*tempX并无实际意义，但是还是有需要的，如if语句*/

			}
			return value;
			break;
		}
		case DEC:
		{
			int value;
			if(T->ptr[0])
			{
				if(T->ptr[0]->kind!=VARCALL&&T->ptr[0]->kind!=ARRAYCALL)
				{
					errormsg(22,T->pos,"Dec","Inappropriate opreation value");
					return -1;
				}
				value=Exp(T->ptr[0]);
				int result1=expresultindex;
				if(value<0)
				{
					errormsg(22,T->pos,"Dec","Opreation value error");
					return -1;
				}

				//将自减视为-=1的操作，由于mips不存在DEC指令，所以这样的操作是完全等价的
				//子树的代码
                T->code=merge(2,T->code,T->ptr[0]->code);


                char tempvar[10];

                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);

                //后缀自增通过增加临时变量解决

                op2.kind=ID_IR;
                strcpy(op2.id,symbolTable.symbols[result1].alias);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));


                //处理加法过程

                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=ID_IR;
                strcpy(op1.id,symbolTable.symbols[result1].alias);
                op1.offset=symbolTable.symbols[result1].offset;
                op1.level=symbolTable.symbols[result1].level;
                op2.kind=INT_CONST;
                op2.const_int=1;


                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                //常量的使用要先创建临时变量
                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

                op2.kind=ID_IR;
                strcpy(op2.id,result.id);


                T->code=merge(2,T->code,genIR(MINUS,op1,op2,result));

                //处理赋值过程
                op2.kind=ID_IR;
                strcpy(op2.id,result.id);

                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);
                T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
                /*这里的意思是，tempX=op1=op2*/
                /*tempX并无实际意义，但是还是有需要的，如if语句*/


			}
			else if(T->ptr[1])
			{
				if(T->ptr[1]->kind!=VARCALL&&T->ptr[1]->kind!=ARRAYCALL)
				{
					errormsg(23,T->pos,"Dec","Inappropriate opreation value");
					return -1;
				}
				value=Exp(T->ptr[1]);
				int result1=expresultindex;
				if(value<0)
				{
					errormsg(23,T->pos,"Dec","Opreation value error");
					return -1;
				}

				//后缀自减的处理，传出当前临时变量对应的值即可，在缀上自减-=操作，唯二一个运算前传值的运算符
				//仍然需要临时变量存储
				//将自增视为-=1的操作，由于mips不存在DEC指令，所以这样的操作是完全等价的
				//子树的代码
                T->code=merge(2,T->code,T->ptr[1]->code);


                //处理减法过程



                char tempvar[10];
                strcpy(tempvar,createTemp());
                fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                op1.kind=ID_IR;
                strcpy(op1.id,symbolTable.symbols[result1].alias);
                op1.offset=symbolTable.symbols[result1].offset;
                op1.level=symbolTable.symbols[result1].level;
                op2.kind=INT_CONST;
                op2.const_int=1;


                result.kind=ID_IR;
                strcpy(result.id,tempvar);

                //常量的使用要先创建临时变量
                T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

                op2.kind=ID_IR;
                strcpy(op2.id,result.id);


                T->code=merge(2,T->code,genIR(MINUS,op1,op2,result));

                //处理赋值过程
                op2.kind=ID_IR;
                strcpy(op2.id,result.id);

                strcpy(tempvar,createTemp());
                expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
                result.kind=ID_IR;
                strcpy(result.id,tempvar);
                T->code=merge(2,T->code,genIR(ASSIGN_IR,op1,op2,result));
                /*这里的意思是，tempX=op1=op2*/
                /*tempX并无实际意义，但是还是有需要的，如if语句*/


			}
			return value;
			break;
		}
		case NOT:
		{
			int value=Exp(T->ptr[0]);
			int result1=expresultindex;
			if(value<0)
			{
				errormsg(23,T->pos,"Not","Opreation value error");
				return -1;
			}
			//逻辑非的处理不同于按位非，是以0值为判断的
			//按照是否为0比较取值，逻辑非

			//语句序列 exp ，if ，temp=1，goto end，truelable,temp=0,endlable
            T->code=merge(2,T->code,T->ptr[0]->code);

			//创建标签：true,end

			strcpy(T->Etrue,createLabel());
			char tempend[15];
			strcpy(tempend,createLabel());

			char tempvar[10];
			strcpy(tempvar,createTemp());
			expresultindex=fillSymbolTable(NULL,tempvar,LV,0,'T',1,0);
			result.kind=ID_IR;
			strcpy(result.id,T->Etrue);

            op1.kind=ID_IR;
			strcpy(op1.id,symbolTable.symbols[result1].alias);
			op2.kind=INT_CONST;
			op2.const_int=0;

			//某操作决定是否goto result
			struct codenode* tempnode=genIR(EQ_IR,op1,op2,result);
			T->code=merge(2,T->code,tempnode);

            //temp=0
            strcpy(tempvar,createTemp());
            op2.kind=INT_CONST;
            op2.const_int=0;
            result.kind=ID_IR;
            strcpy(result.id,tempvar);
            T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));
            tempnode->next->flag='H';
            //ASSIGN表示常量赋值result=op2
            T->code=merge(2,T->code,genGoto(tempend));

            T->code=merge(2,T->code,genLabel(T->Etrue));

            //temp=1
            strcpy(tempvar,createTemp());
            op2.kind=INT_CONST;
            op2.const_int=1;
            result.kind=ID_IR;
            strcpy(result.id,tempvar);
            T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

            T->code=merge(2,T->code,genLabel(tempend));

			return value;
			break;
		}
		case FUNCCALL:
		{
			/*函数调用一律以函数定义为准，若无函数定义只有函数声明，则以函数声明为准*/

			int nodestack[20];
			int top=-1;

			int type;
			int i=searchTarget(T->type_id,'F');
			if(i<0)
			{
			    i=searchTarget(T->type_id,'D');
			    if(i<0)
                    errormsg(24,T->pos,T->type_id,"Undeclared function or Undefined function");
			}
			type=symbolTable.symbols[i].type;

			/*以符号表中的参数为准，依次向下寻找*/
			int j=1;//从当前函数头开始向下匹配参数

			int pnum=symbolTable.symbols[i].paramnum;

			int deftype;
			int paratype;

			struct node *temp=T->ptr[0];

			while(j<=pnum)
			{
				deftype=symbolTable.symbols[i+j].type;
				paratype=Exp(temp->ptr[0]);
				//函数调用栈，每次调用表达式处理都会有一个expresult的index然后赋值给调用栈即可
				T->code=merge(2,T->code,temp->ptr[0]->code);
                nodestack[++top]=expresultindex;

				if(paratype<0)
				{
					errormsg(26,T->pos,T->type_id,"Real paramaters type error");
					return -1;
				}

				if(deftype%3!=paratype%3)
				{
					errormsg(27,T->pos,T->type_id,"Real paramaters do not match formal paramaters");
					return -1;
				}
				temp=temp->ptr[1];
				j++;
				if(temp==NULL&&j<=pnum)
				{
					//实参序列小于形参序列
					errormsg(28,T->pos,"Function_call","Real paramaters and formal paramaters are not equal");
					return -1;
				}
			}
			if(temp!=NULL)
			{
				//实参序列大于形参序列
				errormsg(28,T->pos,"Function_call","Real paramaters and formal paramaters are not equal");
				return -1;
			}
			/*函数调用的结果是返回值参*/


			while(top>=0)
            {
                result.kind=ID_IR;
                strcpy(result.id,symbolTable.symbols[nodestack[top--]].alias);
                T->code=merge(2,T->code,genIR(ARG_IR,op1,op2,result));
            }

            result.kind=0;
            if(type!=-2)//非void函数
            {
                result.kind=ID_IR;
                strcpy(result.id,createTemp());
                expresultindex=fillSymbolTable(NULL,result.id,LV,0,'T',1,0);//函数返回值
            }
			op2.kind=ID_IR;
            strcpy(op2.id,T->type_id);
            T->code=merge(2,T->code,genIR(CALL_IR,op1,op2,result));
			//格式：result(temp)=call op2，若result不存在（kind==0）则无返回值

			return type+3;
			break;
		}
		case ARRAYCALL:
		{
			int type;
			int i=searchTarget(T->type_id,'A');//找一个数组
			if(i<0)
			{
				errormsg(29,T->pos,T->type_id,"Undefined array");
				return -1;
			}
			type=symbolTable.symbols[i].type;

            int result1;//默认此时只是一维数组

			struct node *temp=T->ptr[0];
			int dnum=symbolTable.symbols[i].paramnum;

			while(dnum>0)
			{
				if(Exp(temp->ptr[0])%3!=0)
				{
					errormsg(30,T->pos,T->type_id,"Array dimention is not Int");
					return -1;
				}
				T->code=merge(2,T->code,temp->ptr[0]->code);
				result1=expresultindex;

				dnum--;
				temp=temp->ptr[1];

				if(dnum>0&&temp==NULL)
				{
					errormsg(31,T->pos,T->type_id,"Array dimention is greater than call dimention");
					return -1;
				}
			}
			if(temp!=NULL)
			{
				errormsg(32,T->pos,T->type_id,"Call dimention is greater than array dimention");
				return -1;
			}



			op2.kind=ID_IR;//OP2指示下标
            strcpy(op2.id,symbolTable.symbols[result1].alias);


            op1.kind=ID_IR;//op1指示
            strcpy(op1.id,symbolTable.symbols[i].alias);
            op1.offset=symbolTable.symbols[i].offset;
            op1.level=symbolTable.symbols[i].level;

            result.kind=ID_IR;
            strcpy(result.id,createAddr());
            expresultindex=fillSymbolTable(NULL,result.id,LV,0,'T',1,0);
            result.offset=op1.offset;//数组基址
            result.level=op1.level;

            T->code=merge(2,T->code,genIR(ARR_IR,op1,op2,result));
			//格式：result(temp)=op1[op2]，即op1+op2*4，若result不存在（kind==0）则无返回值


			return type+3;
			break;
		}
		case VARCALL:
		{
			int type;
			int i=searchTarget(T->type_id,'V');

			int j=-1;
			if(nowfunc>=0)
				j=searchTarget(T->type_id,'P');

			int varindex;

			if(j>=0&&symbolTable.symbols[j].paramnum==nowfunc)
            {
                if(i>j)//说明为局部变量
                {
                    varindex=i;
                }
                else//说明是参数或外部变量
                {
                    varindex=j;
                }
			}
			else
			{
				if(i<0)
				{
					errormsg(33,T->pos,T->type_id,"Undefined ID");
					return -1;
				}
				varindex=i;
			}

			result.kind=ID_IR;
            strcpy(result.id,createAddr());

			op2.kind=ID_IR;
            strcpy(op2.id,symbolTable.symbols[varindex].alias);
			result.offset=symbolTable.symbols[varindex].offset;
			result.level=symbolTable.symbols[varindex].level;

			expresultindex=fillSymbolTable(NULL,result.id,LV,0,'T',1,0);

			T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));
			//变量调用，为临时变量，这个的原因是方便目标代码生成，必须将变量存入寄存器

			return symbolTable.symbols[varindex].type+3;
		}
		case STRUCTCALL:
		{
			int type;
			int i=searchTarget(T->type_id,'B');

			if(i<0)
			{
				errormsg(34,T->pos,T->type_id,"Undeclared struct variable");
				return -1;
			}

            int master=searchTarget(symbolTable.symbols[symbolTable.symbols[i].paramnum].name,'S');

			int j=1;
			struct node *temp=T->ptr[0];

			while(symbolTable.symbols[master+j].flag=='N'&&symbolTable.symbols[master+j].paramnum==master)
			{
			    //成员变量比对
				if(!strcmp(symbolTable.symbols[master+j].name,temp->type_id))
				{
				    result.kind=ID_IR;
                    strcpy(result.id,createAddr());
                    expresultindex=fillSymbolTable(NULL,result.id,LV,0,'T',1,0);//结构调用值

                    op2.kind=ID_IR;
                    strcpy(op2.id,temp->type_id);//成员变量
                    op2.offset=temp->offset;//成员变量对应的偏移量

                    op1.kind=ID_IR;
                    strcpy(op1.id,symbolTable.symbols[i].name);//结构变量
                    op1.offset=symbolTable.symbols[i].offset;
                    op1.level=symbolTable.symbols[i].level;

                    result.offset=op1.offset+op2.offset;
                    result.level=op1.level;
                    T->code=merge(2,T->code,genIR(STRUCT_IR,op1,op2,result));
                    //格式：result(temp)=op1.op2，即op1+op2，若result不存在（kind==0）则无返回值

					return symbolTable.symbols[master+j].type+3;
				}
				j++;
			}
			errormsg(35,T->pos,temp->type_id,"No such member variable in the struct");
			return -1;
		}
		case INT_CONST:

		    result.kind=ID_IR;
            strcpy(result.id,createTemp());
            expresultindex=fillSymbolTable(NULL,result.id,LV,0,'T',1,0);//结构调用值

            op2.kind=INT_CONST;
            op2.const_int=T->type_int;
            T->code=merge(2,T->code,genIR(ASSIGN,op1,op2,result));

			return 0;
			break;
		case CHAR_CONST:
		    errormsg(0,T->pos,"#","Not support char constant, temporarily");
			return -1;
			break;
		case FLOAT_CONST:
		    errormsg(0,T->pos,"#","Not support float constant, temporarily");
			return -2;
			break;
		case STRING_CONST:
			errormsg(36,T->pos,"String_const","not support in the expression, temporarily");
			return -1;

	}

}



/*重复声明判断*/
/*涉及全部为外部变量*/
/*局部变量可以与任何外部声明重名，不论是外部函数，结构声明等，计算均以局部声明为准*/
/*变量，数组，结构变量只能与参数，成员变量重名*/
/*函数原型可以与函数定义，参数，成员变量重名*/
/*函数定义可以与函数原型，参数，成员变量重名*/
/*结构名可以与参数，函数原型， 函数定义，成员变量重名*/
/*成员变量可以与除本结构体成员变量外一切重名*/
/*参数可以与除本函数参数外一切重名*/
/*此函数保证外部变量命名规范，不保证局部变量命名规范，因此判断局部变量还要在结构体内加入相关判断*/
/*为了防止极端情况，个人认为还需要一定的修改，有可能出现变量之前就是一个成员变量，而成员变量之前的变量就被掩盖了,已解决*/
int redec(char* name,char type,int master)
{
	switch(type)
	{
		int i=0;
		char typef;
		case 'V':
		case 'A':
		case 'B':
			for (i=symbolTable.index-1;i>=0;i--)
				if (!strcmp(symbolTable.symbols[i].name,name))
				{
					typef=symbolTable.symbols[i].flag;
					if(typef!='N'&&typef!='P')
						return 0;
				}
			break;
		case 'D':
			for (i=symbolTable.index-1;i>=0;i--)
				if (!strcmp(symbolTable.symbols[i].name,name))
				{
					typef=symbolTable.symbols[i].flag;
					if(typef!='N'&&typef!='P'&&typef!='F')
						return 0;
				}
			break;
		case 'F':
			for (i=symbolTable.index-1;i>=0;i--)
					if (!strcmp(symbolTable.symbols[i].name,name))
					{
						typef=symbolTable.symbols[i].flag;
						if(typef!='N'&&typef!='P'&&typef!='D')
							return 0;
					}
				break;
		case 'S':
			for(i=symbolTable.index-1;i>=0;i--)
					if (!strcmp(symbolTable.symbols[i].name,name))
					{
						typef=symbolTable.symbols[i].flag;
						if(typef!='N'&&typef!='P'&&typef!='D'&&typef!='F')
							return 0;
					}
				break;
		case 'N'://默认的是从后向前找，因此同结构体的成员变量冲突一定出现在查找其它定义之前
			for(i=symbolTable.index-1;i>=0;i--)
				if (!strcmp(symbolTable.symbols[i].name,name))
				{
					typef=symbolTable.symbols[i].flag;
					if(symbolTable.symbols[i].paramnum==master&&typef=='N')
						return 0;
				}
			break;
		case 'P'://默认的是从后向前找，因此同函数的参数冲突一定出现在查找其它定义之前
			for(i=symbolTable.index-1;i>=0;i--)
				if (!strcmp(symbolTable.symbols[i].name,name))
				{
					typef=symbolTable.symbols[i].flag;
					if(symbolTable.symbols[i].paramnum==master&&typef=='P')
						return 0;
				}
			break;
		default:
			break;
	}
	return 1;
}
/*局部变量重复检查*/
/*任何局部变量只要连续的同层次没有相同声明即可*/
/*连续同层次意味着同一复合语句块，不可能出现其它情况*/
/*因为复合语句退出时会删除对应语句块的局部变量*/
int localRedec(char* name,int LV)
{
	int i=symbolTable.index-1;
	while(symbolTable.symbols[i].level==LV)
	{
		if(!strcmp(symbolTable.symbols[i].name,name))
			return 0;
		i--;
	}
	return 1;
}

void errormsg(int No,int line, char *msg1, char *msg2)
{
    NoIRgenerate=1;
    printf("No:%d Line:%d, %s %s\n",No, line, msg1, msg2);
}

/*搜索特定定义的符号表，函数用于判断是否有这样的声明，一般这样的声明是独一无二的*/
int searchTarget(char *name,char flag)
{
    int i;
    for (i = symbolTable.index - 1; i >= 0; i--)
        if (!strcmp(symbolTable.symbols[i].name, name)&&symbolTable.symbols[i].flag==flag)
            return i;
    return -1;
}


int parammatch(int dec,int def)
{
    int k=0;
    int pnum;

    if(symbolTable.symbols[dec].type!=symbolTable.symbols[def].type)
        return 1;

    if(symbolTable.symbols[dec].paramnum!=symbolTable.symbols[def].paramnum)
        return 2;

    pnum=symbolTable.symbols[dec].paramnum;

    while(k<=pnum)
    {
        if(symbolTable.symbols[dec+k].type!=symbolTable.symbols[def+k].type)
            return 3;
        k++;
    }
    return 0;

}

int getresult(struct node* T)
{
    if(T==NULL)
        return 0;
    if(T->kind==INT_CONST)
    {
        return T->type_int;
    }
    else
    {
        switch(T->kind)
        {
            case PLUS:
                return getresult(T->ptr[0])+getresult(T->ptr[1]);
                break;
            case MINUS:
                if(T->ptr[1]!=NULL)
                    return getresult(T->ptr[0])-getresult(T->ptr[1]);
                else
                    return -getresult(T->ptr[0]);
                break;
            case MUL:
                return getresult(T->ptr[0])*getresult(T->ptr[1]);
                break;
            case DIV:
                return getresult(T->ptr[0])/getresult(T->ptr[1]);
                break;
            case MOD:
                return getresult(T->ptr[0])%getresult(T->ptr[1]);
                break;
            case LSHFIT:
                return getresult(T->ptr[0])<<getresult(T->ptr[1]);
                break;
            case RSHFIT:
                return getresult(T->ptr[0])>>getresult(T->ptr[1]);
                break;
            default:
                errormsg(0,T->pos,"#","Not Int Const");
                return 0;
                break;

        }
    }
}


/*填充符号表*/
int fillSymbolTable(char *name,char *alias, int level, int type, char flag,int local,int offset)
{

    int i;
	i=symbolTable.index;

    //填写符号表内容
    if(name==NULL)
        strcpy(symbolTable.symbols[i].name, "");
    else
        strcpy(symbolTable.symbols[i].name, name);
    if(alias==NULL)
        strcpy(symbolTable.symbols[i].alias, "");
    else
        strcpy(symbolTable.symbols[i].alias, alias);
    symbolTable.symbols[i].level = level;
    symbolTable.symbols[i].type = type;
    symbolTable.symbols[i].flag = flag;
    symbolTable.symbols[i].local = local;
    symbolTable.symbols[i].offset = offset;
    return symbolTable.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

struct codenode *genIR(int op,struct opn opn1,struct opn opn2,struct opn result)
{
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=op;
    h->opn1=opn1;
    h->opn2=opn2;
    h->result=result;
    h->flag='N';
    h->next=h;
    h->prior=h;
    return h;
}

//生成一条标号语句，返回头指针
struct codenode *genLabel(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=LABEL_IR;
    strcpy(h->result.id,label);
    h->flag='H';
    h->next=h->prior=h;
    return h;
}

//生成GOTO语句，返回头指针
struct codenode *genGoto(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=GOTO_IR;
    strcpy(h->result.id,label);
    h->flag='N';
    h->next=h->prior=h;
    return h;
}

//合并多个中间代码的双向循环链表，首尾相连
struct codenode *merge(int num,...){
    struct codenode *h1,*h2,*p,*t1,*t2;
    va_list ap;
    va_start(ap,num);
    h1=va_arg(ap,struct codenode *);
    while (--num>0) {
        h2=va_arg(ap,struct codenode *);
        if (h1==NULL) h1=h2;
        else if (h2){
            t1=h1->prior;
            t2=h2->prior;
            t1->next=h2;
            t2->next=h1;
            h1->prior=t2;
            h2->prior=t1;
            }
        }
    va_end(ap);
    return h1;
}
