#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *op_strings[] = {"PLUS", "TIMES", "ISEQUAL", "ISLESS", "ISGREATER", "IF", "ELSE", "MINUS", "MERGE", "END"};

void print_single_node(struct ast_node *ast, int tabs)
{
	if(ast == (struct ast_node *)0)
		return;

	int temp_tabs = tabs;

	while(temp_tabs > 0)
	{
		printf("\t");
		temp_tabs--;
	}
	switch(ast->type)
	{
		case PROGRAM:
			print_single_node(ast->down,tabs);
			break;
		case SCOPE:
			printf("%s\n",ast->name);
			print_single_node(ast->down,tabs + 1);
			print_single_node(ast->side,tabs);
			break;
		case CONSTANT:
			printf("%s: %d\n",ast->name,ast->value);
			print_single_node(ast->side,tabs);
			break;
		case DATUM:
			printf("%s\n",ast->name);
			print_single_node(ast->side,tabs);
			break;
		case INPUT:
			printf("input %s\n",ast->name);
			print_single_node(ast->side,tabs);
			break;
		case OUTPUT:
			printf("output %s\n",ast->name);
			print_single_node(ast->side,tabs);
			break;
		case OPERATOR:
			printf("%s = %s %s %s\n",ast->op_exp.op_args.dest,op_strings[ast->op_exp.op_args.op],ast->op_exp.op_args.arg1,ast->op_exp.op_args.arg2);
			print_single_node(ast->side,tabs);
			break;
		case EXPANSION:
			printf("expansion %s\n",ast->name);
			print_single_node(ast->down,tabs + 1);
			break;
		case MAPIN:
			printf("Map in %s <- %s\n",ast->op_exp.exp_args.inner,ast->op_exp.exp_args.outer);
			print_single_node(ast->side,tabs);
			break;
		case MAPOUT:
			printf("Map out %s -> %s\n",ast->op_exp.exp_args.inner,ast->op_exp.exp_args.outer);
			print_single_node(ast->side,tabs);
			break;
		default:
			return;
	}
}

//TODO....
void print_ast(struct ast_node *ast)
{
	printf("\n");
	print_single_node(ast,0);
	printf("\n");
}
