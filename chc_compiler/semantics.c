#include "semantics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	Tests for semantic validity of the code

	Tests:
		existence of data used in operators in current scope
		existence of mapped IO data in respective scope for expansions 
*/

int check_semantics(struct ast_node *ast)
{
	int errors = 0;
	//test existence of data used in operators in current scope
	errors += check_valid_data(ast, (struct ast_node *)0);

	if(errors)
		return errors;

	//test existence of name of expansions in current AST
	errors += check_valid_expansions(ast, ast);

	if(errors)
		return errors;

	//test existence of mapped IO data in respective scope for expansions 
	errors += check_mappings_valid(ast);


	return errors;
}

int check_outer_mappings(struct ast_node *mappings, struct ast_node *scope)
{
	int errors = 0;

	while(mappings != (struct ast_node *)0)
	{
		errors += datum_exists(mappings->op_exp.exp_args.outer,scope);
		mappings = mappings->side;
	}

	return errors;
}

int check_inner_mappings(struct ast_node *mappings, struct ast_node *scope)
{
	int errors = 0;

	while(mappings != (struct ast_node *)0)
	{
		errors += datum_exists(mappings->op_exp.exp_args.inner,scope);
		mappings = mappings->side;
	}

	return errors;
}

struct ast_node *get_scope(struct ast_node *ast, char *n)
{
	//assuming ast points to program, returns pointer to scope named n, if it exists
	ast = ast->down;

	while(ast != (struct ast_node *)0)
	{
		if(strcmp(ast->name,n)==0)
			return ast;
		ast = ast->side;
	}
	return (struct ast_node *)0;
}

//ast points to a scope: for all expansions in that scope, check whether mappings exist
int check_mappings_in_scope(struct ast_node *ast, struct ast_node *ast_root)
{
	int errors = 0;
	struct ast_node *current_scope = ast;

	//ast points to statement
	ast = ast->down;

	while(ast != (struct ast_node *)0)
	{
		if(ast->type == EXPANSION)
		{
			char *n = ast->name;
			while(*n != ':')
				n++;
			n++;
			errors += check_inner_mappings(ast->down, get_scope(ast_root ,n));
			errors += check_outer_mappings(ast->down, current_scope);
		}
		ast = ast->side;
	}

	return errors;
}

//For every expansion, checks whether mappings in that expansion are valid
//i.e., outer data exists in current scope and inner data exists in target scope
int check_mappings_valid(struct ast_node *ast)
{
	int errors = 0;
	//keep a copy of pointer to program root
	struct ast_node *ast_root = ast;

	//down to scope level
	ast = ast->down;

	while(ast != (struct ast_node *)0)
	{
		errors += check_mappings_in_scope(ast, ast_root);
		ast = ast->side;
	}

	return errors;
}

//Checks that subgraph names in expansions correspond to valid scopes
int check_valid_expansions(struct ast_node *ast, struct ast_node *ast_root)
{
	int errors = 0;
	if(ast == (struct ast_node *)0)
		return errors;


	switch(ast->type)
	{
		case PROGRAM:
			errors += check_valid_expansions(ast->down, ast_root);
			break;
		case SCOPE:
			errors += check_valid_expansions(ast->down, ast_root);
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case CONSTANT:
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case DATUM:
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case INPUT:
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case OUTPUT:
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case OPERATOR:
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		case EXPANSION:
			errors += check_scope_exists(ast->name, ast_root);
			errors += check_valid_expansions(ast->side, ast_root);
			break;
		default:
			return errors;
	}


	return errors;
}


//Finds all data use in operators
//calls check existence in scope to test for existence
//adds declarations to symbol table for checking
int check_valid_data(struct ast_node *ast, struct ast_node *scope)
{
	int errors = 0;
	if(ast == (struct ast_node *)0)
		return errors;


	switch(ast->type)
	{
		case PROGRAM:
			errors += check_valid_data(ast->down, (struct ast_node *)0);
			break;
		case SCOPE:
			errors += check_valid_data(ast->down, ast);
			errors += check_valid_data(ast->side, (struct ast_node *)0);
			break;
		case CONSTANT:
			errors += check_valid_data(ast->side, scope);
			break;
		case DATUM:
			errors += check_valid_data(ast->side, scope);
			break;
		case INPUT:
			errors += datum_exists(ast->name, scope);
			errors += check_valid_data(ast->side, scope);
			break;
		case OUTPUT:
			errors += datum_exists(ast->name, scope);
			errors += check_valid_data(ast->side, scope);
			break;
		case OPERATOR:
			errors += datum_exists(ast->op_exp.op_args.dest, scope);
			errors += datum_exists(ast->op_exp.op_args.arg1, scope);
			errors += datum_exists(ast->op_exp.op_args.arg2, scope);
			errors += check_valid_data(ast->side, scope);
			break;
		case EXPANSION:
			errors += check_valid_data(ast->side, scope);
			break;
		default:
			return errors;
	}
	return errors;
}

//returns 0 if datum exists in current scope, 1 if not found
int datum_exists(char *d, struct ast_node *scope)
{
	char *scope_name = scope->name;

	if(scope == (struct ast_node *)0)
		return 0;

	//move down to statement list
	scope = scope->down;

	while(scope != (struct ast_node *)0)
	{
		if((scope->type == DATUM) || (scope->type == CONSTANT))
		{
			if(strcmp(scope->name,d)==0)
				return 0;
		}
		scope = scope->side;
	}
	printf("Datum %s not found in scope %s\n",d,scope_name);
	return 1;
}

//returns 0 if scope with given name exists, 1 if not found
int check_scope_exists(char *n, struct ast_node *ast_root)
{
	//always called pointing to program
	ast_root = ast_root->down;

	while(*n != ':')
		n++;
	n++;

	//now at scope level
	while(ast_root != (struct ast_node *)0)
	{
		if(strcmp(ast_root->name,n)==0)
			return 0;
		ast_root = ast_root->side;
	}
	printf("Could not find scope \"%s\" for expansion.\n",n);
	return 1;
}