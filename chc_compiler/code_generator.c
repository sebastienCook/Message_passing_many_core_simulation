#include "parser.tab.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include "ir_generator.h"
#include "hr_interpreter.h"
#include "code_generator.h"

/*
	Revisiting code layout:

		First evaluation is for node not dead. Thus, first element should be created count
		Second evaluation is for a completed node (has value). Thus, second element is value
		
		Next, we decide what to do based on operation

		0-created count
		1-value		
		2-end_address + 1 //convenient to move quickly to next node
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
				(subgraph address)
			NOT EXPANSION
				(address), or OUTPUT code
			EXPANSION
				() address
				() offset into expansion

*/

struct code_node_size get_code_node_size(struct datum_ir *IR_node)
{
	int node_size = 0; //in sizeof(int)

	int arg_number = 0;
	int arg_size = 0;

	int dest_number = 0;
	int dest_size = 0;

	struct code_node_size result;

	//need to determine if expansion (variable number of arguments)
	if(strcmp(IR_node->operation,"expansion")==0)
	{
		
		struct argument_node *args = IR_node->args;

		while(args != (struct argument_node *)0)
		{
			arg_number++;//expansion always has arg size 2, inner name + value
			args = args->next;
		}
		arg_size = 2;
	}
	else
	{
		//not an expansion
		//if constant or input, 0 args
		if((IR_node->value != NAV) || (strcmp(IR_node->operation,"input")==0))
		{	
			arg_number = 0;
			arg_size = 0;
		}
		else
		{
			//if identity, just one argument
			if((strcmp(IR_node->operation,"identity")==0))
			{
				arg_number = 1;
				arg_size = 1;
			}
			//all other operators, 2 arguments of size 1
			else
			{
				arg_number = 2;
				arg_size = 1;
			}
		}
	}
	struct destination_node *destination = IR_node->destination;

	while(destination != (struct destination_node *)0)
	{
		dest_number++;
		destination = destination->next;
	}
	if(strcmp(IR_node->operation,"expansion")==0)
	{
		dest_size = 2;
	}
	else
	{
		dest_size = 1;
	}

	node_size = (7 + (arg_number * arg_size) + (dest_number * dest_size));

	if(strcmp(IR_node->operation,"expansion")==0)
		node_size++; //add one for expansion subgraph code

	result.arg_number = arg_number;
	result.arg_size = arg_size;
	result.dest_number = dest_number;
	result.dest_size = dest_size;
	result.size = node_size;

	return result;
}


struct annotated_IR_scope *annotated_IR = (struct annotated_IR_scope *)0;

void populate_annotated_IR(struct scope_ir *IR)
{
	while(IR != (struct scope_ir *)0)
	{
		struct annotated_IR_scope *new_annotation;

		new_annotation = (struct annotated_IR_scope *)malloc(sizeof(struct annotated_IR_scope));

		new_annotation->IR_node = IR;
		new_annotation->size = 0;
		new_annotation->number_nodes = 0;

		struct datum_ir *nodes_ptr = IR->nodes;

		while(nodes_ptr != (struct datum_ir *)0)
		{
			struct annotated_IR_node *new_annotated_node;

			new_annotated_node = (struct annotated_IR_node *)malloc(sizeof(struct annotated_IR_node));

			new_annotated_node->size = get_code_node_size(nodes_ptr);
			new_annotated_node->IR_node = nodes_ptr;

			//Populate info for sizes of all nodes
			struct all_nodes_info *new_info_node;
			new_info_node = (struct all_nodes_info *)malloc(sizeof(struct all_nodes_info));
			new_info_node->node_name = nodes_ptr->name;
			new_info_node->node_offset=new_annotation->size;
			new_info_node->node_size = get_code_node_size(nodes_ptr).size;
			
			new_info_node->next = new_annotation->all_offsets;
			new_annotation->all_offsets = new_info_node;

			//This will be very convenient to later on process expansions
			//thus the dedicated structure for IOs
			//io
			if((strcmp(nodes_ptr->operation,"input")==0))
			{
				struct io_nodes_info *new_io;
				new_io = (struct io_nodes_info *)malloc(sizeof(struct io_nodes_info));
				new_io->io_node_name = nodes_ptr->name;
				new_io->io_node_offset=new_annotation->size;
				new_io->io_type = 0;
				new_io->io_node_size = get_code_node_size(nodes_ptr).size;
				
				new_io->next = new_annotation->io_offsets;
				new_annotation->io_offsets = new_io;
			}
			struct destination_node *dest_ptr = nodes_ptr->destination;
			while(dest_ptr != (struct destination_node *)0)
			{
				if((strcmp(dest_ptr->destination,"output")==0))
				{
					struct io_nodes_info *new_io;
					new_io = (struct io_nodes_info *)malloc(sizeof(struct io_nodes_info));
					new_io->io_node_name = nodes_ptr->name;
					new_io->io_node_offset=new_annotation->size;
					new_io->io_type = 1;
					new_io->io_node_size = get_code_node_size(nodes_ptr).size;
					
					new_io->next = new_annotation->io_offsets;
					new_annotation->io_offsets = new_io;
				}
				dest_ptr = dest_ptr->next;
			}

			new_annotation->number_nodes++;
			new_annotation->size += new_annotated_node->size.size;

			new_annotated_node->next = new_annotation->nodes;
			new_annotation->nodes = new_annotated_node;
			nodes_ptr = nodes_ptr->next;
		}

		new_annotation->next = annotated_IR;
		annotated_IR = new_annotation;
		IR=IR->next;
	}
	//NOTE: annotated IR is in reverse order than current IR (both scopes and nodes)
	//This is visible in node offsets, which are mirrored
	struct annotated_IR_scope *new_annotation = annotated_IR;
	while(new_annotation != (struct annotated_IR_scope *)0)
	{
		printf("Scope %s has %d nodes, total size %d\n",new_annotation->IR_node->name,new_annotation->number_nodes,new_annotation->size);



		struct annotated_IR_node *new_annotated_node = new_annotation->nodes;
		while(new_annotated_node != (struct annotated_IR_node *)0)
		{
			printf("\tNode %s %d args (size %d) %d destinations (size %d), total %d\n",(new_annotated_node->IR_node)->name,
				new_annotated_node->size.arg_number,new_annotated_node->size.arg_size,
				new_annotated_node->size.dest_number,new_annotated_node->size.dest_size,
				new_annotated_node->size.size);
			new_annotated_node = new_annotated_node->next;	
		}
		printf("Inputs/Outputs:\n");
		struct io_nodes_info *new_io = new_annotation->io_offsets;

		//let's reverse all node offsets so they match annotated IR order (which is the one we want to generate code in)
		
		while(new_io != (struct io_nodes_info *)0)
		{
			new_io->io_node_offset = new_annotation->size - new_io->io_node_offset - new_io->io_node_size;
			new_io = new_io->next;
		}

		struct all_nodes_info *new_node = new_annotation->all_offsets;

		while(new_node != (struct all_nodes_info *)0)
		{
			new_node->node_offset = new_annotation->size - new_node->node_offset - new_node->node_size;
			new_node = new_node->next;
		}

		new_io = new_annotation->io_offsets;
		while(new_io != (struct io_nodes_info *)0)
		{
			printf("\t%s, offset %d, %s\n",new_io->io_node_name, new_io->io_node_offset, (new_io->io_type == 1) ? "output":"input");
			new_io = new_io->next;
		}
		printf("\nALL NODES\n");
		struct all_nodes_info *debug_ptr = new_annotation->all_offsets;
		while(debug_ptr != (struct all_nodes_info *)0)
		{
			printf("\t%s, offset %d\n",debug_ptr->node_name, debug_ptr->node_offset);
			debug_ptr = debug_ptr->next;
		}
		printf("\n");


		new_annotation = new_annotation->next;
	}
}

struct code_scope *program_code = (struct code_scope *)0;

struct code_scope *get_code_scope(char *n)
{
	struct code_scope *finder = program_code;

	while(finder != (struct code_scope *)0)
	{
		if(strcmp(finder->scope_name,n)==0)
			return finder;
		finder = finder->next;
	}	
}

int get_code_scope_address(struct code_scope *annotation,char *n)
{
	while(strcmp(annotation->scope_name,n)!=0)
		annotation = annotation->next;
	return annotation->address;
}


//returns equivalent code operation
int get_code_operation(char *operation)
{
	if(strcmp(operation,"expansion")==0)
		return code_expansion;
	if(strcmp(operation,"input")==0)
		return code_input;
	if(strcmp(operation,"output")==0)
		return code_output;
	if(strcmp(operation,"PLUS")==0)
		return code_plus;
	if(strcmp(operation,"TIMES")==0)
		return code_times;
	if(strcmp(operation,"ISEQUAL")==0)
		return code_is_equal;
	if(strcmp(operation,"ISLESS")==0)
		return code_is_less;
	if(strcmp(operation,"ISGREATER")==0)
		return code_is_greater;
	if(strcmp(operation,"IF")==0)
		return code_if;
	if(strcmp(operation,"ELSE")==0)
		return code_else;
	if(strcmp(operation,"MINUS")==0)
		return code_minus;
	if(strcmp(operation,"MERGE")==0)
		return code_merge;
	if(strcmp(operation,"identity")==0)
		return code_identity;
	if(strcmp(operation,"END")==0)
		return code_end;
	if(strcmp(operation,"terminate")==0)
		return code_end;
	return 0;
}

//given an annotated IR, generates equivalent "machine" code
void generate_machine_code(struct annotated_IR_scope *ann_IR)
{
	//per scope, generate equivalent code
	//keep a pointer of scope name to code block
	struct annotated_IR_scope *IR_ptr = ann_IR;

	int size_thus_far = 0;

	while(IR_ptr != ((struct annotated_IR_scope *)0))
	{
		//allocate code space, calculate subgraph addresses
		struct code_scope *new_code_block = (struct code_scope *)malloc(sizeof(struct code_scope));
		new_code_block->scope_name = (IR_ptr->IR_node)->name;
		new_code_block->length = IR_ptr->size;
		new_code_block->num_nodes = IR_ptr->number_nodes;
		
		new_code_block->address = size_thus_far;
		size_thus_far += new_code_block->length;
		
		new_code_block->code_ptr = (int *)malloc((IR_ptr->size) * sizeof(int));


		new_code_block->next = program_code;
		program_code = new_code_block;

		IR_ptr = IR_ptr->next;
	}

	//print_machine_code(program_code);

	IR_ptr = ann_IR;
	//generate_machine_code_scope(ann_IR, new_code_block->code_ptr);
	
	
	while(IR_ptr != ((struct annotated_IR_scope *)0))
	{
		struct annotated_IR_node *n_ptr = IR_ptr->nodes;
		struct code_scope *c_ptr = get_code_scope((IR_ptr->IR_node)->name);

		int *ip = c_ptr->code_ptr; //instruction pointer

		int addr_count = 0;

		while(n_ptr != (struct annotated_IR_node *)0)
		{

			

			//special case for expansion (cannot yest be resolved because of address resolution)
			if(strcmp((n_ptr->IR_node)->operation,"expansion")==0)
			{
				ip += n_ptr->size.size;
			}
			else
			{
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
				//-1-node start marker (0x7FFFFFFF)
				*ip = 0x7FFFFFFF;
				ip++;
				//0-created count
				*ip = (n_ptr->IR_node)->created_count;
				ip++;
				//1-value
				*ip = (n_ptr->IR_node)->value;
				ip++;
				//2-node size
				*ip=n_ptr->size.size;
				ip++;
				//3-operation
				*ip = get_code_operation((n_ptr->IR_node)->operation);
				ip++;
				//4-number of arguments
				*ip = n_ptr->size.arg_number; //we know arg_size is 1, since this is not an expansion
				ip++;
				//arguments - reserve space
				ip += n_ptr->size.arg_number; 
				//5-number of destinations
				
				*ip = n_ptr->size.dest_number; //we know dest_size is 1, since this is not an expansion
				ip++;

				struct destination_node *dest_ptr = (n_ptr->IR_node)->destination;

				while(dest_ptr != (struct destination_node *)0)
				{
					//REDO HERE FOR ADDRESS
					int destination_address;

					if(strcmp(dest_ptr->destination,"output")==0)
					{
						destination_address = code_output;
					}
					else
					{
						//find address of target node (relative to beginning of subgraph)
						char *target_name = dest_ptr->destination;

						//printf("\tFINDING TARGET %s\n",target_name);
						//get rid or args
						while(*target_name != '_')
							target_name++;
						target_name++;

						char *arg_number_str = dest_ptr->destination;
						arg_number_str += 3;

						char *arg_number_helper = arg_number_str;
						while(*arg_number_helper != '_')
							arg_number_helper++;
						arg_number_helper = '\0';

						int arg_index = atoi(arg_number_str);

						//printf("\tUSING ARG OFFSET %d\n",arg_index);

						struct datum_ir *target_node = find_IR_node_by_name(target_name, (IR_ptr->IR_node)->nodes);
						struct code_node_size target_size = get_code_node_size(target_node);

						struct all_nodes_info *info_ptr = IR_ptr->all_offsets;
						while(strcmp(info_ptr->node_name, target_name)!=0)
							info_ptr = info_ptr->next;

						int target_offset = info_ptr->node_offset;

						//printf("\tUSING TSARGET NODE OFFSET %d\n",target_offset);

						destination_address = target_offset + 5 + ((arg_index  * target_size.arg_size + 1));

						//destination_address = target_offset + 4 /*number of args*/ + arg_index + (target_size.arg_number * target_size.arg_size)
						//	+ 1 /*number of destinations*/ + (arg_index * target_size.dest_size);
					}



					*ip = destination_address;

					dest_ptr = dest_ptr->next;
					ip++;
				}
			}
			n_ptr = n_ptr->next;
		}
		IR_ptr = IR_ptr->next;
	}


	//At this point we should have all our machine code generated, with proper addresses per subgraph
	//But missing expansions 
	//print_machine_code(program_code);

	update_machine_code(ann_IR);

	print_machine_code(program_code);

	adjust_machine_code(ann_IR);
}


struct annotated_IR_scope *get_annotated_scope_by_name(struct annotated_IR_scope *annotations, char *n)
{
	while(strcmp((annotations->IR_node)->name,n)!=0)
		annotations = annotations->next;
	if(annotations == (struct annotated_IR_scope *)0)
		printf("Error: NULL at get_annotated_scope_by_name\n");
	return annotations;
}

//updates generated machine code to resolve expansions
void update_machine_code(struct annotated_IR_scope *ann_IR)
{
	//per scope, patch all expansions
	//keep a pointer of scope name to code block
	struct annotated_IR_scope *IR_ptr = ann_IR;

	
	
	while(IR_ptr != ((struct annotated_IR_scope *)0))
	{
		struct annotated_IR_node *n_ptr = IR_ptr->nodes;
		struct code_scope *c_ptr = get_code_scope((IR_ptr->IR_node)->name);

		int *ip = c_ptr->code_ptr; //instruction pointer

		int addr_count = 0;

		while(n_ptr != (struct annotated_IR_node *)0)
		{
			//patch expansions 
			if(strcmp((n_ptr->IR_node)->operation,"expansion")==0)
			{
				//-1-node marker
				*ip = 0x7FFFFFFF;
				ip++;
				//0-created count
				*ip = (n_ptr->IR_node)->created_count;
				ip++;
				//1-value
				*ip = (n_ptr->IR_node)->value;
				ip++;
					//2-end_address + 1 //convenient to move quickly to next node
					//addr_count += n_ptr->size.size;
					//*ip = addr_count;
				//2-node size
				*ip =n_ptr->size.size;
				ip++;
				//3-operation
				*ip = get_code_operation((n_ptr->IR_node)->operation);
				ip++;
				//4-number of arguments
				*ip = n_ptr->size.arg_number; //we know arg_size is 2, since this is an expansion
				ip++;

				//ARGUMENTS

				struct argument_node *args_ptr = (n_ptr->IR_node)->args;


				char *real_name = (n_ptr->IR_node)->name;

				while(args_ptr != (struct argument_node *)0)
				{
					
					while(*real_name != ':')
						real_name++;
					real_name++;
					//		() value
					ip++; //reserve space
					//		offset into expansion
					//need to figure out which expansion
					struct annotated_IR_scope *target_scope=get_annotated_scope_by_name(ann_IR, real_name);
					//then get node offset
					struct all_nodes_info *target_node = target_scope->all_offsets;
					while(strcmp(target_node->node_name,args_ptr->inner_name)!=0)
						target_node = target_node->next;

					*ip=target_node->node_offset;
					ip++; 
					args_ptr = args_ptr->next;
				}
				//DESTINATIONS
				//5-number of destinations
				*ip = n_ptr->size.dest_number; //we know arg_size is 2, since this is an expansion
				ip++;
				//		(If expansion, one additional element here: code of what to expand)
				*ip = get_code_scope_address(program_code,real_name); 
				ip++;

				//destinations...
				//	() address
				//	() offset into expansion
				//TODO 
				//	ip += n_ptr->size.dest_number;


				struct destination_node *dest_ptr = (n_ptr->IR_node)->destination;

				while(dest_ptr != (struct destination_node *)0)
				{
					//Addresses are the same as in normal nodes
					int destination_address;

					//find address of target node (relative to beginning of subgraph)
					char *target_name = dest_ptr->destination;


					//get rid or args
					while(*target_name != '_')
						target_name++;
					target_name++;

					char *arg_number_str = dest_ptr->destination;
					arg_number_str += 3;

					char *arg_number_helper = arg_number_str;
					while(*arg_number_helper != '_')
						arg_number_helper++;
					arg_number_helper = '\0';

					int arg_index = atoi(arg_number_str);

					struct datum_ir *target_node = find_IR_node_by_name(target_name, (IR_ptr->IR_node)->nodes);
					struct code_node_size target_size = get_code_node_size(target_node);

					struct all_nodes_info *info_ptr = IR_ptr->all_offsets;
					while(strcmp(info_ptr->node_name, target_name)!=0)
						info_ptr = info_ptr->next;

					int target_offset = info_ptr->node_offset;

					destination_address = target_offset + 5 + ((arg_index  * target_size.arg_size + 1));

					//destination_address = target_offset + 4 /*number of args*/ + (target_size.arg_number * target_size.arg_size)
					//	+ 1 /*number of destinations*/ + (arg_index * target_size.dest_size);
					

					*ip = destination_address;
					ip++;

					//offset within expansion is the same as for argument
					//need to figure out which expansion
					struct annotated_IR_scope *target_scope=get_annotated_scope_by_name(ann_IR, real_name);
					//then get node offset
					struct all_nodes_info *expansion_node = target_scope->all_offsets;
					while(strcmp(expansion_node->node_name,dest_ptr->inner_name)!=0)
						expansion_node = expansion_node->next;

					*ip=expansion_node->node_offset;
					ip++; 


					dest_ptr = dest_ptr->next;
					
				}

			}
			else
			{
				ip += n_ptr->size.size;
			}
			n_ptr = n_ptr->next;
		}
		IR_ptr = IR_ptr->next;
	}


	//At this point we should have all our machine code generated
	//print_machine_code(program_code);
}

int find_IR_scope_size_by_name(char *t, struct annotated_IR_scope *IR)
{
	struct annotated_IR_scope *IR_ptr = IR;

	while(IR_ptr != (struct annotated_IR_scope *)0)
	{
		if(strcmp(((IR_ptr->IR_node)->name),t)==0)
		{
			return IR_ptr->size;
		}
		IR_ptr = IR_ptr->next;
	}
	return 0;
}

//last function called prior to real code generation and interpretation
//fixes all addresses and offsets in code
void adjust_machine_code(struct annotated_IR_scope *ann_IR)
{
	//What we want to do here:
	
	//Make sure code addresses are in bytes (each node element has size 4 (sizeof(int)))
	//Make sure subgraph addresses match descending stack addresses 
	//Make sure destinations are relative to top of stack (assuming 0 per node)
		//We can then use Stack Top relative addressing during interpretation
		//And when expanding, add value of SP so offset is correctly maintained (stack grows down) 
	//Make sure expansion offsets are treated the same way as destinations
	struct annotated_IR_scope *IR_ptr = ann_IR;

	
	
	while(IR_ptr != ((struct annotated_IR_scope *)0))
	{
		struct annotated_IR_node *n_ptr = IR_ptr->nodes;
		struct code_scope *c_ptr = get_code_scope((IR_ptr->IR_node)->name);

		int *ip = c_ptr->code_ptr; //instruction pointer

		int addr_count = 0;

		//in sizeof(int)
		int node_size = c_ptr->length;

		while(n_ptr != (struct annotated_IR_node *)0)
		{
			if(strcmp((n_ptr->IR_node)->operation,"expansion")==0)
			{
				//-1-node marker
				ip++;
				//0-created count
				ip++;
				//1-value
				ip++;
				//2-node size ADJUST IN BYTES
				*ip *= sizeof(int);
				ip++;
				//3-operation
				ip++;
				//4-number of arguments
				ip++;
				struct argument_node *args_ptr = (n_ptr->IR_node)->args;

				while(args_ptr != (struct argument_node *)0)
				{
					//		() value
					ip++; //reserve space
					//		offset into expansion

					//name of target subgraph
					char *target_name = (n_ptr->IR_node)->name;
					while(*target_name != ':')
						target_name++;
					target_name++;

					int target_node_size = find_IR_scope_size_by_name(target_name,ann_IR);

					int current_offset = *ip;	
					//this is currently top(0) + offset (in sizeof(int)) 
					current_offset = target_node_size - current_offset - 1;
					//this gives us offset, in sizeof(int), from bottom
					current_offset *= sizeof(int);
					//this gives us offset in bytes
					*ip = current_offset;


					ip++; 
					args_ptr = args_ptr->next;
				}
				//DESTINATIONS
				//5-number of destinations
				ip++;
				//		(If expansion, one additional element here: code of what to expand) 
				ip++;
				struct destination_node *dest_ptr = (n_ptr->IR_node)->destination;

				while(dest_ptr != (struct destination_node *)0)
				{
					printf("Patching %d (0x%x) and %d (0x%x)\n",*ip,*ip,*(ip+1),*(ip+1));
					//		() value - offset of node in current scope
					char *target_name = (n_ptr->IR_node)->name;
					while(*target_name != ':')
						target_name++;
					target_name++;
					int current_des_addr = *ip;

					current_des_addr = node_size - current_des_addr - 1;
					
					printf("patched (%d) using target with size %d\n",*ip,node_size);
					current_des_addr *= sizeof(int);
					*ip = current_des_addr;
					ip++; //reserve space

					//offset into expansion
					current_des_addr = *ip;	
					//this is currently top(0) + offset (in sizeof(int)) 
					
					int target_node_size = find_IR_scope_size_by_name(target_name,ann_IR);
					current_des_addr = target_node_size - current_des_addr - 1;
					//this gives us offset, in sizeof(int), from bottom
					current_des_addr *= sizeof(int);
					//this gives us offset in bytes
					*ip = current_des_addr;
					ip++;

					dest_ptr = dest_ptr->next;
					
					printf("Patched %d (0x%x) and %d (0x%x)\n",*(ip-2),*(ip-2),*(ip-1),*(ip-1));
				}

			}
			else
			{
				//not expansion
				//-1 node marker
				ip++;
				//0-created count
				ip++;
				//1-value
				ip++;
				//2-node size ADJUST IN BYTES
				*ip *= sizeof(int);
				ip++;
				//3-operation
				ip++;
				//4-number of arguments
				ip++;
				//arguments 
				ip += n_ptr->size.arg_number; 
				//5-number of destinations
				ip++;
				struct destination_node *dest_ptr = (n_ptr->IR_node)->destination;

				while(dest_ptr != (struct destination_node *)0)
				{
					if(strcmp(dest_ptr->destination,"output")==0)
					{
						//do nothing
					}
					else
					{
						int current_des_addr = *ip;	
						//this is currently top(0) + offset (in sizeof(int)) 
						current_des_addr = node_size - current_des_addr - 1;
						//this gives us offset, in sizeof(int), from bottom
						current_des_addr *= sizeof(int);
						//this gives us offset in bytes
						*ip = current_des_addr;
					}
					dest_ptr = dest_ptr->next;
					ip++;
				}
			}

			n_ptr = n_ptr->next;
		}
		IR_ptr = IR_ptr->next;
	}


	print_adjusted_machine_code(program_code);
}



void print_adjusted_machine_code(struct code_scope *p)
{
	printf("\nMACHINE CODE:\n");
	
	while(p != (struct code_scope *)0)
	{
		int length = p->length ;
		
		int *ip = p->code_ptr;
		int ctr = 0;

		printf("Start %s @(%d):\n",p->scope_name,p->address);

		while(length > 0)
		{
			printf("\t-%x:	0x%x\n",length*4,*ip);
			ip++;
			length--;
		}

		printf("End %s:\n",p->scope_name);

		p = p->next;
	}
}

void print_machine_code(struct code_scope *p)
{
	printf("\nMACHINE CODE:\n");
	while(p != (struct code_scope *)0)
	{

		
		int *ip = p->code_ptr;
		int ctr = 0;

		printf("Start %s @(%d):\n",p->scope_name,p->address);

		while((ctr) < (p->length))
		{
			printf("\t%d:	%d\n",ctr,*ip);
			ip++;
			ctr++;
		}

		printf("End %s:\n",p->scope_name);

		p = p->next;
	}
}
























