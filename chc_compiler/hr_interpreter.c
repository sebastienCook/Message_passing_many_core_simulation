#include "parser.tab.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include "ir_generator.h"
#include "hr_interpreter.h"

//interprets, in human readable form, the human readable IR
//acts as a verbose interpreter for debug purposes


void hr_interpret_IR(struct scope_ir *IR, char *subgraph)
{

	//we'll use this as a test for the validity of our operational semantics

	/*Basic algorithm:


		@RULES:
		The IR passed as an argument is immutable: it's our canonical reference
		for function (subgraph) specification, which we'll need throughout evaluation

		@INITIALIZATION:
		Interpretation begins with an empty stack, to which corresponding "subgraph" IR is copied
		(from then on we just see one stack, even if expansions create new subgraphs)
		IT's convenient to reverse the order when copied, i.e., first datum (shown on top in IR) is pushed first, 
		so it's at the bottom of the stack. This is because the way the IR is generated lends itself to more 
		"constructable" nodes at the bottom, which we want at the top during interpretation for ease of execution.
		This can of course be removed later by re-organizing IR prior to this step.

		@RUNTIME:
		Traverse the stack from top to bottom. Evaluate node

			Node evaluation:
				If node has construction <0, it's dead. Destroy it (destruction rules below) and proceed to next one
				If node has construction >0, not ready to be evaluated; proceed to next one 
				If node has construction 0
					If it does not have a value
						Execute its operation to construct it
					else (has a value)
						do nothing
					Propagate result to destinations
					Update construction in destinations
					Destroy node

				For now, destruction just means marking node as dead. This has memory implications which we'll address later
				Options: 
					when destroying, remove it and update pointers accordingly
					Just mark as dead and run GC periodically

				Most evaluations are trivial (construct node through either input or data processing operation)
				Expansion is the exception:
					Must copy new subgraph to top of stack
					Update inputs and outputs according to mapping rules

	*/
	//stack re-uses the data structure that the IR uses
	//always points to top
	struct datum_ir *stack;
	//sp is the pointer to our current node used to traverse the stack
	struct datum_ir *sp;
	//length of stack (counting only live nodes)
	//used to terminate evaluation
	unsigned int stack_size = 0;

	//initialization

		//find nodes of "subgraph" in IR
		struct datum_ir *current_subgraph_nodes = find_subgraphnodes_inIR(IR,subgraph);

		if(current_subgraph_nodes == (struct datum_ir *)0)
		{
			printf("Error: could not find subgraph \"%s\" to evaluate.\n",subgraph);
			return;
		}

		//populate stack with every node in subgraph (reverse order)
		while(current_subgraph_nodes != (struct datum_ir *)0)
		{
			push(&stack,current_subgraph_nodes);
			//assuming here that IR did not generate any dead nodes
			stack_size++;
			current_subgraph_nodes = current_subgraph_nodes->next;	
		}

		//begin evaluation at top
		sp = stack;

	//runtime
		printf("Beginning \"%s\" evaluation\n",subgraph);

		while(stack_size > 0)
		{
			printf("Stack size %d\n",stack_size);
			print_stack(stack, sp);
			char c=getc(stdin);

			//evaluate pointed by sp
			//only evaluate if node is alive
			if(node_not_dead(sp))
			{
				//if node already has a value, evaluate constructed node
				if(node_has_value(sp))
				{
					evaluate_constructed(sp, stack);
					kill_node(sp);
					stack_size--;
				}
				else
				{
					//node does not have a value; only evaluate if it's ready (operator or expansion)
					if(node_ready(sp))
					{
						//This will construct the operator
						//need to pass pointer to stack and IR in case of expansion
						evaluate_ready(sp, &stack, IR, &stack_size);
						//except in the case of expansion, just evaluate constructed now
						if(strcmp(sp->operation,"expansion")!=0)	
							evaluate_constructed(sp, stack);
						kill_node(sp);
						stack_size--;
					}
				}
			}

			//moving to next element
			if(sp->next != (struct datum_ir *)0)
			{
				sp = sp->next;
			}
			else
			{
				sp = stack;
			}
		}
		printf("Finished program evaluation.\n");
}

int node_not_dead(struct datum_ir *sp)
{
	if(sp->created_count != DEAD)
		return 1;
	else
		return 0;
}

int node_is_dead(struct datum_ir *sp)
{
	if(sp->created_count == DEAD)
		return 1;
	else
		return 0;
}

int node_ready(struct datum_ir *sp)
{
	if(sp->created_count == READY)
		return 1;
	else
		return 0;
}

int node_has_value(struct datum_ir *sp)
{
	if(sp->value != NAV)
		return 1;
	else
		return 0;
}


//Evaluation functions
void kill_node(struct datum_ir *sp)
{
	sp->created_count = DEAD;
}
void evaluate_constructed(struct datum_ir *sp, struct datum_ir *stack)
{
	//evaluates a fully constructed node pointed by sp (has value)

	//the only thing needed is to update destination arguments
	//with the exception of "output"

	//some nodes might have killed themselves, so make sure we don't evaluate further if they have
	if(node_is_dead(sp))
		return;

	//different if destination is "expansion", so have to be careful
	struct destination_node *tmp_dest = sp->destination;
	while(tmp_dest != (struct destination_node *)0)
	{
		//get current destination string
		char *dest = tmp_dest->destination;

		//check for output node (or just this destination)
		if(strcmp(dest,"output")==0)
		{
			printf("Output: %d\n",sp->value);
			tmp_dest = tmp_dest->next;
			continue;
		}

		//crop out arg number and node name
		//basic char to int conversion
		dest += 3;
		int arg_number = (int)dest[0] - 48;
		//crop argx_
		dest += 2;

		//get pointer to target node
		struct datum_ir *target=find_target_node(stack,dest);

		if(strcmp(target->operation,"expansion")==0)
		{
			//need a special case for this, as arguments are already allocated, only need to populate value
			struct argument_node *correct_arg = target->args;
			while(arg_number > 0)
			{
				correct_arg = correct_arg->next;
				arg_number--;
			}
			correct_arg->arg_value = sp->value;
			target->created_count--;
		}
		else
		{
			//alocate space for new target argument
			struct argument_node *new_arg = (struct argument_node *)malloc(sizeof(struct argument_node));
			new_arg->arg_value = sp->value;
			new_arg->inner_name = (char *)0;
			//figure out where to put the new argument
			if(arg_number == 0)
			{
				new_arg->next = target->args;
				target->args = new_arg;
			}
			else
			{
				//This is second argument (1), but check whether first one is there or not

				if(target->args == (struct argument_node *)0)
				{
					//first one not there yet
					new_arg->next = target->args;
					target->args = new_arg;
				}
				else
				{
					(target->args)->next = new_arg;
				}
			}
			target->created_count--;
		}

		tmp_dest = tmp_dest->next;
	}
}

//This function gets a double pointer as we might need to modify original stack in case of expansion
void evaluate_ready(struct datum_ir *sp, struct datum_ir **pointer_to_stack, struct scope_ir *IR, int *stack_size)
{

	//special case for expansion, that needs to push elements onto the stack
	if(strcmp(sp->operation,"expansion")==0)
	{
		//printf("Evaluating expansion\n");
		//find nodes of "subgraph" in IR
		struct datum_ir *current_subgraph_nodes = find_subgraphnodes_inIR(IR,sp->name);
		if(current_subgraph_nodes == (struct datum_ir *)0)
		{
			printf("Error: could not find subgraph \"%s\" to expand.\n",sp->name);
			return;
		}

		//populate stack with every node in subgraph (reverse order)
		while(current_subgraph_nodes != (struct datum_ir *)0)
		{
			push(pointer_to_stack,current_subgraph_nodes);
			//assuming here that IR did not generate any dead nodes
			(*stack_size)++;
			current_subgraph_nodes = current_subgraph_nodes->next;	
		}
		//need to map/modify inputs and outputs now

		//for every named argument, modify relevant node to constructed, with value
		while(sp->args != (struct argument_node *)0)
		{
			//find node in stack
			struct datum_ir *node_finder = *pointer_to_stack;
			while(strcmp(node_finder->name,(sp->args)->inner_name)!=0)
			{
				node_finder = node_finder->next;
				if(node_finder == (struct datum_ir *)0)
				{
					printf("Error: failed to find node \"%s\" after expansion.\n",(sp->args)->inner_name);
				}
			}
			//now pointing to relevant node
			node_finder->value = (sp->args)->arg_value;
			node_finder->created_count = READY;

			sp->args = (sp->args)->next;
		}
		//for every named destination, modify relevant output to new destination
		while(sp->destination != (struct destination_node *)0)
		{
			//find node in stack
			struct datum_ir *node_finder = *pointer_to_stack;
			while(strcmp(node_finder->name,(sp->destination)->inner_name)!=0)
			{
				node_finder = node_finder->next;
				if(node_finder == (struct datum_ir *)0)
				{
					printf("Error: failed to find node \"%s\" after expansion.\n",(sp->destination)->inner_name);
				}
			}
			//now pointing to relevant node
			free((node_finder->destination)->destination);
			(node_finder->destination)->destination = (char *)malloc(strlen((sp->destination)->destination)+1);
			strcpy((node_finder->destination)->destination,(sp->destination)->destination);

			sp->destination = (sp->destination)->next;
		}

		return;
	}
	//for now, other operators:
	else
	{
		
		if(strcmp(sp->operation,"input")==0)
		{
			printf("Input value for \"%s\":\n",sp->name);
			scanf("%d",&(sp->value));
			return;
		}
		if(strcmp(sp->operation,"identity")==0)
		{
			sp->value = (sp->args)->arg_value;
			return;
		}
		if(strcmp(sp->operation,"PLUS")==0)
		{
			sp->value = (sp->args)->arg_value + ((sp->args)->next)->arg_value;
			return;
		}
		if(strcmp(sp->operation,"MINUS")==0)
		{
			sp->value = (sp->args)->arg_value - ((sp->args)->next)->arg_value;
			return;
		}
		if(strcmp(sp->operation,"TIMES")==0)
		{
			sp->value = (sp->args)->arg_value * ((sp->args)->next)->arg_value;
			return;
		}
		if(strcmp(sp->operation,"MERGE")==0)
		{
			sp->value = (sp->args)->arg_value;
			return;
		}
		if(strcmp(sp->operation,"ISEQUAL")==0)
		{
			if((sp->args)->arg_value == ((sp->args)->next)->arg_value)
				sp->value = 1;
			else
				sp->value = 0;
			return;
		}
		if(strcmp(sp->operation,"ISLESS")==0)
		{
			if((sp->args)->arg_value < ((sp->args)->next)->arg_value)
				sp->value = 1;
			else
				sp->value = 0;
			return;
		}
		if(strcmp(sp->operation,"ISGREATER")==0)
		{
			if((sp->args)->arg_value > ((sp->args)->next)->arg_value)
				sp->value = 1;
			else
				sp->value = 0;
			return;
		}
		//IF and ELSE are special cases, might kill themselves
		if(strcmp(sp->operation,"IF")==0)
		{
			if((sp->args)->arg_value != 0)
				sp->value = ((sp->args)->next)->arg_value;
			else
				kill_node(sp);
			return;
		}
		if(strcmp(sp->operation,"ELSE")==0)
		{
			if((sp->args)->arg_value == 0)
				sp->value = ((sp->args)->next)->arg_value;
			else
				kill_node(sp);
			return;
		}
		//need TODO all operators
	}
}


//End evaluation functions


struct datum_ir *find_target_node(struct datum_ir *stack, char *dest)
{
	while(stack != (struct datum_ir *)0)
	{
		if(stack->name != (char *)0)
		{
			if(strcmp(stack->name,dest)==0)
			{
				return stack;
			}
		}	
		stack = stack->next;
	}
	printf("Error: could not find node \"%s\" during evaluation.\n", dest);
	return (struct datum_ir *)0;
}


void push(struct datum_ir **stack,struct datum_ir *node)
{
	//create new stack node
	struct datum_ir *stack_element = (struct datum_ir *)malloc(sizeof(struct datum_ir));


	//deep copy of current node (except "next" pointer)
	if(node->name != (char *)0)
	{
		stack_element->name = (char *)malloc(strlen(node->name)+1);
		strcpy(stack_element->name,node->name);
	}
	else
		stack_element->name = (char *)0;

	stack_element->created_count = node->created_count;
	
	if(node->operation != (char *)0)
	{
		stack_element->operation = (char *)malloc(strlen(node->operation)+1);
		strcpy(stack_element->operation,node->operation);
	}
	else
		stack_element->operation = (char *)0;

	stack_element->args = args_deep_copy(node->args);

	stack_element->value = node->value;

	stack_element->destination = destination_deep_copy(node->destination);


	//update stack to point to top (new element)
	stack_element->next = *stack;
	*stack = stack_element;	
}



struct datum_ir *find_subgraphnodes_inIR(struct scope_ir *IR, char *subgraph)
{
	while(IR != (struct scope_ir *)0)
	{
		if(strcmp(IR->name,subgraph)==0)
		{
			return IR->nodes;
		}
		IR = IR->next;
	}
	return (struct datum_ir *)0;
}


struct argument_node *args_deep_copy(struct argument_node *args)
{
	if(args == (struct argument_node *)0)
	{
		return (struct argument_node *)0;
	}
	struct argument_node *result = (struct argument_node *)malloc(sizeof(struct argument_node));
	//copy values here
	if(args->inner_name != (char *)0)
	{
		result->inner_name = (char *)malloc(strlen(args->inner_name)+1);
		strcpy(result->inner_name,args->inner_name);
	}
	else
		result->inner_name = (char *)0;
	result->arg_value = args->arg_value;

	result->next = args_deep_copy(args->next);

	return result;	
}

struct destination_node *destination_deep_copy(struct destination_node *destination)
{
	if(destination == (struct destination_node *)0)
	{
		return (struct destination_node *)0;
	}
	struct destination_node *result = (struct destination_node *)malloc(sizeof(struct destination_node));
	//copy values here
	if(destination->inner_name != (char *)0)
	{
		result->inner_name = (char *)malloc(strlen(destination->inner_name)+1);
		strcpy(result->inner_name,destination->inner_name);
	}
	else
		result->inner_name = (char *)0;
	if(destination->destination != (char *)0)
	{
		result->destination = (char *)malloc(strlen(destination->destination)+1);
		strcpy(result->destination,destination->destination);
	}
	result->next = destination_deep_copy(destination->next);

	return result;	
}

#define COLOR_RED printf("\033[0;31m");
#define COLOR_GREEN printf("\033[0;32m");
#define COLOR_DEFAULT	printf("\033[0m");

void print_stack(struct datum_ir *stack, struct datum_ir *sp)
{
	while(stack != (struct datum_ir *)0)
	{
		if(stack == sp)
			COLOR_RED
		else
			COLOR_DEFAULT
		//only print if not dead
		if(stack->created_count != DEAD)
		{
			
			printf("%s:",stack->name);

			//if it has a value, only print value and destinations
			if(stack->value != NAV)
			{
				//value
				printf("\tValue\t%d\n",stack->value);
				//destinations
				struct destination_node *dest_tmp = stack->destination;
				while(dest_tmp != (struct destination_node *)0)
				{
					if(dest_tmp->inner_name != (char *)0)
					{
						printf("\tDestination\t(%s)->%s\n",dest_tmp->inner_name,dest_tmp->destination);
					}
					else
					{
						printf("\tDestination\t%s\n",dest_tmp->destination);
					}
					dest_tmp = dest_tmp->next;
				}
			}
			else
			{
				//only print dependencies if not 0
				if(stack->created_count != 0)
				{
					printf("\tDependencies\t%d\n",stack->created_count);
				}
				printf("\tOperation\t%s\n",stack->operation);

				//args
				struct argument_node *args_tmp = stack->args;
				while(args_tmp != (struct argument_node *)0)
				{
					if(args_tmp->inner_name != (char *)0)
					{
						if(args_tmp->arg_value != NAV)
							printf("\tArgument\t(%s)<-%d\n",args_tmp->inner_name,args_tmp->arg_value);
						else
							printf("\tArgument\t(%s)<-\n",args_tmp->inner_name);
					}
					else
					{
						if(args_tmp->arg_value != NAV)
							printf("\tArgument\t%d\n",args_tmp->arg_value);
						else
							printf("\tArgument\t\n");
					}
					args_tmp = args_tmp->next;
				}

				//destinations
				struct destination_node *dest_tmp = stack->destination;
				while(dest_tmp != (struct destination_node *)0)
				{
					if(dest_tmp->inner_name != (char *)0)
					{
						printf("\tDestination\t(%s)->%s\n",dest_tmp->inner_name,dest_tmp->destination);
					}
					else
					{
						printf("\tDestination\t%s\n",dest_tmp->destination);
					}
					dest_tmp = dest_tmp->next;
				}
			}
			if(stack->next != (struct datum_ir *)0)
			{
				COLOR_GREEN
				printf("\t\t->%s\n",(stack->next)->	name);
				COLOR_DEFAULT
			}
		}
		
		stack = stack->next;
	}
}