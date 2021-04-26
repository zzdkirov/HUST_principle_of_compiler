#include "def.h"

struct node *mknode(int kind, struct node *first, struct node *second, struct node *third, struct node *forth, int pos)
{
    struct node *T = (struct node *)malloc(sizeof(struct node));
    T->kind = kind;
    if(first)first->parent=T;
    if(second)second->parent=T;
    if(third)third->parent=T;
    if(forth)forth->parent=T;
    T->ptr[0] = first;
    T->ptr[1] = second;
    T->ptr[2] = third;
    T->ptr[3] = forth;
    T->pos = pos;
    T->code=NULL;
    T->width=0;
    T->offset=0;
    return T;
}






//#define DISPAST



void display(struct node *T, int indent)
{
#ifdef DISPAST
    if (T)
    {
        switch (T->kind)
        {
        case EXTLIST:
            printf("%*cExternal Defs: \n", indent, ' ');
            display(T->ptr[0], indent + 4); //显示外部定义列表的第一个
            putchar('\n');
            display(T->ptr[1], indent); //显示外部定义列表的其他个
            break;
        case VAR:
            printf("%*cGlobal Var: \n", indent, ' ');
            display(T->ptr[0], indent+4); //显示外部(全局)变量类型
            printf("%*cVar Lists:\n", indent, ' ');
            display(T->ptr[1], indent + 4); //显示全局变量列表
            break;
        case FUNCDEC:
            printf("%*cFunc Dec:\n", indent, ' ');
            display(T->ptr[0], indent + 4); //显示函数返回类型
            display(T->ptr[1], indent + 4); //显示函数名和参数
            break;
		case FUNCDEF:
            printf("%*cFunc Def: \n", indent, ' ');
            display(T->ptr[0], indent + 4); //显示函数返回类型
            display(T->ptr[1], indent + 4); //显示函数名和参数
            display(T->ptr[2], indent + 4); //显示函数体
            break;
        case VARLIST:
			display(T->ptr[0],indent);
			if(T->ptr[1]!=NULL)
				display(T->ptr[1], indent); //下一个变量名
            break;
        case VARDEC:
            display(T->ptr[0], indent+4);//变量名
            if(T->ptr[1]!=NULL)
            {
                putchar('\n');
                printf("%*cInitial Value: ", indent+4, ' ');
                display(T->ptr[1],indent);
            }
            putchar('\n');
            break;
        case ARRDEC:
            display(T->ptr[0], indent+4);//变量名
            printf("%*cArray Lth, Dimen 1:", indent+4, ' ');
            display(T->ptr[1], indent+4);//数组长
            putchar('\n');
            if(T->ptr[2]!=NULL)
            {
                printf("%*cArray Ini Lists: ", indent+4, ' ');
                display(T->ptr[2],indent+8);
                putchar('\n');
            }
            break;
        case DIMELIST:
            display(T->ptr[0], indent+4);
            if(T->ptr[1]!=NULL)
            {
                putchar('\n');
                printf("%*cDimen next: ", indent, ' ');
                display(T->ptr[1],indent);
            }
            break;
        case STRUCTDEC:
			printf("%*cGlobal Struct Dec: \n", indent, ' ');
			printf("%*cStruct Name: ", indent+4, ' ');
            printf("%s\n",T->type_id);
			printf("%*cNumber Varible: \n", indent+4, ' ');
            display(T->ptr[0],indent+4);
            break;
        case NUMBERLIST:
            display(T->ptr[0], indent+4);
			printf("%*cNumber Varible List: \n", indent+4, ' ');
            display(T->ptr[1],indent+8);
            putchar('\n');
            display(T->ptr[2],indent);
            break;
        case STRUCTDEF:
			printf("%*cGlobal Struct Def: \n", indent, ' ');
			printf("%*cStruct Name: %s\n", indent, ' ',T->type_id);
			printf("%*cVar List: ", indent, ' ');
            display(T->ptr[0],indent);
            break;
        case STRUCTNAME:
			printf("Var: ");
            printf("%s ", T->type_id);
            display(T->ptr[0],indent);
            break;
		case DATATYPE:
			printf("%*cData Type: ", indent, ' ');
            printf("%s\n",T->type_id); //显示基本类型
            break;
        case VARNAME:
			printf("%*cVar: ", indent, ' ');
            printf("%s", T->type_id);
            if(T->ptr[0])
            {
                display(T->ptr[0],indent);
            }
            break;
        case ARRNAME:
			printf("%*cArray: ", indent, ' ');
            printf("%s\n", T->type_id);
            break;
		case ARRINITIAL:
			printf("\n%*cLists: ", indent, ' ');
			if(T->ptr[0])
                display(T->ptr[0], indent); //初始化序列
            else
                printf("String initial: %s",T->type_id);
            if(T->ptr[1])
            {
                display(T->ptr[1], indent); //初始化序列
            }
            break;
		case FUNCHEAD:
            display(T->ptr[0], indent); //函数名
			if(T->ptr[1]!=NULL)
			{
				printf("%*cPara Sequences:\n", indent, ' ');
				display(T->ptr[1], indent);//参数列表
			}
            else
				printf("%*cNo Para Func!\n", indent+4, ' ');
            break;
		case FUNCNAME:
			printf("%*cFunc Name: ", indent, ' ');
            printf("%s\n", T->type_id);
            break;
        case PARAMLIST:
            display(T->ptr[0], indent+4); //依次显示参数类型和名称
            display(T->ptr[1], indent);
            break;
        case PARAM:
            printf("%*c%s %s\n", indent, ' ', T->ptr[0]->type_id, T->ptr[1]->type_id);
            break;
        case COMPSTAT:
            printf("%*cComp Stat: {\n",indent,' ');
            display(T->ptr[0], indent + 4); //语句序列
            printf("%*c}",indent,' ');
            break;
        case STATLIST:
            display(T->ptr[0], indent); //显示第一条语句
            display(T->ptr[1], indent); //显示剩下语句
            break;
        case LOCALVAR:
            printf("%*cLocal Var: \n", indent, ' ');
            display(T->ptr[0], indent+4); //显示外部(全局)变量类型
            printf("%*cVar Lists:\n", indent, ' ');
            display(T->ptr[1], indent + 4); //显示全局变量列表
            break;
        case LOCALSTRUCTDEF:
            printf("%*cLocal Struct Def: \n", indent, ' ');
			printf("%*cStruct Name: %s\n", indent, ' ',T->type_id);
			printf("%*cVar List: ", indent, ' ');
            display(T->ptr[0],indent);
            putchar('\n');
            break;
        case EXP:
			printf("%*cExp: ", indent, ' ');
            display(T->ptr[0], indent);
            putchar('\n');
            break;
        case RETURN:
            printf("%*cStat: return: ", indent, ' ');
            display(T->ptr[0], indent + 4);
            putchar('\n');
            break;
		case BREAK:
            printf("%*cStat: break\n", indent, ' ');
            break;
		case CONTINUE:
            printf("%*cStat: continue\n", indent, ' ');
            break;
        case IF:
            printf("%*cif:\n", indent, ' ');
            printf("%*cIf Condi:", indent+4, ' ');
            display(T->ptr[0], indent + 4); //显示condition
            putchar('\n');
			printf("%*cStat:\n", indent+4, ' ');
            display(T->ptr[1], indent + 4); //显示if_body
            putchar('\n');
			if(T->ptr[2]!=NULL)
			{
				printf("%*celse:\n", indent, ' ');
				printf("%*cStat:\n", indent+4, ' ');
				display(T->ptr[2], indent + 4); //显示if_body
				putchar('\n');
			}
            break;
        case WHILE:
            printf("%*cwhile:\n", indent, ' ');
            printf("%*cWhile Condi:", indent+4, ' ');
            display(T->ptr[0], indent + 4);
            putchar('\n');
			printf("%*cStat:\n", indent+4, ' ');
            display(T->ptr[1], indent + 4);
            putchar('\n');
            break;
        case FOR:
            printf("%*cfor:\n", indent, ' ');
            display(T->ptr[0], indent + 4); //显示for1
            putchar('\n');
            display(T->ptr[1], indent + 4); //显示for2
            putchar('\n');
            display(T->ptr[2], indent + 4); //显示for3
            putchar('\n');
            display(T->ptr[3], indent + 4); //显示语句
            break;
        case FOR1:
            printf("%*cCondi 1:", indent, ' ');
            if(T->ptr[0]->kind==DATATYPE)
            {
                putchar('\n');
                printf("%*cLocal Var: \n", indent, ' ');
                display(T->ptr[0], indent + 4);
                printf("%*cVar Lists:\n", indent, ' ');
                display(T->ptr[1], indent + 4);
            }
            else
                display(T->ptr[0], indent + 4);
            break;
        case FOR2:
            printf("%*cCondi 2:", indent, ' ');
            display(T->ptr[0], indent + 4); //显示for循环第二
            break;
        case FOR3:
            printf("%*cCondi 3:", indent, ' ');
            display(T->ptr[0], indent + 4); //显示for循环第三
            break;
        case ASSIGN:
            printf("= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_PLUS:
            printf("+= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_MINUS:
            printf("-= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_MUL:
            printf("*= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_DIV:
            printf("/= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_MOD:
            printf("%%= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_LSHFIT:
            printf("<<= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case ASSIGN_RSHFIT:
            printf(">>= ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case AND:
            printf("&& ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case OR:
            printf("|| ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case RELOP:
            printf("%s ",T->type_id);
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case PLUS:
            printf("+ ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case MINUS:
            printf("- ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case MUL:
            printf("* ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case DIV:
            printf("/ ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case MOD:
            printf("%% ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case LSHFIT:
            printf("<< ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case RSHFIT:
            printf(">> ");
            display(T->ptr[0], indent+4);
            display(T->ptr[1], indent+4);
            break;
        case UMINUS:
            printf("- ");
            display(T->ptr[0], indent+4);
            break;
        case NOT:
            printf("! ");
            display(T->ptr[0], indent+4);
            break;
        case INC:
            display(T->ptr[0], indent);
            printf("++ ");
            display(T->ptr[1], indent+8);
            break;
        case DEC:
            display(T->ptr[0], indent);
            printf("-- ");
            display(T->ptr[1], indent+8);
            break;
        case FUNCCALL:
            printf("FuncCall: %s", T->type_id);
            if (T->ptr[0])
            {
                printf("ParaLists:");
                display(T->ptr[0], indent + 8);
            }
            break;
        case STRUCTCALL:
            printf("StructCall: %s ", T->type_id);
            if (T->ptr[0])
            {
                printf(".");
                display(T->ptr[0], indent + 8);
            }
            break;
        case ARRAYCALL:
            printf("ArrayCall: %s", T->type_id);
            if (T->ptr[0])
            {
                display(T->ptr[0], indent + 8);
                putchar(' ');
            }
            break;
        case CALLDIMELIST:
            printf("[");
            display(T->ptr[0],indent);
            printf("]");
            if(T->ptr[1])
            {
                display(T->ptr[1],indent);
            }
            break;
        case VARCALL:
            printf("VarCall:%s ",T->type_id);
            break;
        case INT_CONST:
            printf("CONST:");
            printf("%d ", T->type_int);
            break;
        case FLOAT_CONST:
            printf("CONST:");
            printf("%f ", T->type_float);
            break;
        case CHAR_CONST:
            printf("CONST:");
            printf("'%c' ", T->type_char);
            break;
        case STRING_CONST:
            printf("CONST:");
            printf("%s ", T->type_id);
            break;
        case ARGS:
            while(T!=NULL)
            {
                display(T->ptr[0],indent);
                T = T->ptr[1];
                if(T)
                {
                    putchar(',');
                }
            }
            break;
        }
    }
#endif // DISPAST
}
