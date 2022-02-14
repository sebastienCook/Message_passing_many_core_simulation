#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

//at the end of parsing, pointer to root of AST
struct ast_node *ast;




//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_datum_ast_node(char *n)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = DATUM;
	result->name = (char *)malloc(strlen(n)+1);
	strcpy(result->name,n);
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;
	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_const_ast_node(char *n, int v)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = CONSTANT;
	result->name = (char *)malloc(strlen(n)+1);
	strcpy(result->name,n);
	result->value = v;
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;
	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_input_ast_node(char *n)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = INPUT;
	result->name = (char *)malloc(strlen(n)+1);
	strcpy(result->name,n);
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;
	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_output_ast_node(char *n)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = OUTPUT;
	result->name = (char *)malloc(strlen(n)+1);
	strcpy(result->name,n);
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;
	return result;
}

int func_ctr = 0;

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_subgraph_ast_node(char *n)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = SCOPE;
	result->name = (char *)malloc(strlen(n)+1);
	strcpy(result->name,n);
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;

	//reset function call counter
	func_ctr = 0;

	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_program_ast_node()
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = PROGRAM;
	
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;
	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_operator_ast_node(enum operator op, char *d, char *s1, char *s2)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = OPERATOR;

	result->op_exp.op_args.op = op;
	result->op_exp.op_args.dest = (char *)malloc(strlen(d)+1);
	strcpy(result->op_exp.op_args.dest,d);
	result->op_exp.op_args.arg1 = (char *)malloc(strlen(s1)+1);
	strcpy(result->op_exp.op_args.arg1,s1);
	result->op_exp.op_args.arg2 = (char *)malloc(strlen(s2)+1);
	strcpy(result->op_exp.op_args.arg2,s2);
	
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;

	return result;
}



struct ast_node *new_terminator_ast_node(char * n,struct var_list *list)
{
	
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = TERMINATOR;


	result->op_exp.op_args.dest = (char *)malloc(strlen(n)+1);
	strcpy(result->op_exp.op_args.dest,n);
	
	result->op_exp.op_args.arg1 = (char *)0;
	
	result->op_exp.op_args.arg2 = (char *)0;
		
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;

	result->end_vars = list;

	return result;
}





//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_mapping_ast_node(enum node_type map, char *i, char *o)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = map;

	result->op_exp.exp_args.inner = (char *)malloc(strlen(i)+1);
	strcpy(result->op_exp.exp_args.inner,i);
	result->op_exp.exp_args.outer = (char *)malloc(strlen(o)+1);
	strcpy(result->op_exp.exp_args.outer,o);
	
	result->down = (struct ast_node *)0;
	result->side = (struct ast_node *)0;

	return result;
}

//always returns pointer to newly created ast_node so we can propagate it during parsing
//arguments match content of ast_node structure required for specific node
struct ast_node *new_expansion_ast_node(char * n, struct ast_node *m)
{
	struct ast_node *result = (struct ast_node *)malloc(sizeof(struct ast_node));

	result->type = EXPANSION;

	char tmp[32];
	sprintf(tmp,"%d",func_ctr);
	func_ctr++;

	result->name = (char *)malloc(strlen(n)+2+strlen(tmp));

	strcpy(result->name,tmp);
	strcat(result->name,":");

	strcat(result->name,n);
	
	result->down = m;
	result->side = (struct ast_node *)0;

	return result;
}




//helper function for scanner: converts operator strings to internal types
enum operator convert_operators(char *n)
{
	if(strcmp("op_PLUS",n)==0)
		return PLUS;
	if(strcmp("op_TIMES",n)==0)
		return TIMES;
	if(strcmp("op_ISEQUAL",n)==0)
		return ISEQUAL;
	if(strcmp("op_ISLESS",n)==0)
		return ISLESS;
	if(strcmp("op_ISGREATER",n)==0)
		return ISGREATER;
	if(strcmp("op_IF",n)==0)
		return IF;
	if(strcmp("op_ELSE",n)==0)
		return ELSE;
	if(strcmp("op_MINUS",n)==0)
		return MINUS;
	if(strcmp("op_MERGE",n)==0)
		return MERGE;
	if(strcmp("op_END",n)==0)
		return END;
//	if(strcmp("",n)==0)
//		return 
}