%{
#include "parser.tab.h"
#include "string.h"
#include "def.h"

int yycolumn=1;
#define YY_USER_ACTION yylloc.first_line=yylloc.last_line=yylineno; \
	yylloc.first_column=yycolumn; yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;
typedef union {
	int type_int;
	int type_float;
	char type_char;
	char type_id[32];
	struct node *ptr;
} YYLVAL;
#define YYSTYPE YYLVAL

%}
%option yylineno



id   [A-Za-z_][A-Za-z0-9_]*
int    [0-9]+
float  ([0-9]*\.[0-9]+)|([0-9]+\.)|([0-9]*[Ee][+-]?[0-9]+)
char  \'.\'
udefid [0-9@#$]+[A-Za-z_]+

%%
{int}       {yylval.type_int=atoi(yytext);return INT_CONST;}
{float}     {yylval.type_float=atof(yytext); return FLOAT_CONST;}
{char}      {yylval.type_char=yytext[1];return CHAR_CONST;}
"\""         {yylval.type_id[0]='"';
			  char c;
			  int i=1;
			  
			  while((c=input())!='"'&&c!=0)
			  {
				yylval.type_id[i++]=c;
			  }
			  
			  yylval.type_id[i++]='"';
			  yylval.type_id[i]='\0';
			  return STRING_CONST;}
"int"        {strcpy(yylval.type_id,  yytext);return DATATYPE;}
"char"       {strcpy(yylval.type_id,  yytext);return DATATYPE;}
"float"      {strcpy(yylval.type_id,  yytext);return DATATYPE;}

"return"     {return RETURN;}
"break"      {return BREAK;}
"continue"   {return CONTINUE;}
"struct"     {return STRUCT;}
"if"         {return IF;}
"else"       {return ELSE;}
"while"      {return WHILE;}
"for"        {return FOR;}
"void"		 {return VOID;}

{id}        {strcpy(yylval.type_id,  yytext); return ID;}

";"			{return SEMI;}
","			{return COMMA;}
"."         {printf("Op: .\n");return DOT;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext);return RELOP;}
"="			{return ASSIGN;}
"+"			{return PLUS;}
"+="		{return ASSIGN_PLUS;}
"++"		{return INC;}
"-"			{return MINUS;}
"-="		{return ASSIGN_MINUS;}
"--"		{return DEC;}
"*"			{return MUL;}
"*="		{return ASSIGN_MUL;}
"/"			{return DIV;}
"/="		{return ASSIGN_DIV;}
"&&"       	{return AND;}
"||"        {return OR;}
"%"         {return MOD;}
"%="        {return ASSIGN_MOD;}
"!"			{return NOT;}
">>"        {return RSHFIT;}
">>="       {return ASSIGN_RSHFIT;}
"<<"        {return LSHFIT;}
"<<="       {return ASSIGN_LSHFIT;}
"("			{return LP;}
")"			{return RP;}
"["         {return LB;}
"]"         {return RB;}
"{"			{return LC;}
"}"			{return RC;}
[\n]        {yycolumn=1;}
[ \r\t]     {}
"//"		{char c=input();while(c!='\n') c=input();}/*lint note*/
"/*"        {Note();}/*full note*/
.			{printf("Error type A :Mysterious character \"%s\"\t at Line %d\n",yytext,yylineno);}
{udefid}	{printf("Error type A :Mysterious character \"%s\"\t at Line %d\n",yytext,yylineno);}

%%
int yywrap()
{
	return 1;
}

int Note()
{
	char c,c1;
	while(1)
	{
		while((c=input())!='*'&&c!=0);
		if((c1 = input())!='/'&&c!=0)
		{
			unput(c1);
		}
		else
			return 0;
	}
}
