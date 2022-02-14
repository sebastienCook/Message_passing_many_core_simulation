%{
#include <stdio.h>
#include "parser.tab.h"
#include "ast.h"

extern int linenum;
%}

alpha       [a-zA-Z_][a-zA-Z0-9_]*
value       ([0-9])+  
operators   "op_PLUS"|"op_TIMES"|"op_ISEQUAL"|"op_ISLESS"|"op_ISGREATER"|"op_IF"|"op_ELSE"|"op_MINUS"|"op_MERGE"|"op_END"
comment		"//".*
%%

"\n"			{linenum++;}
{comment}		{}
"DATUM"			return(TOKEN_DATUM);
"OPERATOR"      return(TOKEN_OPERATOR);
"CONST"         return(TOKEN_CONST);
"("             return(TOKEN_OP);
")"             return(TOKEN_CL);
";"             return (SEMICOLON);
","             return (TOKEN_COMMA);
"MAP_IN"        return(TOKEN_MAPIN);
"MAP_OUT"       return(TOKEN_MAPOUT);
"EXPAND"        return(TOKEN_EXPAND);
"INPUT"         return(TOKEN_INPUT);
"OUTPUT"        return(TOKEN_OUTPUT);
"SUBGRAPH"		return(TOKEN_SUBGRAPH);
"TERMINATE"		return(TOKEN_TERMINATE);
{operators}     {
                	yylval.var = strdup(yytext); 
					return(operators);
				}
{value}         {
					yylval.val = atoi(yytext); 
					return (values);
				}
{alpha}         {
                	yylval.var = (char *)malloc(strlen(yytext)+1);
					//strncpy(yylval.var, yytext, strlen(yytext));
					strcpy(yylval.var, yytext);
					return(variable);
				}

%%

int yywrap()
{
	return 1;
}