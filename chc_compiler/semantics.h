#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"

int check_semantics(struct ast_node *ast);
int check_valid_data(struct ast_node *ast, struct ast_node *scope);
int datum_exists(char *d, struct ast_node *scope);
int check_scope_exists(char *n, struct ast_node *ast_root);
int check_valid_expansions(struct ast_node *ast, struct ast_node *ast_root);
int check_mappings_valid(struct ast_node *ast);
int check_outer_mappings(struct ast_node *mappings, struct ast_node *scope);
int check_inner_mappings(struct ast_node *mappings, struct ast_node *scope);
struct ast_node *get_scope(struct ast_node *ast, char *n);
int check_mappings_in_scope(struct ast_node *ast, struct ast_node *ast_root);

#endif