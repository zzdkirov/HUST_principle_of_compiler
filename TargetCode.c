#include "def.h"

/*目标代码生成，根据中间代码序列，选择合适的mips指令*/

/*目前暂时支持的mips指令为*/

/*
add,addi,sub,subi,mul,and,andi,or,ori,beq,bne,bgt,bge,blt,ble,sllv,srlv,lw,sw,j,jal,jr
*/

/*
主要是因为电路的原因，上面的指令也涵盖了R，I，J三种类型的指令，
*/
/*寄存器分配算法采用局部寄存器分配算法，将程序代码按块划分但不优化，其它算法实现起来比较麻烦*/

FILE* fp;

struct codenode* head=NULL;

struct codenode* tail=NULL;

int regState[10];//未分配为0，否则为引用计数
//依次为8-17号寄存器

//此二为在具体代码生成过程中的用途

struct registerSymbol regtable[100];

int top;

char targetcode[30];

int cmpreg;
int cmpregindex;//取值只有16 17
//这个变量主要指示，若当前处于比较语句的赋值，则赋值默认为相同寄存器，即如果第一次赋值为16号，第二次也为16号寄存器


/*翻译原则
1.类似于addr=v的语句翻译为lw，分配寄存器
2.类似于temp=const的语句，先临时记录该const的值，如果后续有使用翻译为addi等，若无使用则没有任何意义
按照常理，这个temp一定是用到的
3.运算类语句均翻译为对应的三地址语句
4.赋值语句翻译为sw，返回的结果为被赋值的寄存器值
5.arg翻译为入栈
6.parameter视为变量定义，同样有偏移量值
7.function语句为标号，
8.call语句翻译为开辟栈帧，返回值从$v0取
9.return语句翻译为返回值写入$v0，jr
10.goto语句翻译为j
11.if语句翻译为beq $0
12.eq,ne,gt,ge,lt,le分别对应于上述六种判断语句
13.标号语句原样翻译

开辟临时变量栈，标识为常量或变量，对于常量直接以常量运算，对于变量则需要写入符号栈并明确对应寄存器
*/



void generateTargetCode(struct codenode* root)
{

        fp=fopen("F:\\Y3\\compliationexp\\lab4\\TargetCode\\target.asm","w+");

        /*首先要有system的调用，作为一个函数原型，没有什么意义*/
        printf("\n\n\n");
        label("system");
        push(31);//返回地址进栈
        push(29);//ebp进栈
        addi(29,30,0);//ebp=esp
        //由于无参因此无需开拓栈帧
        lw(4,0,8,1);//打印参数为ebp+8，送四号寄存器
        lw(3,0,12,1);//控制参数为ebp+12，送三号寄存器
        printf("syscall\n");
        fprintf(fp,"syscall\n");

        addi(30,29,0);//esp=ebp
        pop(29);
        pop(31);
        jr(31);

        head=root;
        struct opn *op1;
        struct opn *op2;
        struct opn *res;
        while(head->op!=END_IR)
        {
            op1=&(head->opn1);
            op2=&(head->opn2);
            res=&(head->result);
            switch(head->op)
            {
                case FUNC_IR:
                {
                    printf("\n\n\n");
                    label(res->id);
                    if(!strcmp(res->id,"main"))
                    {
                        addi(28,0,512);
                        addi(29,0,2048);
                        addi(30,0,2048);
                    }
                    //被调用者保护现场
                    push(31);//返回地址进栈
                    push(29);//ebp进栈
                    addi(29,30,0);//ebp=esp
                    addi(30,30,-getframesize(root,res->id));

                    push(8);
                    push(9);
                    push(10);
                    push(11);
                    push(12);
                    push(13);
                    push(14);
                    push(15);
                    push(16);
                    push(17);
                    break;
                }
                case PARAM_IR:
                {
                    //不处理
                    break;
                }
                case ASSIGN:
                {
                    //assign有四类，一类是常量赋值temp，一类是变量赋值addr
                    //一类是temp（常量）赋值v，一类是变量（addr）赋值v
                    //注意，常量一旦出现在非assign处需要立即释放，16为专门的constreg
                    //有常量就直接往16号寄存器写就行了，个人完成的所有常量都是即取即用
                    if(op2->kind==INT_CONST)//temp=const情况
                    {
                        strcpy(regtable[top].tempvar,res->id);
                        if(cmpreg==2)
                        {
                            //绝不会出现两个常量寄存器都被占用还需要赋值常量的情况
                            if(regState[8]==1)
                            {
                                regtable[top].reg=17;
                                regState[9]=1;
                                cmpregindex=17;
                            }
                            else
                            {
                                regtable[top].reg=16;
                                regState[8]=1;
                                cmpregindex=16;
                            }
                            cmpreg--;
                        }
                        else if(cmpreg==1)
                        {
                            regtable[top].reg=cmpregindex;
                            cmpreg--;
                            //赋值时两者用同一寄存器
                        }
                        else//不涉及比较的常量寄存器分配
                        {
                            if(regState[8]==1)//16号寄存器被占用
                            {
                                regtable[top].reg=17;
                                regState[9]=1;
                            }
                            else
                            {
                                regtable[top].reg=16;
                                regState[8]=1;
                            }
                        }

                        addi(regtable[top].reg,0,op2->const_int);
                        regtable[top++].flag=0;
                    }
                    else if(res->id[0]=='a')//addr=v情况
                    {
                        strcpy(regtable[top].tempvar,res->id);
                        regtable[top].reg=allocRegister();
                        regtable[top].flag=1;
                        lw(regtable[top].reg,0,-res->offset,res->level>0);
                        regtable[top].addrreg=allocRegister();
                        addi(regtable[top++].addrreg,0,-res->offset);
                        //level>0表示为局部变量，offset需要从ebp取，否则offset从gp取
                    }
                    else if(res->id[0]=='v'&&op2->id[0]=='t')//v=temp情况，相当于变量赋初值，结果不需寄存器，释放op2寄存器即可
                    {
                        int regresult=getopnum(op2->id);

                        sw(regtable[regresult].reg,0,-res->offset,res->level>0);

                        revokeRegister(regtable[regresult].reg);//赋初值寄存器需要释放

                    }
                    else if(res->id[0]=='v'&&op2->id[0]=='a')//v=addr情况，相当于变量对变量赋值，此时变量值存在reg，地址存在addrreg，均释放
                    {
                        int regresult=getopnum(op2->id);

                        sw(regtable[regresult].reg,0,-res->offset,res->level>0);

                        revokeRegister(regtable[regresult].reg);//赋初值寄存器需要释放

                        revokeRegister(regtable[regresult].addrreg);//赋初值地址寄存器需要释放

                    }
                    break;
                }
                case ASSIGN_IR:
                {
                    //寄存器值写入存储器
                    int regresult=getopnum(op2->id);
                    if(regresult==-1)
                        printf("ASSIGN_IR error\n");

                    int temp=regtable[regresult].reg;//op2的寄存器序号
                    int regresult2=getopnum(op1->id);//这里主要是获取地址，同时为释放寄存器变量做准备
                    int temp2=regtable[regresult2].reg;
                    int temp3=regtable[regresult2].addrreg;//op1的寄存器序号
                    //非常简单:op2寄存器内容，写入op1对应地址内容即可
                    sw(temp,temp3,0,op1->level>0);//这就是偏移量地址写于寄存器，要让它和29/28号寄存器相加

                    revokeRegister(temp2);//释放op1（地址和值）寄存器
                    revokeRegister(temp3);//释放op1（地址和值）寄存器

                    //将op2对应的寄存器写入res对应的临时变量的寄存器
                    regtable[top].reg=temp;
                    regtable[top].flag=0;
                    strcpy(regtable[top++].tempvar,res->id);

                    break;
                }
                case ARG_IR:
                {
                    //总之有常量就往16,17号寄存器写
                    int regresult=getopnum(res->id);

                    push(regtable[regresult].reg);
                    revokeRegister(regtable[regresult].reg);
                    break;
                }
                case CALL_IR:
                {
                    //顺序：$31入栈，ebp入栈，ebp=esp，开辟新栈帧（数量减），jal
                    //本实验中system作为函数原型，作用是直接打印


                    if(!strcmp(op2->id,"system"))
                    {
                        jal("system");
                    }
                    else
                    {
                        jal(op2->id);

                        if(res->kind!=0)
                        {
                            regtable[top].reg=allocRegister();//函数返回值固定存放于2号寄存器
                            strcpy(regtable[top].tempvar,res->id);
                            regtable[top].flag=0;
                            addi(regtable[top++].reg,2,0);
                        }
                    }
                    break;
                }
                case ARR_IR:
                {
                    //对于数组地址类的解决方法
                    //某个赋值assign时（包括结构和数组），将其值存入某些寄存器，将其地址也存入节点
                    //18位地址寄存器
                    int offsetresult=getopnum(op2->id);//获得存放op2值的寄存器，或op2常量

                    add(18,0,regtable[offsetresult].reg);
                    sllv(18,18,2);//地址左移两位获得字节地址
                    sub(18,0,18);//偏移量为负
                    addi(18,18,-op1->offset);
                    regtable[top].reg=allocRegister();
                    strcpy(regtable[top].tempvar,res->id);
                    lw(regtable[top].reg,18,0,op1->level>0);
                    regtable[top].addrreg=allocRegister();
                    addi(regtable[top++].addrreg,0,18);
                    revokeRegister(regtable[offsetresult].reg);//释放暂存变址的寄存器

                    break;
                }
                case STRUCT_IR:
                {
                    //由于偏移量已经写在addr结果里面，这里不用生成目标代码
                    /*暂时废弃*/
                    int addrconst=res->offset;

                    regtable[top].reg=allocRegister();
                    strcpy(regtable[top].tempvar,res->id);
                    lw(regtable[top].reg,0,addrconst,op1->level>0);
                    regtable[top].addrreg=allocRegister();
                    add(regtable[top++].addrreg,0,res->offset);
                    break;
                }
                case RETURN_IR:
                {
                    //顺序：返回值写入2号寄存器，恢复现场，esp=ebp,pop ebp，pop $31，esp减去（加上）参数个数*4个字节（已存储于节点）
                    int regresult=getopnum(res->id);
                    add(2,0,regtable[regresult].reg);//返回值写入2号寄存器

                    pop(17);
                    pop(16);
                    pop(15);
                    pop(14);
                    pop(13);
                    pop(12);
                    pop(11);
                    pop(10);
                    pop(9);
                    pop(8);

                    addi(30,29,0);//esp=ebp

                    pop(29);//pop(ebp)
                    pop(31);//pop(esp)
                    addi(30,30,res->offset);//函数参数抹掉


                    jr(31);
                    revokeRegister(regtable[regresult].reg);

                    break;
                }
                case IF_IR:
                {
                    int regresult=getopnum(res->id);

                    beq(regtable[regresult].reg,0,op1->id);
                    revokeRegister(regtable[regresult].reg);//此处存疑，以待后观

                    break;
                }
                case LABEL_IR:
                {
                    label(res->id);
                    break;
                }
                case GOTO_IR:
                {
                    j(res->id);
                    break;
                }
                case PLUS:
                case MINUS:
                case LSHFIT:
                case RSHFIT:
                case AND:
                case OR:
                {
                    //运算中的寄存器释放规则
                    //右值寄存器作为运算结果的存储
                    //左值寄存器若非地址寄存器，则释放
                    //如此两常量寄存器中便有一个可以释放
                    int regresult1=getopnum(op1->id);
                    int reg1,reg2;
                    reg1=regtable[regresult1].reg;

                    int regresult2=getopnum(op2->id);
                    reg2=regtable[regresult2].reg;


                    regtable[top].reg=reg2;
                    strcpy(regtable[top].tempvar,res->id);
                    regtable[top].flag=0;

                    switch(head->op)
                    {
                        case PLUS:
                            add(regtable[top++].reg,reg1,reg2);
                            break;
                        case MINUS:
                            sub(regtable[top++].reg,reg1,reg2);
                            break;
                        case LSHFIT:
                            sllv(regtable[top++].reg,reg1,reg2);
                            break;
                        case RSHFIT:
                            srlv(regtable[top++].reg,reg1,reg2);
                            break;
                        case AND:
                            and(regtable[top++].reg,reg1,reg2);
                            break;
                        case OR:
                            or(regtable[top++].reg,reg1,reg2);
                            break;
                    }
                    if(regtable[regresult1].flag==0)
                    {
                        revokeRegister(reg1);
                    }
                    break;
                }
                case EQ_IR:
                case NE_IR:
                case GT_IR:
                case GE_IR:
                case LT_IR:
                case LE_IR:
                {

                    int regresult1=getopnum(op1->id);
                    int reg1,reg2;
                    reg1=regtable[regresult1].reg;


                    int regresult2=getopnum(op2->id);

                    reg2=regtable[regresult2].reg;

                    cmpreg=2;//每进行一次赋值减1
                    switch(head->op)
                    {
                        case EQ_IR:
                            beq(reg1,reg2,res->id);
                            break;
                        case NE_IR:
                            bne(reg1,reg2,res->id);
                            break;
                        case GT_IR:
                            bgt(reg1,reg2,res->id);
                            break;
                        case GE_IR:
                            bge(reg1,reg2,res->id);
                            break;
                        case LT_IR:
                            blt(reg1,reg2,res->id);
                            break;
                        case LE_IR:
                            ble(reg1,reg2,res->id);
                            break;
                    }
                    revokeRegister(reg1);
                    revokeRegister(reg2);

                    break;
                }
                case MUL:
                case DIV:
                case MOD:
                {
                    printf("Not support opreator\n");
                    break;
                }
                case END_IR:
                {
                    //程序结束，return
                    return;
                    break;
                }
                default:
                    break;
            }
            head=head->next;

        }


    fclose(fp);
}

/*
用到的寄存器：
$2（$v0）：RETURN的返回值
$8-$15，（$t0-$t7）:临时变量寄存器,8个，去掉常量应该够用，
做如下规定：8,9,10为临时变量寄存器，rs=8,rt=9,rd=10


$16-$23 :$16为常量寄存器，用于存储临时常量进行运算
$17为地址寄存器，存储变量地址（数组，结构，变量）
$18为辅助常量寄存器
$28:全局变量基址，开始时置为0x200
$29:%ebp，程序开始时置为0x800
$30:%esp，程序开始时置为0x800
（main函数不带参，特殊处理，在0x800视为存储main调用者的ebp，main的局部变量从0x7fc开始，
main的返回地址指向0x0，0x0存储一条停机指令，然后再开始代码生成）
$31:返回地址

*/

int getframesize(struct codenode* root,char* s)
{
    if(!strcmp(s,"system"))
        return 0;

    while(root->op!=END_IR)
    {
        if(root->op==FUNC_IR&&(!strcmp(root->result.id,s)))
            return root->result.offset;
        else
            root=root->next;
    }
    printf("debug error 6\n");
    return -1;
}


/*
运行时存储组织：
从高地址到低地址：调用者栈帧：参数n......参数1，函数栈帧：（此处为当前函数ebp）调用者ebp，局部变量......
返回地址存于$31，esp指向栈帧末尾，用以进行参数入栈操作
相较于X86主要是返回地址的改变
生成目标代码时要注意判断函数使用的寄存器的个数
*/
int allocRegister()
{
    for(int i=0;i<8;i++)
    {
        if(regState[i]==0)
        {
            regState[i]=1;
            return i+8;
        }
    }
    printf("reg is not enough for use\n");
    return -1;
}

/*
寄存器回收原则：
1.return ，arg，无条件回收
2.比较指令左右值
3.赋值指令左右值（非结果值）（包括地址寄存器和数值寄存器）

*/
void revokeRegister(int reg)
{
    for(int i=0;i<top;i++)
    {
        if(regtable[i].reg==reg&&regtable[i].flag==1)
        {
            regtable[i].flag=0;
            if(regtable[i].tempvar[0]=='a')//地址变量
            {
                regState[regtable[i].addrreg-8]=0;
            }
        }

    }
    regState[reg-8]=0;
}

/*根据tvar获取其寄存器或常量，为寄存器返回0，为常量返回1*/
int getopnum(char* tvar)
{
    for(int i=0;i<top;i++)
    {
        if(!strcmp(regtable[i].tempvar,tvar))
        {
            return i;
        }
    }
    return -1;
}

int getregtableindex(char* tvar)
{
    for(int i=0;i<top;i++)
    {
        if(!strcmp(regtable[i].tempvar,tvar)&&regtable[i].flag!=0)
        {
            return i;
        }
    }
    return -1;
}

int getBaseBlock(struct codenode* root)
{
    //入口语句
    if(head==NULL)
        head=root;
    else
    {
        if(tail->op!=END_IR)
            head=tail->next;
        else
            return 0;
        while(head->op!=END_IR)
        {
            head=head->next;
            if(head->flag=='H')
                break;
        }
    }

    if(head->op==END_IR)
    {
        return 0;
    }
    //结尾语句

    tail=head->next;
    while(tail->op!=END_IR)
    {
        if(tail->flag=='H')
        {
            tail=tail->prior;
            break;
        }
        else if((tail->op>=IF_IR&&tail->op<=LE_IR)||tail->op==GOTO_IR)
        {
            break;
        }
        tail=tail->next;
    }
    return 1;
}


void sw(int rd,int rt,int offset,int base)
{
    if(base>0)
    {
        add(18,0,rt);
        add(18,18,29);
        sprintf(targetcode,"sw $%d,%d($18)\n",rd,offset);
    }
    else
    {
        add(18,0,rt);
        add(18,18,28);
        sprintf(targetcode,"sw $%d,%d($18)\n",rd,offset);
    }
    recordcode();
}

void lw(int rd,int rt,int offset,int base)
{
    if(base>0)
    {
        add(18,0,rt);
        add(18,18,29);
        sprintf(targetcode,"lw $%d,%d($18)\n",rd,offset);
    }
    else
    {
        add(18,0,rt);
        add(18,18,28);
        sprintf(targetcode,"lw $%d,%d($18)\n",rd,offset);
    }
    recordcode();
}

void push(int rd)
{
    sprintf(targetcode,"addi $30,$30,-4\n");
    recordcode();
    sprintf(targetcode,"sw $%d,0($30)\n",rd);
    recordcode();
}

void pop(int rd)
{
    sprintf(targetcode,"lw $%d,0($30)\n",rd);
    recordcode();
    sprintf(targetcode,"addi $30,$30,4\n");
    recordcode();
}

void label(char* s)
{
    sprintf(targetcode,"%s:\n",s);
    recordcode();
}

void addi(int rd,int rs,int imm)
{
    sprintf(targetcode,"addi $%d,$%d,%d\n",rd,rs,imm);
    recordcode();
}

void subi(int rd,int rs,int imm)
{
    sprintf(targetcode,"subi $%d,$%d,%d\n",rd,rs,imm);
    recordcode();
}

void add(int rd,int rs,int rt)
{
    sprintf(targetcode,"add $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void sub(int rd,int rs,int rt)
{
    sprintf(targetcode,"sub $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void sllv(int rd,int rs,int rt)
{
    sprintf(targetcode,"sllv $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void srlv(int rd,int rs,int rt)
{
    sprintf(targetcode,"srlv $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void and(int rd,int rs,int rt)
{
    sprintf(targetcode,"and $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void or(int rd,int rs,int rt)
{
    sprintf(targetcode,"or $%d,$%d,$%d\n",rd,rs,rt);
    recordcode();
}

void jr(int rd)
{
    sprintf(targetcode,"jr $%d\n",rd);
    recordcode();
}
void j(char* s)
{
    sprintf(targetcode,"j %s\n",s);
    recordcode();
}

void jal(char* s)
{
    sprintf(targetcode,"jal %s\n",s);
    recordcode();
}

void beq(int rd,int rt,char* s)
{
    sprintf(targetcode,"beq $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void bne(int rd,int rt,char* s)
{
    sprintf(targetcode,"bne $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void bgt(int rd,int rt,char* s)
{
    sprintf(targetcode,"bgt $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void bge(int rd,int rt,char* s)
{
    sprintf(targetcode,"bge $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void blt(int rd,int rt,char* s)
{
    sprintf(targetcode,"blt $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void ble(int rd,int rt,char* s)
{
    sprintf(targetcode,"ble $%d,$%d,%s\n",rd,rt,s);
    recordcode();
}

void recordcode()
{
    printf("%s",targetcode);
    fprintf(fp,"%s",targetcode);
    //重定向到文件
}
