%{
#include <stdio.h>
#include<string.h>
  #include<stdlib.h>
#include "parser.tab.h"
#include "ast.h"

int yyerror(char *s);
int yylex();

extern int linenum;
extern struct ast_node *ast;

%}

// Symbols.
%union
{
        char *var;
        int val;
        struct ast_node *ast_ptr;
        struct var_list *var_list_ptr;
};
%token <var> operators
%token <val> values
%token TOKEN_DATUM
%token TOKEN_COMMA
%token TOKEN_OPERATOR
%token TOKEN_EXPAND
%token TOKEN_INPUT
%token TOKEN_OUTPUT
%token TOKEN_MAPIN
%token TOKEN_MAPOUT
%token TOKEN_OP
%token TOKEN_CL
%token TOKEN_CONST
%token SEMICOLON
%token TOKEN_SUBGRAPH
%token TOKEN_TERMINATE
%token <var> variable
%start prog

%type <ast_ptr> Datum
%type <ast_ptr> Const
%type <ast_ptr> Input
%type <ast_ptr> Output
%type <ast_ptr> Subgraph
%type <ast_ptr> Stmts
%type <ast_ptr> Stmt
%type <ast_ptr> Operator
%type <ast_ptr> Expand
%type <ast_ptr> Scope
%type <ast_ptr> Scopes
%type <ast_ptr> Mapping
%type <ast_ptr> Mappings
%type <ast_ptr> termination

%type <var_list_ptr> terminators

%type <val> map_operator
%%

prog:
  Scopes
  {
    ast = new_program_ast_node();
    ast->down = $1;
  }
;

Scopes:
    Scope
    {
      $$ = $1;
    }
  | Scope Scopes
    {
      $1->side = $2;
      $$ = $1;
    }
  ;

Scope:
  Subgraph Stmts
  {
    $1->down = $2;
    $$ = $1;
  }
  ;


Stmts:
    {
      $$ = (struct ast_node *)0;
    }
		| Stmt Stmts
    {
      $1->side = $2;
      $$ = $1;
    };

Stmt:
      Datum     { $$ = $1; }
  |   Operator  { $$ = $1; }
  |   Const     { $$ = $1; }
  |   Expand    { $$ = $1; }
  |   Input     { $$ = $1; }
  |   Output    { $$ = $1; }
  |   termination { $$ = $1; }
	;

Subgraph:
  TOKEN_SUBGRAPH  TOKEN_OP variable TOKEN_CL 
  {
    $$ = new_subgraph_ast_node($3);
  }
  ;

termination: TOKEN_TERMINATE TOKEN_OP variable TOKEN_COMMA terminators TOKEN_CL SEMICOLON 
  {
    $$ = new_terminator_ast_node($3,$5);
  };

terminators:  variable 
              {
                $$ = (struct var_list *)malloc(sizeof(struct var_list));
                $$->next = (struct var_list*)0;
                $$->id = strdup($1);
              }
          |   variable TOKEN_COMMA terminators 
              {
                $$ = (struct var_list *)malloc(sizeof(struct var_list));
                $$->next = $3;
                $$->id = strdup($1);
              };

Datum:
  TOKEN_DATUM TOKEN_OP variable TOKEN_CL SEMICOLON  
  {
    $$ = new_datum_ast_node($3); 
  }
	;

Const:
  TOKEN_CONST TOKEN_OP variable TOKEN_COMMA values TOKEN_CL SEMICOLON 
  { 
    $$ = new_const_ast_node($3,$5); 
  }
  ;

Input:
  TOKEN_INPUT TOKEN_OP variable TOKEN_CL SEMICOLON 
  { 
    $$ = new_input_ast_node($3); 
  }
  ;

Output:
  TOKEN_OUTPUT TOKEN_OP variable TOKEN_CL SEMICOLON 
  { 
    $$ = new_output_ast_node($3); 
  }  
  ;

Operator:
  TOKEN_OPERATOR TOKEN_OP variable TOKEN_COMMA operators TOKEN_COMMA variable TOKEN_COMMA variable TOKEN_CL SEMICOLON 
  { 
    $$ = new_operator_ast_node(convert_operators($5),$3,$7,$9);
  }
	;

Expand:
  TOKEN_EXPAND TOKEN_OP variable TOKEN_COMMA  Mappings  TOKEN_CL SEMICOLON 
  {
    $$ = new_expansion_ast_node($3, $5);
  }
  ;

Mappings:
    {
      $$ = (struct ast_node *)0;
    }
  | Mapping Mappings 
    {
      $1->side = $2;
      $$ = $1;
    }
    ;


Mapping:
  map_operator TOKEN_OP variable TOKEN_COMMA variable TOKEN_CL SEMICOLON 
  {
    $$ = new_mapping_ast_node($1, $3, $5);
  }
  ;

map_operator:
    TOKEN_MAPIN 
    {
      $$ = MAPIN;
    }
  | TOKEN_MAPOUT 
    {
      $$ = MAPOUT;
    }
  ;

%%

int yyerror(char *s) {
  printf("yyerror : %s at line %d\n",s,linenum);
}


