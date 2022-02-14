#include "parser.tab.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include "ir_generator.h"


extern char *op_strings[];

/*
	collapsing stack

@test
	a:	0 		//missing arguments (potentially replaced after expansion)
		INPUT	//operation for construction
		()		//value
		@z_arg0 //destination
	y: 	0		//missing arguments
		NONE
		1		//value
		@z_arg1	//destination
	z:	2		//missing argument
		arg 0	//argument 0
		arg 1	//argument 1
		PLUS
		result  //value
		OUTPUT	////destination (replaced after expansion)


	For expansions:

	//expansions have no values
	//for inputs, args have to be tuples: mapped node name, value
	//same for outputs, except destination instead of value

		//for human readable IR, at least, node names:
		//for executable one, just node offset in subgraph code
		//so we don't have strings lying around

	test:	1			//readiness
			expansion 	//operation
			a	//arg 0 node
				//arg 0 value
			z	//arg 1 node
				//arg 1 destination

*/

struct scope_ir *IR = (struct scope_ir *)0;

//Receives a full program AST as argument
//for each subgraph in the AST, creates an IR basic block that contains IR code for that subgraph
int generate_ir(struct ast_node *ast)
{
	int errors = 0;
	//should be called with root node of type PROGRAM
	if(ast->type != PROGRAM)
	{
		return 1;
	}
	//down to scopes
	ast = ast->down;
	while(ast != (struct ast_node *)0)
	{
		errors += generate_scope_ir(ast);
		ast = ast->side;
	}
	return errors;
}

int generate_scope_ir(struct ast_node *ast)
{
	int errors = 0;

	struct scope_ir *tmp = IR;

	//allocate new subgraph IR
	IR = (struct scope_ir *)malloc(sizeof(struct scope_ir));
	IR->next = tmp;

	//create name
	IR->name = (char *)malloc(strlen(ast->name)+1);
	strcpy(IR->name,ast->name);

	IR->nodes = (struct datum_ir *)0;

	struct ast_node *ast_nodes = ast->down;	

	//create datum IRs for all nodes in scope
	/*
		create IR for every datum and const (all nodes)
			consts are created with readiness 0
			all others are 2 by default
	*/
	struct datum_ir *current_datum;

	while(ast_nodes != (struct ast_node *)0)
	{
		if((ast_nodes->type == DATUM) || (ast_nodes->type == CONSTANT))
		{
			current_datum = (struct datum_ir *)malloc(sizeof(struct datum_ir));

			current_datum->name = (char *)malloc(strlen(ast_nodes->name)+1);
			strcpy(current_datum->name,ast_nodes->name);

			if(ast_nodes->type == DATUM)
			{
				current_datum->created_count = 1;
				current_datum->value = NAV;
			}
			else
			{
				//if constant
				current_datum->created_count = READY;
				current_datum->value = ast_nodes->value;
			}

			current_datum->args = (struct argument_node *)0;
			current_datum->operation = (char *)malloc(strlen("identity")+1);
			strcpy(current_datum->operation,"identity");

			current_datum->destination = (struct destination_node *)0;

			current_datum->next = IR->nodes;
			IR->nodes = current_datum;
		}
		ast_nodes = ast_nodes->side;
	}
	ast_nodes = ast->down;
	/*
		add all INPUT and OUTPUT information (in readiness and destination)
	*/
	while(ast_nodes != (struct ast_node *)0)
	{
		if(ast_nodes->type == INPUT)
		{
			//find IR node of corresponding datum
			struct datum_ir *found_datum= find_IR_node_by_name(ast_nodes->name,IR->nodes);
			found_datum->created_count = READY;
			if(found_datum->operation != (char *)0)
			{
				free(found_datum->operation);
			} 
			found_datum->operation = (char *)malloc(strlen("input")+1);
			strcpy(found_datum->operation,"input");
		}
		if(ast_nodes->type == OUTPUT)
		{
			//find IR node of corresponding datum
			struct datum_ir *found_datum= find_IR_node_by_name(ast_nodes->name,IR->nodes);

			struct destination_node *tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
			tmp_destination->next = found_datum->destination;
			found_datum->destination = tmp_destination;
			found_datum->destination->destination = (char *)malloc(strlen("output")+1);
			strcpy(found_datum->destination->destination,"output");
		}
		ast_nodes = ast_nodes->side;
	}
	ast_nodes = ast->down;
	/*
		for every operator, fill in destination in nodes
			and operation in destination
	*/
	while(ast_nodes != (struct ast_node *)0)
	{
		if(ast_nodes->type == OPERATOR)
		{

			//used arg 1 and 2 in AST, 0 and 1 in IR.... to refactor later
			char *dest = ast_nodes->op_exp.op_args.dest;
			char *arg1 = ast_nodes->op_exp.op_args.arg1;
			char *arg2 = ast_nodes->op_exp.op_args.arg2;

			//fill in operation in corresponding destination
			struct datum_ir *found_datum= find_IR_node_by_name(dest,IR->nodes);
			if(found_datum->operation != (char *)0)
			{
				free(found_datum->operation);
			}
			found_datum->operation = (char *)malloc(strlen(op_strings[ast_nodes->op_exp.op_args.op])+1);
			strcpy(found_datum->operation,op_strings[ast_nodes->op_exp.op_args.op]);

			//Exception to this is "merge", where readiness should be 1
			if(ast_nodes->op_exp.op_args.op == MERGE)
				found_datum->created_count = 1;
			else
				found_datum->created_count = 2;

			//fill in destination in first operand
			found_datum= find_IR_node_by_name(arg1,IR->nodes);

			struct destination_node *tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
			tmp_destination->next = found_datum->destination;
			found_datum->destination = tmp_destination;

			found_datum->destination->destination = (char *)malloc(strlen(dest)+1+5);
			strcpy(found_datum->destination->destination,"arg0_");
			strcat(found_datum->destination->destination,dest);

			//fill in destination in second operand
			found_datum= find_IR_node_by_name(arg2,IR->nodes);

			tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
			tmp_destination->next = found_datum->destination;
			found_datum->destination = tmp_destination;

			found_datum->destination->destination = (char *)malloc(strlen(dest)+1+5);
			strcpy(found_datum->destination->destination,"arg1_");
			strcat(found_datum->destination->destination,dest);


		}
		ast_nodes = ast_nodes->side;
	}
	ast_nodes = ast->down;

	//for every expansion:
	//create expansion IR
	while(ast_nodes != (struct ast_node *)0)
	{
		if(ast_nodes->type == EXPANSION)
		{
			current_datum = (struct datum_ir *)malloc(sizeof(struct datum_ir));

			current_datum->name = (char *)malloc(strlen(ast_nodes->name)+1);
			strcpy(current_datum->name,ast_nodes->name);

			//need to count number of map_ins to get dependency number
			current_datum->created_count = 0;
			struct ast_node *mapin_counter = ast_nodes->down;

			current_datum->destination = (struct destination_node *)0;

			current_datum->args = (struct argument_node *)0;

			int arg_ctr = 0;

			while(mapin_counter != (struct ast_node *)0)
			{
				if(mapin_counter->type == MAPIN)
				{
					current_datum->created_count++;

					struct argument_node *args_tmp = (struct argument_node *)malloc(sizeof(struct argument_node));
					args_tmp->next = current_datum->args;
					current_datum->args = args_tmp;
					current_datum->args->inner_name = (char *)malloc(strlen(mapin_counter->op_exp.exp_args.inner)+1);
					strcpy(current_datum->args->inner_name,mapin_counter->op_exp.exp_args.inner);
					current_datum->args->arg_value = NAV;

					//find outer node (in current scope), set destination here
					char *dest = mapin_counter->op_exp.exp_args.outer;

					struct datum_ir *found_datum= find_IR_node_by_name(dest,IR->nodes);

					struct destination_node *tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
					tmp_destination->next = found_datum->destination;
					found_datum->destination = tmp_destination;

					found_datum->destination->destination = (char *)malloc(strlen(current_datum->name)+2+5);
					sprintf(found_datum->destination->destination,"arg%d_%s",arg_ctr,current_datum->name);
					arg_ctr++;
					//strcpy(found_datum->destination->destination,"arg0_");
					//strcat(found_datum->destination->destination,current_datum->name);

				}
				//missing outer mappings in destination
				if(mapin_counter->type == MAPOUT)
				{
					char *dest = mapin_counter->op_exp.exp_args.outer;
					char *inner = mapin_counter->op_exp.exp_args.inner;

					printf("dest %s, inner %s\n",dest,inner);

					struct destination_node *tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
					tmp_destination->next = current_datum->destination;
					current_datum->destination = tmp_destination;

					current_datum->destination->destination = (char *)malloc(strlen(dest)+1+5);
					strcpy(current_datum->destination->destination,"arg0_");
					strcat(current_datum->destination->destination,dest);

					current_datum->destination->inner_name = (char *)malloc(strlen(inner)+1);
					strcpy(current_datum->destination->inner_name,inner);
				}
				mapin_counter = mapin_counter->side;
			}

			current_datum->value = NAV;	

			if(current_datum->operation != (char *)0)
			{
				free(current_datum->operation);
			}
			current_datum->operation = (char *)malloc(strlen("expansion")+1);
			strcpy(current_datum->operation,"expansion");

			current_datum->next = IR->nodes;
			IR->nodes = current_datum;
		}
		ast_nodes = ast_nodes->side;
	}
	ast_nodes = ast->down;

	//Re-order IR here?


	//terminator here
	ast_nodes = ast->down;
	/*
		for every operator, fill in destination in nodes
			and operation in destination
	*/
	while(ast_nodes != (struct ast_node *)0)
	{
		if(ast_nodes->type == TERMINATOR)
		{
			//used arg 1 and 2 in AST, 0 and 1 in IR.... to refactor later
			char *dest = ast_nodes->op_exp.op_args.dest;
			

			//fill in operation in corresponding destination
			struct datum_ir *found_datum= find_IR_node_by_name(dest,IR->nodes);
			if(found_datum->operation != (char *)0)
			{
				free(found_datum->operation);
			}
			found_datum->operation = (char *)malloc(strlen("terminate")+1);
			strcpy(found_datum->operation,"terminate");

			int source_cnt = 0;
			struct var_list *list = ast_nodes->end_vars;
			while(list != (struct var_list *)0)
			{
				source_cnt++;
				list = list->next;
			}

			//number of arguments
			found_datum->created_count = source_cnt;

			//fill in destinations
			list = ast_nodes->end_vars;

			while(list != (struct var_list *)0)
			{
				found_datum= find_IR_node_by_name(list->id,IR->nodes);

				struct destination_node *tmp_destination = (struct destination_node *)malloc(sizeof(struct destination_node));
				tmp_destination->next = found_datum->destination;
				found_datum->destination = tmp_destination;

				found_datum->destination->destination = (char *)malloc(strlen(list->id)+1+5);
				strcpy(found_datum->destination->destination,"arg0_");
				strcat(found_datum->destination->destination,dest);
				list = list->next;
			}
		}
		ast_nodes = ast_nodes->side;
	}




	return errors;
}


void print_human_readable_IR(struct scope_ir *IR)
{
	

	printf("\n\nIntermediate Representation:\n");
	while(IR != (struct scope_ir *)0)
	{
		printf("@%s:\n",IR->name);
		struct datum_ir *tmp = IR->nodes;
		while(tmp != (struct datum_ir *)0)
		{
			printf("\t%s:",tmp->name);

			if(tmp->created_count == NAV)
				printf("\t()\t#Dependencies\n");
			else
			{
				if(tmp->created_count == DEAD)
					printf("\tDead\n");
				else
					printf("\t%d\t#Dependencies\n",tmp->created_count);
			}

			//if Dead, print no further information
			if(tmp->created_count != DEAD)
			{
				if(tmp->created_count == READY)
				{
					//if it's a constant
					if(tmp->value != NAV)
					{
						printf("\t\t%d\t#Value\n",tmp->value);

						struct destination_node *destination_tmp = tmp->destination;
						if(destination_tmp == (struct destination_node *)0)
							printf("\t\t()\t#Destination\n");
						else
						{
							while(destination_tmp != (struct destination_node *)0)
							{	
								printf("\t\t%s\t#Destination\n",destination_tmp->destination);
								destination_tmp = destination_tmp->next;
							}
						}
					}
					else
					{
						//if it's an input
						if(strcmp(tmp->operation,"input")==0)
						{
							if(tmp->operation == (char *)0)
								printf("\t\t()\t#Operation\n");
							else
								printf("\t\t%s\t#Operation\n",tmp->operation);

							if(tmp->value == NAV)
								printf("\t\t()\t#Value\n");
							else
								printf("\t\t%d\t#Value\n",tmp->value);

							struct destination_node *destination_tmp = tmp->destination;
							if(destination_tmp == (struct destination_node *)0)
								printf("\t\t()\t#Destination\n");
							else
							{
								while(destination_tmp != (struct destination_node *)0)
								{	
									printf("\t\t%s\t#Destination\n",destination_tmp->destination);
									destination_tmp = destination_tmp->next;
								}
							}
						}
						else
						{
							//if has just been completed, but not further processed
							goto default_print; 
							//I believe this is the first time I've used a "goto" statement, and it's because
							//I'm lazy to refactor this pretty printer. I can feel Dijkstra's ghost getting ready to haunt me.
							//Will refactor if anything abnormal starts happening around the house.
						}
					}
				}
				//If waiting for completion
				else
				{
				default_print:
					if(tmp->operation == (char *)0)
						printf("\t\t()\t#Operation\n");
					else
						printf("\t\t%s\t#Operation\n",tmp->operation);

					if(tmp->args == (struct argument_node *)0)
						printf("\t\t()\t#Args\n");
					else
					{
						struct argument_node *args_tmp = tmp->args;
						while(args_tmp != (struct argument_node *)0)
						{
							if(strcmp(tmp->operation,"expansion")==0)
							{
								if(args_tmp->inner_name != (char *)0)
									printf("\t\t%s\n",args_tmp->inner_name);
							}
							if(args_tmp->arg_value == NAV)
								printf("\t\t()\t#Args\n");
							else
								printf("\t\t%d\t#Args\n",args_tmp->arg_value);
							args_tmp = args_tmp->next;
						}
					}


					if(tmp->value == NAV)
						printf("\t\t()\t#Value\n");
					else
						printf("\t\t%d\t#Value\n",tmp->value);

					struct destination_node *destination_tmp = tmp->destination;
					if(destination_tmp == (struct destination_node *)0)
						printf("\t\t()\t#Destination\n");
					else
					{
						while(destination_tmp != (struct destination_node *)0)
						{	
							if(tmp->operation != (char *)0)
							{
								if(strcmp(tmp->operation,"expansion")==0)
								{
									if(destination_tmp->inner_name != (char *)0)
										printf("\t\t%s\n",destination_tmp->inner_name);
								}
							}
							printf("\t\t%s\t#Destination\n",destination_tmp->destination);
							destination_tmp = destination_tmp->next;
						}
					}
				}
			}	

			tmp = tmp->next;
		}
		IR = IR->next;
		printf("\n");
		
	}
	printf("\n\n");
}

struct datum_ir *find_IR_node_by_name(char *name, struct datum_ir *IR)
{
	while(IR != (struct datum_ir *)0)
	{
		if(strcmp(IR->name, name)==0)
		{
			return IR;
		}
		IR = IR->next;
	}
	printf("Error: NULL at find_IR_node_by_name\n");
	return (struct datum_ir *)0;
}