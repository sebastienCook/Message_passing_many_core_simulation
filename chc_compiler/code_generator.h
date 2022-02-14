#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include<stdio.h>
#include<stdlib.h>
#include "ir_generator.h"

/*
				   -1-node start marker (0x7FFFFFFF)
					0-created count
					1-value		
					2-node size, convenient to move around
					3-operation
					4-number of arguments
						NOT EXPANSION 
							0, 1 or 2 args, size 1
						EXPANSION
							any number of args, size 2
						each arg:
							() value
							offset into expansion
					5-number of destinations
						(If expansion, one additional element here: code of what to expand)
							(subgraph address) in canon code table
						NOT EXPANSION
							(absolute address), or OUTPUT code
						EXPANSION
							(absolute ) address
							() offset into expansion
				*/

/*
	Before generating code, we need some precise information about the IR

	Specifically, the size of each node (in sizeof(int)) 
	The relative positions of each named input/output mapping in expansions
	The total size of each subgraph

	So, the first thing is to calculate that for the entire IR, encapsulated in this structure
	that points to the IR, plus keeps size info 
*/
struct code_node_size
{
	int size; 		//total node size
	int arg_number; //number of arguments
	int arg_size;	//size of arguments
	int dest_number;//number of destinations
	int dest_size;	//size of destinations
};

struct annotated_IR_node
{
	struct datum_ir *IR_node;

	//additional information
	struct code_node_size size;

	struct annotated_IR_node *next;
};

struct annotated_IR_scope
{
	struct scope_ir *IR_node;

	//additional information
	int size; 		//total scope size
	int number_nodes; //number of nodes

	struct io_nodes_info
	{
		char *io_node_name;
		int io_type; //0 in, 1 out
		int io_node_offset;
		int io_node_size;
		struct io_nodes_info *next;
	} *io_offsets;

	struct all_nodes_info
	{
		char *node_name;
		int node_offset;
		int node_size;
		struct all_nodes_info *next;
	} *all_offsets;

	struct annotated_IR_node *nodes;

	struct annotated_IR_scope *next;
};

struct annotated_IR_scope *get_annotated_scope_by_name(struct annotated_IR_scope *annotations, char *n);


int find_IR_scope_size_by_name(char *t,struct annotated_IR_scope *IR);
void populate_annotated_IR(struct scope_ir *IR);

//calculates required size for a code node
struct code_node_size get_code_node_size(struct datum_ir *IR_node);

//given an annotated IR, generates equivalent "machine" code
void generate_machine_code(struct annotated_IR_scope *ann_IR);
//updates generated machine code to resove expansions
void update_machine_code(struct annotated_IR_scope *ann_IR);


//last function called prior to real code generation and interpretation
//fixes all addresses and offsets in code
void adjust_machine_code(struct annotated_IR_scope *ann_IR);



//Structure to hold a scope (subgraph) and its code representation

struct code_scope
{
	char *scope_name;
	int address;
	int length; //in sizeof(int)
	int *code_ptr;
	int num_nodes; //number of nodes in scope

	struct code_scope *next;
};

int get_code_scope_address(struct code_scope *annotation,char *n); 


void print_machine_code(struct code_scope *p);
void print_adjusted_machine_code(struct code_scope *p);

struct code_scope *get_code_scope(char *n);



//operation definitions

#define code_expansion 	0
#define code_input		1
//#define code_output		2 //output isn't really an operation, it's a destination
#define code_plus 		3
#define code_times		4
#define	code_is_equal	5
#define	code_is_less	6
#define code_is_greater	7
#define	code_if			8 
#define code_else		9 
#define code_minus		10
#define code_merge		11
#define code_identity	12
#define code_end        13

#define code_output		0xFFFFFFFF	//convenient to have it set to a special value that can be tested at runtime

//returns equivalent code operation
int get_code_operation(char *operation);






#endif

