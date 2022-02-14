

#define STACK_SIZE 0x40000

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


#define NAV			0xFFFFFFFC
//Dead operator: remove it
#define DEAD 		0xFFFFFFFF
//Can process corresponding operation and send result to destinations (all arguments resolved)
#define READY 		0



void startup(int addr, int num_nodes);
void push_subgraph(int addr,int size,int num_nodes);
void print_code_stack();
void interpret();
void interpret_constructed();
void interpret_ready();
void *propagate_death(int *sp);

//points to the bottom of the current stack. Decremented when we expand subgraphs (incremented by GC)
int *sb;
//points to top address of our stack - remains unchanged throughout program execution
int *st;
//pointer to current node on the stack
int *sp;
//size of stack in number of live nodes
int stack_size = 0;
int stack_size_bytes = 0;

void *propagate_death(int *sp)
{
	int dest_number;

	if((*(sp+4)) == code_expansion)
	{
		//printf("killing expansion (%lx)\n",(long unsigned int)sp);
		//print_code_stack();
		
		

		

		int number_of_inputs = *(sp + 5);
		int *n_out_addr = (int *)(sp + 6);
		
		n_out_addr += (long unsigned int)(number_of_inputs * 2);

		int number_of_outputs = *n_out_addr;

		//printf("prop with %d outputs\n",number_of_outputs);



		int *output_ptr = (int *)(sp + 8 +(*(sp + 5)*2));
		while(number_of_outputs > 0)
		{
			//printf("outputs %d, out ptr %lx with *%x\n",number_of_outputs,(long unsigned)output_ptr,*output_ptr);
			if((int)output_ptr != code_output)
			{

				
				//output ptr is looking at first destination

				

					int *token_ptr = (int *)((long unsigned)st - (long unsigned)(*output_ptr));

					while(*token_ptr != 0x7FFFFFFF)
					{
						token_ptr--;
					
					}
					//pthread_mutex_lock( &readiness_mutex );
					
					if(*(token_ptr + 1) == DEAD)
						return NULL;

					if(*(token_ptr + 4) == code_merge)
						return NULL;

					//printf("propagate (%lx)\n",(long unsigned int)token_ptr);

					/*char c;
					scanf("%c",&c);
					printf("%c\n",c+1);
*/

					propagate_death(token_ptr);
					*(token_ptr + 1) = DEAD;
					

					
				
				}
				output_ptr += 2;

					number_of_outputs--;
		}
	}
	else
	{	dest_number = *(*(sp+5) + sp + 6);
		//dest_number = (int)(*(int *)((long unsigned int)(*((sp)+5)) + ((long unsigned int)sp) + 6));
		int dest_offset = 0;
		
		//ptr is first destination address
		int *ptr;
		
		
		//printf("prop not expansion, dest %d\n",dest_number);


		while(dest_number > 0)
		{
			ptr = (int *)((long int)(*(*(sp+5) + sp + 7 + dest_offset)));
			//ptr = (int *)((long int)(*(int *)(*(int *)(((long unsigned int)sp)+5) + (long unsigned int)sp + 7 + dest_offset)));
			
			//propagate destinations here
		
			if((int)ptr != code_output)
			{
		
				
				int *created_ptr = ((int *)((long unsigned int)((long unsigned int)(st)-(long unsigned int)ptr)));

				
				int *token_ptr = created_ptr;
				while(*token_ptr != 0x7FFFFFFF)
				{
					token_ptr--;
				
				}
				//pthread_mutex_lock( &readiness_mutex );
				
				if((*(token_ptr + 1)) == DEAD)
					return NULL;

				if((*(token_ptr + 4)) == code_merge)
					return NULL;

				propagate_death(token_ptr);
				*(token_ptr + 1) = DEAD;

				
			}
			else
			{
				//Must output node value
				
			}

			dest_offset++;
			//ptr++;
			dest_number--;
		}
		return NULL;
	}
}


void startup(int addr, int num_nodes)
{
	//canonic code tables are given in "program code"

	//initializes stack environment
	sb = (int *)malloc(STACK_SIZE * sizeof(int));
	st = sb + STACK_SIZE - 1;
	sp = st;

	//pushes "main" onto the stack
	int main_size;

	int i;
	for(i=0;;i++)
	{
		if(dictionary[i][0] == addr)
			break;
	}
	main_size = dictionary[i][1];

	push_subgraph(addr,main_size,num_nodes);

	//print_code_stack();
	//evaluates
}




//Just copies a subgraph to the stack
//updating target addresses in destination to match absolute addresses on the stack
//(prior, they were relative to the beginning of the subgraph)
void push_subgraph(int addr,int size, int num_nodes)
{

	int is_expansion = 0;
	int num_args;
	int num_dest;

	
	
	sp = sp - size + 1;
	sb = sp;

	int *code_ptr = &(code[code_size - addr - size]);

	int offset = 0;

	while(offset < (size))
	{
		
		*(sp+offset) = *code_ptr;
		if(*code_ptr == 0x7FFFFFFF) 
		{
			//potentially increase number of live nodes
			//if(*(code_ptr + 1) == 0)	
			//	stack_size++;
		}
		
		code_ptr++;
		offset++;
	}
	//stack_size += num_nodes;
	//stack_size_bytes += offset;
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
	int stack_empty = 0;
	while(1)
	{
		//if(node_not_dead(sp))
		if(*(sp+1) != DEAD)
		{
			stack_empty = 0;
			//if(node_has_value(sp))
			if(*(sp+2) != NAV)
			{
				
				//evaluate_constructed(sp, stack);
				interpret_constructed();
				//kill_node(sp);
				*(sp+1) = DEAD;
				//if by any chance we're on top of stack (sb), change it after killing node
				//(after we resolve expansions)
				//if(sp == sb)
				//	sb =  sp + *(sp+3);
				//stack_size--;
				//stack_size--;
			}
			else
			{
				
				//if(node_ready(sp)) // if(sp->created_count == READY)
				if(*(sp + 1) == READY)
				{
					
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
					//stack_size--;
				}
			}
		}
		else
		{
			
			if(sp == sb)
			{
				//print_code_stack();
				stack_empty = 1;
			}
			int *end_checker = ((int *)((long unsigned int)((long unsigned int)(sp)+(long unsigned int)(*((sp)+3)))));
			if(end_checker >= (st))
			{
				//print_code_stack();
				//print_code_stack();
				if(stack_empty)
				{
					
					goto end_thread;
				}
			}
		}
		//if(sp->next != (struct datum_ir *)0)
		int *end_checker = ((int *)((long unsigned int)((long unsigned int)sp+(long unsigned int)(*(sp+3)))));
		//if((sp + (*(sp+3))) < st)
		
		if(end_checker < st)
		{
			//sp = sp->next;
			
			sp = end_checker;
		}
		//else
		else
		{
			//sp = stack;
			
			sp = sb;

			//GC

			while(*(sp+1) == DEAD)
			{
				int *next = ((int *)((long unsigned int)((long unsigned int)sp+(long unsigned int)(*(sp+3)))));
				sp = sb = next;
				if(sp >= st)
					goto end_thread;
			}
			//GC end

		}
		//print_code_stack();
	}
	end_thread:
	return;
}

void print_code_stack()
{
	int *ptr = sb;

	printf("\n\nStack starts at 0x%lx, haslive nodes\n",(long int)sp);
	printf("Stack is %d bytes long\n",stack_size_bytes);
	printf("Stack top at 0x%lx\n",(long int)st);

	while(ptr <= st)
	{
		printf("0x%lx:\t0x%x\n",(long int)ptr,*ptr);
		ptr++;
	}
}

//This is actually much simpler here than in the human-readable IR form
//Since target addresses are computed, we don't need to worry about destination
//being an expansion or not
void interpret_constructed()
{
	if(*(sp+1) == DEAD)
	{
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

					//if((*token_ptr) == 0)
					//	stack_size++;
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


void interpret_ready()
{
	
	
	//expansion is different
	if(*(sp+4) == code_expansion)
	{
		int *old_sb = sb;
		

		//get addr of subgraph to expand
		int addr = ((*(*(sp+5)*2 + sp + 7)));


		int i;
		int size;
		for(i=0;;i++)
		{
			if(dictionary[i][0] == addr)
				break;
		}
		size = dictionary[i][1];
		int *ip = &(code[code_size - addr - size]);
		
		


		//expansion info
		int number_of_inputs = *(sp + 5);
		int *n_out_addr = (int *)(sp + 6);
		
		n_out_addr += (long unsigned int)(number_of_inputs * 2);

		int number_of_outputs = *n_out_addr;

		

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
		
		while(offset < size)
		{
			
			*(sb+offset) = *ip;
			/*if(*ip == 0x7FFFFFFF)
			{
				if(*(ip + 1) == 0)	
					stack_size++;
			}*/	
			ip++;
			offset++;
		}
		//stack_size += dictionary[i][2];
		//stack_size_bytes += offset;
		
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
			

			//for current input
			//find it in newly created code
			int *node_to_replace = (int *)((unsigned long int)old_sb - (unsigned long int)(*(input_ptr + 1)) - 4);
			

			//update its value and readiness
				//readiness 
			*(node_to_replace + 1) = READY;
			*(node_to_replace + 2) = *input_ptr;
			*(node_to_replace + 4) = code_identity;

			input_ptr += 2;

			number_of_inputs--;
		}

		int *output_ptr = (int *)(sp + 8 +(*(sp + 5)*2));
		//output ptr is looking at first destination

		while(number_of_outputs > 0)
		{
			

			//for current output
			//find it in newly created code
			int *node_to_replace = (int *)((long unsigned int)old_sb - (long unsigned int)(*(output_ptr + 1)) - 4);
			
			//update its destination 

			*(node_to_replace + 7 + *(node_to_replace + 5)) = *output_ptr;


			number_of_outputs--;
		}
 
		//then, go over every node
			//if it's not expansion related (IO), just update destinations relative to stack size (in bytes)
			//if it is expansion related
				//if it's an input, replace ...
				//if it's an output, ...



	}
	//all other operators
	else
	{
		
		//get operation
		int operation = *(sp + 4);
		switch(operation)
		{
			case code_input: 		printf("\t<< "); scanf("%d",(sp + 2)); break;
			case code_plus: 		*(sp + 2) = *(sp + 6) + *(sp + 7); break;
			case code_times:		*(sp + 2) = *(sp + 6) * *(sp + 7); break;
			case code_is_equal:		(*(sp + 6) == *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); break;
			case code_is_less:		(*(sp + 6) < *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); break;
			case code_is_greater:	(*(sp + 6) > *(sp + 7)) ? (*(sp + 2)=1): (*(sp + 2)=0); break;
			case code_if:			if((*(sp + 6) != 0))
									{ 
										(*(sp + 2)=*(sp + 7));
									}
									else
									{ 
										
										propagate_death(sp);
										*(sp+1) = DEAD; 
									} break; 
			case code_else:			if(*(sp + 6) == 0)
									{
										(*(sp + 2)=*(sp + 7));
									}
									else
									{ 
										
										propagate_death(sp);
										*(sp+1) = DEAD; 
									} break; 
			case code_minus:		*(sp + 2) = *(sp + 6) - *(sp + 7); break;
			//TODO: Fix merge so it has a single argument
			case code_merge:		*(sp + 2) = (*(sp + 6) | *(sp + 7)); break;
			case code_identity:		*(sp + 2) = *(sp + 6); break;
			case code_end:			exit(0); break;
			default: printf("Error: unknown code found during interpretation\n");
		}
	}
}