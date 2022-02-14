#include "parser.tab.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "ast.h"
#include "ir_generator.h"
#include "hr_interpreter.h"
#include "code_generator.h"
#include "code_interpreter.h"


extern struct annotated_IR_scope *annotated_IR;
extern struct code_scope *program_code;

struct canon_code_entry *code_table = (struct canon_code_entry *)0;

//Stack grows down

//points to the bottom of the current stack. Decremented when we expand subgraphs (incremented by GC)
int *sb;
//points to top address of our stack - remains unchanged throughout program execution
int *st;
//pointer to current node on the stack
int *sp;
//size of stack in number of live nodes
int stack_size = 0;
int stack_size_bytes = 0;

void startup()
{
	//canonic code tables are given in "program code"

	//initializes stack environment
	sb = (int *)malloc(STACK_SIZE * sizeof(int));
	st = sb + STACK_SIZE - 1;
	sp = st;

	//pushes "main" onto the stack
	int main_addr, main_size;

	struct code_scope *main_finder = program_code;

	while(strcmp(main_finder->scope_name,"main")!=0)
		main_finder = main_finder->next;
	main_addr = main_finder->address;
	main_size = main_finder->length;

	push_subgraph(main_addr,main_size);

	print_code_stack();
	//evaluates
}

void print_code_stack()
{
	int *ptr = sb;

	printf("\n\nStack starts at 0x%lx, has %d live nodes\n",(long int)sp,stack_size);
	printf("Stack is %d bytes long\n",stack_size_bytes);
	printf("Stack top at 0x%lx\n",(long int)st);

	while(ptr <= st)
	{
		printf("0x%lx:\t0x%x\n",(long int)ptr,*ptr);
		ptr++;
	}
}

#define NODE_TOKEN			0
#define NODE_COUNT			1
#define NODE_VALUE 			2
#define NODE_SIZE			3
#define NODE_OPERATION		4
#define NODE_NUMARGS		5
#define NODE_ARGVALUE		6
#define NODE_ARGOFFSET		7
#define NODE_NUMDESTS		8
#define NODE_EXPANSIONCODE	9
#define NODE_DESTADDRESS	10
#define NODE_DESTOFFSET		11


//Just copies a subgraph to the stack
//updating target addresses in destination to match absolute addresses on the stack
//(prior, they were relative to the beginning of the subgraph)
void push_subgraph(int addr,int size)
{
	struct code_scope *finder = program_code;

	int node_field = NODE_TOKEN;
	int is_expansion = 0;
	int num_args;
	int num_dest;

	while(finder->address != addr)
		finder = finder->next;
	//code is in the wrong order...
	//need to potentially refactor later
	sp = sp - size + 1;
	sb = sp;

	int *code_ptr = finder->code_ptr;

	int offset = 0;

	while(offset < (size))
	{
		
		*(sp+offset) = *code_ptr;
		
		//if node token
		if(*code_ptr == 0x7FFFFFFF) 
		{
			//potentially increase number of live nodes
			if(*(code_ptr + 1) == 0)	
				stack_size++;
		}
		
		code_ptr++;
		offset++;
	}
	//stack_size += finder->num_nodes;
	stack_size_bytes += offset;
}

//Most of the heavy lifting will be done here
//we'll need to have a different "push" that resolves expansions
//for now let's ignore those

//Everything here is a re-phrasing of the code in "hr_interpreter.c"
//when we use destinations, these are relative to ST
//i.e., effective address is st-dest
void interpret()
{
	//while(stack_size > 0)
	while(stack_size > 0)
	{
		//if(node_not_dead(sp))
		if(*(sp+1) != DEAD)
		{
			//if(node_has_value(sp))
			if(*(sp+2) != NAV)
			{
				/*char c;
				printf("Node has value\n");
				scanf("%c",&c);
				printf("%c",c);*/
				//evaluate_constructed(sp, stack);
				interpret_constructed();
				//kill_node(sp);
				*(sp+1) = DEAD;
				//if by any chance we're on top of stack (sb), change it after killing node
				//(after we resolve expansions)
				//if(sp == sb)
				//	sb =  sp + *(sp+3);
				//stack_size--;
				stack_size--;
			}
			else
			{
				/*char c;
				printf("Node has value\n");
				scanf("%c",&c);
				printf("%c",c);*/
				//if(node_ready(sp)) // if(sp->created_count == READY)
				if(*(sp + 1) == READY)
				{
					/*char c;
					printf("Node has value else ready\n");
					scanf("%c",&c);
					printf("%c",c);*/
					//evaluate_ready(sp, &stack, IR, &stack_size);
					interpret_ready();
					//if(strcmp(sp->operation,"expansion")!=0)	
					if(*(sp+4) != code_expansion)
					{
						//evaluate_constructed(sp, stack);
						interpret_constructed();
					}
					//kill_node(sp);
					*(sp+1) = DEAD;
					//if by any chance we're on top of stack (sb), change it after killing node
					//(after we resolve expansions)
					//if(sp == sb)
					//	sb =  sp + *(sp+3);
					//stack_size--;
					stack_size--;
				}
			}
		}
		else
		{
			
			//this condition should never happen, 
			//but it's useful for debugging

			//if we have a single node on the stack, and it's dead, exit
			/*if(stack_size == 1)
			{
				if(*(sp+1) == DEAD)
				{
					printf("single dead node; exiting\n");
					return;
				}
			}*/
		}
		//if(sp->next != (struct datum_ir *)0)
		int *end_checker = ((int *)((long unsigned int)((long unsigned int)sp+(long unsigned int)(*(sp+3)))));
		//if((sp + (*(sp+3))) < st)
		/*printf("Calculated end checker %lx\n",end_checker);
		char c;
				printf("checker\n");
				scanf("%c",&c);
				printf("%c",c);*/
		if(end_checker < st)
		{
			//sp = sp->next;
			//printf("sp next\n");
			sp = end_checker;
		}
		//else
		else
		{
			//sp = stack;
			//printf("sp bottom\n");
			sp = sb;
		}
		//print_code_stack();
		//char c;
		//scanf("%c",&c);
		//printf("%c",c+1);
	}
	printf("Finished interpretation\n");
}

//This is actually much simpler here than in the human-readable IR form
//Since target addresses are computed, we don't need to worry about destination
//being an expansion or not
void interpret_constructed()
{
	if(*(sp+1) == DEAD)
	{
		printf("DEAD\n");
		return;
	}
	//get number of destinations
	//*(sp + 5) is n_args
	//printf("interpreting\n");
	int dest_number = *(*(sp+5) + sp + 6);
	int dest_offset = 0;
	
	//ptr is first destination address
	int *ptr;
	
	while(dest_number > 0)
	{
		
		ptr = (int *)((long int)(*(*(sp+5) + sp + 7 + dest_offset)));
		
		//propagate destinations here
	
		if((int)ptr != code_output)
		{
	
			
			int *created_ptr = ((int *)((long unsigned int)((long unsigned int)st-(long unsigned int)ptr)));


			int *token_ptr = created_ptr;
			while(*token_ptr != 0x7FFFFFFF)
			{
				token_ptr--;
			
			}
			if((*token_ptr + 1) != READY)
			{

				//printf("Updating %lx\n",created_ptr);
				*created_ptr = *(sp + 2);
		
				//break;
				//update target created count
				
				//go down the stack until we find node token
				
				//Don't re-update an already ready node (can happen in merge)
				
					//decrement created count
					//int *readiness_ptr = (created_ptr + 1);
					token_ptr++;
					(*token_ptr)--;

					if((*token_ptr) == 0)
						stack_size++;
					//(*(created_ptr + 1))--;
				//*readiness_ptr--;
			}
		}
		else
		{
			//Must output node value
			printf("\t>> %d\n",*(sp+2));
		}
		dest_offset++;
		//ptr++;
		dest_number--;
	}
}

void propagate_death(int *root);

void interpret_ready()
{
	
	//printf("ready int \n");
	//expansion is different
	if(*(sp+4) == code_expansion)
	{
		int *old_sb = sb;
		struct code_scope *finder = program_code;

		//get addr of subgraph to expand
		int addr = ((*(*(sp+5)*2 + sp + 7)));

		while(finder->address != addr)
			finder = finder->next;


		int *ip = finder->code_ptr;
		int size = finder->length;
		//printf("got target size %d\n",size);


		//expansion info
		int number_of_inputs = *(sp + 5);
		int *n_out_addr = (int *)(sp + 6);
	//	printf("Out_addr = %lx\n",n_out_addr);
		n_out_addr += (long unsigned int)(number_of_inputs * 2);
	//	printf("Out_addr = %lx\n",n_out_addr);
		int number_of_outputs = *n_out_addr;

	//	printf("Got %d and %d from 0x%lx and 0x%lx\n",number_of_inputs,number_of_outputs,(sp + 5),((long unsigned int)sp + (long unsigned int)6 + (long unsigned int)(number_of_inputs * 2)));

		/*char c;
				printf("Node has value\n");
				scanf("%c",&c);
				printf("%c",c);*/

		//need to push subgraph onto the stack

		//doing expansion node replacement thingys

		//and chanding all destination addresses to compensate for stack position

		//For every expansion ARG
			//Find arg address in expanded code
			//Replace value with ARG value

		//For every expansion DEST
			//Find dest address in expanded code
			//Replace target with address of extant node 



		//first, let's push entire code verbatim, keeping track of old stack bottom
		int offset = 0;
		//reserve stack space for new code
		sb = sb - size;
		//printf("old sb 0x%lx, new sb 0x%lx\n",old_sb,sb);
		while(offset < size)
		{
		//	printf("using addr 0x%lx\n",(sb+offset));
			*(sb+offset) = *ip;

			if(*ip == 0x7FFFFFFF)
			{
				if(*(ip + 1) == 0)	
					stack_size++;
			}	
			ip++;
			offset++;
		}
		//stack_size += finder->num_nodes;
		stack_size_bytes += offset;
		
		//the easiest way to do this might be to first update all addresses, regardless of later being overriden by expansion
			//so we don't have to keep track of which are expansion-mapped or not
		//then just replace those based on expansion mappings

			//Try: update all destinations
		ip = sb;
		while(ip != old_sb)
		{
			int is_expansion = (*(ip+4) == code_expansion) ? 1: 0;

			if(is_expansion)
			{
				//move to number of destinations
				ip += 8 + (*(ip + 5) * 2);
				//printf("st is %lx, old_sb is %lx\n",st,old_sb);
				//printf("long sum is %lx, int sum is %x\n",(st-old_sb),(int)(st-old_sb));
				///TRY CHANGE HERE
				//*ip += (int)(st-old_sb);
				*ip += (int)((long unsigned int)st-(long unsigned int)old_sb + 4);
				ip++;
				ip++;
			}
			else
			{
				//move to number of destinations
				ip += 7 + *(ip + 5);
				while(*ip != 0x7FFFFFFF)
				{
					if(*ip != code_output)	
					{
						//printf("st is %lx, old_sb is %lx\n",st,old_sb);
						//printf("long sum is %lx, int sum is %x\n",(st-old_sb),(int)((long unsigned int)st-(long unsigned int)old_sb));
						*ip += (int)((long unsigned int)st-(long unsigned int)old_sb + 4);
					}
					ip++;
				}
			}
		}

		int *input_ptr = (int *)(sp + 6);
		//input ptr is looking at first argument
		
		while(number_of_inputs > 0)
		{
			//printf("Input arg %x\n",*(sp + 5 + 	(number_of_inputs*2)));

			//for current input
			//find it in newly created code
			int *node_to_replace = (int *)((unsigned long int)old_sb - (unsigned long int)(*(input_ptr + 1)) - 4);
			//printf("NODE to replace is %lx\n",node_to_replace);

			//update its value and readiness
				//readiness 
			*(node_to_replace + 1) = READY;
			//stack_size++;
			*(node_to_replace + 2) = *input_ptr;
			*(node_to_replace + 4) = code_identity;

			input_ptr += 2;

			number_of_inputs--;
		}

		int *output_ptr = (int *)(sp + 8 +(*(sp + 5)*2));
		//output ptr is looking at first destination

		while(number_of_outputs > 0)
		{
			printf("Output arg %x\n",*(sp + 5 + 	(number_of_inputs*2)));

			//for current output
			//find it in newly created code
			int *node_to_replace = (int *)((long unsigned int)old_sb - (long unsigned int)(*(output_ptr + 1)) - 4);
			//printf("Output replace at 0x%lx with 0x%lx\n",node_to_replace,output_ptr);
			//update its destination 

			*(node_to_replace + 7 + *(node_to_replace + 5)) = *output_ptr;


			number_of_outputs--;
		}
 
		//then, go over every node
			//if it's not expansion related (IO), just update destinations relative to stack size (in bytes)
			//if it is expansion related
				//if it's an input, replace ...
				//if it's an output, ...

		printf("\n\nEXPANSION\n\n");
		print_code_stack();
		char c;
		scanf("%c",&c);
		printf("%c",c+1);

	}
	//all other operators
	else
	{
		//TRY
		//When either "if" or "else" fail, also decrement number of live nodes
		//to account for their destination

		//printf("not expansion \n");
		//get operation
		int operation = *(sp + 4);
		switch(operation)
		{
			case code_input: 		printf("\t<< "); scanf("%d",(sp + 2)); break;
			case code_plus: 		*(sp + 2) = *(sp + 6) + *(sp + 7); printf("Did %d + %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2)); break;
			case code_times:		*(sp + 2) = *(sp + 6) * *(sp + 7); printf("Did %d * %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2));break;
			case code_is_equal:		(*(sp + 6) == *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); printf("Did %d == %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2)); break;
			case code_is_less:		(*(sp + 6) < *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); break;
			case code_is_greater:	(*(sp + 6) > *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); break;
			case code_if:			if((*(sp + 6) != 0))
									{ 
										(*(sp + 2)=*(sp + 7));
										printf("Did if %d, = %d (got %d)\n",*(sp + 6),*(sp + 7),*(sp+2));
									}
									else
									{ 
										*(sp+1) = DEAD;
										printf("Did if failed\n"); 
										//propagate_death(sp);
									} break; 
			case code_else:			if(*(sp + 6) == 0)
									{
										(*(sp + 2)=*(sp + 7));
										printf("Did else %d, = %d (got %d)\n",*(sp + 6),*(sp + 7),*(sp+2));
									}
									else
									{ 
										*(sp+1) = DEAD; 
										printf("Did else failed\n");
										//propagate_death(sp);
									} break; 
			case code_minus:		*(sp + 2) = *(sp + 6) - *(sp + 7); printf("Did %d - %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2));break;
			//TODO: Fix merge so it has a single argument
			//case code_merge:		(*(sp + 6) != 0) ? (*(sp + 2)=*(sp + 6)): (*(sp + 2)=*(sp + 7)); printf("Did %d merge %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2));break;
			case code_merge:		*(sp + 2) = (*(sp + 6) | *(sp + 7)); printf("Did %d merge %d = %d\n",*(sp + 6),*(sp + 7),*(sp+2));break;
			case code_identity:		*(sp + 2) = *(sp + 6); break;
			default: printf("Error: unknown code found during interpretation\n");
		}
	}
}

void propagate_death(int *root)
{
	//called on failed IF or ELSE
	//follows destinations until a MERGE, killing as it goes
	while(*root != 0x7FFFFFFF)
	{
		root--;
	
	}
	if((*(root+1) == DEAD))
		return;

	if((*(root+4) == code_merge))
		return;

	
	//printf("KILLING %lx\n",root);
			char c;
			scanf("%c",&c);
			printf("%c\n",c+1);


	int is_expansion = (*(root+4) == code_expansion) ? 1: 0;

	int dest_number = *(*(root+5) + root + 6);
	int dest_offset = 0;
	

	


	
	//ptr is first destination address
	int *ptr;

	if(is_expansion)
		dest_offset+=2;
	
	while(dest_number > 0)
	{
		
		ptr = (int *)((long int)(*(*(root+5) + root + 7 + dest_offset)));
		
		//propagate destinations here
	
		if((int)ptr != code_output)
		{

	
			
			int *created_ptr = ((int *)((long unsigned int)((long unsigned int)st-(long unsigned int)ptr)));

			
			*created_ptr = *(root + 2);
	
			//break;
			//update target created count
			
			//go down the stack until we find node token
			while(*created_ptr != 0x7FFFFFFF)
			{
				created_ptr--;
			
			}
			//decrement created count
			//int *readiness_ptr = (created_ptr + 1);
			propagate_death(created_ptr);

			created_ptr++;
			(*created_ptr) = DEAD;
			//(*(created_ptr + 1))--;
			created_ptr--;
			stack_size--;
			
			
		}
		dest_offset++;
		if(is_expansion)
			dest_offset++;
		//ptr++;
		dest_number--;
	}
	*(root + 1) = DEAD;
}