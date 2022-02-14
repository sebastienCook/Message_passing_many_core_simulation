pthread_mutex_t sb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t size_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readiness_mutex = PTHREAD_MUTEX_INITIALIZER;


static pthread_barrier_t bar;



int nodes_to_evaluate = 0;


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
void *interpret(void *tcb_void);
void *interpret_constructed(void *tcb_void);
void *interpret_ready(void *tcb_void);
void *propagate_death(int *sp);

//points to the bottom of the current stack. Decremented when we expand subgraphs (incremented by GC)
int *sb;
//points to top address of our stack - remains unchanged throughout program execution
int *st;
//pointer to current node on the stack
int *sp;



int num_threads;

void *propagate_death(int *sp)
{
	int dest_number;

	if((*(sp+4)) == code_expansion)
	{
		
		int number_of_inputs = *(sp + 5);
		volatile int *n_out_addr = (int *)(sp + 6);
		
		n_out_addr += (long unsigned int)(number_of_inputs * 2);

		int number_of_outputs = *n_out_addr;

		
		volatile int *output_ptr = (int *)(sp + 8 +(*(sp + 5)*2));
		while(number_of_outputs > 0)
		{
			if((int)output_ptr != code_output)
			{

				volatile int *token_ptr = (int *)((long unsigned)st - (long unsigned)(*output_ptr));

				while(*token_ptr != 0x7FFFFFFF)
				{
					token_ptr--;
				
				}
				
				
				if(*(token_ptr + 1) == DEAD)
					return NULL;

				if(*(token_ptr + 4) == code_merge)
					return NULL;

				propagate_death(token_ptr);
				*(token_ptr + 1) = DEAD;
				

					
				
			}
			output_ptr += 2;
			number_of_outputs--;
		}
	}
	else
	{	dest_number = *(*(sp+5) + sp + 6);
		
		int dest_offset = 0;
		
		//ptr is first destination address
		volatile int *ptr;
		
		while(dest_number > 0)
		{
			ptr = (int *)((long int)(*(*(sp+5) + sp + 7 + dest_offset)));
			
			//propagate destinations here
		
			if((int)ptr != code_output)
			{
		
				
				volatile int *created_ptr = ((int *)((long unsigned int)((long unsigned int)(st)-(long unsigned int)ptr)));

				
				volatile int *token_ptr = created_ptr;
				while(*token_ptr != 0x7FFFFFFF)
				{
					token_ptr--;
				
				}
				
				if((*(token_ptr + 1)) == DEAD)
					return NULL;

				if((*(token_ptr + 4)) == code_merge)
					return NULL;

				propagate_death(token_ptr);
				*(token_ptr + 1) = DEAD;

				
			}
			else
			{	
			}

			dest_offset++;
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
}




//Just copies a subgraph to the stack
//updating target addresses in destination to match absolute addresses on the stack
//(prior, they were relative to the beginning of the subgraph)
void push_subgraph(int addr,int size, int num_nodes)
{

	int is_expansion = 0;
	int num_args;
	int num_dest;

	int *colour_ptr = colouring;
	int colour_i = 0; 
	
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
			if(*(code_ptr + 1) == 0)	
			{
				tcb[colouring[colour_i]].nodes_to_evaluate++;
				nodes_to_evaluate++;
			}
			colour_i++;
		}
		
		code_ptr++;
		offset++;
	}
}

//Most of the heavy lifting will be done here
//we'll need to have a different "push" that resolves expansions
//for now let's ignore those

//Everything here is a re-phrasing of the code in "hr_interpreter.c"
//when we use destinations, these are relative to ST
//i.e., effective address is st-dest
void *interpret(void *tcb_void)
{
	struct tcb *tcb = (struct tcb *)tcb_void;
	//while(stack_size > 0)

	pthread_barrier_wait(&bar);
	
	int stack_empty = 0;
	while(1)
	{
	
		//if(node_not_dead(sp))
		if(*((tcb->sp)+1) != DEAD)
		{
			stack_empty = 0;
			tcb->nodes_visited++;
			
			if(*((tcb->sp)+2) != NAV)
			{
				
				//evaluate_constructed(sp, stack);
				interpret_constructed(tcb);
				tcb->nodes_evaluated++;
				//kill_node(sp);
				*((tcb->sp)+1) = DEAD;
				tcb->nodes_to_evaluate--;
				
				
				nodes_to_evaluate--;
				
			}
			else
			{
				
				
				if(*((tcb->sp) + 1) == READY)
				{
					
					//evaluate_ready(sp, &stack, IR, &stack_size);
					interpret_ready(tcb);
					tcb->nodes_evaluated++;
					//if(strcmp(sp->operation,"expansion")!=0)	
					if(*((tcb->sp)+4) != code_expansion)
					{
						//evaluate_constructed(sp, stack);
						interpret_constructed(tcb);
					}
					//kill_node(sp);
					*((tcb->sp)+1) = DEAD;
					tcb->nodes_to_evaluate--;
					
					
				}
			}
		}
		else
		{
			
			if(tcb->sp == tcb->sb)
			{
				stack_empty = 1;
			}
			volatile int *end_checker = ((int *)((long unsigned int)((long unsigned int)(tcb->sp)+(long unsigned int)(*((tcb->sp)+3)))));
			if(end_checker >= (st))
			{
				if(stack_empty)
					goto end_thread;
			}
		}
		//if(sp->next != (struct datum_ir *)0)
		int *end_checker = ((int *)((long unsigned int)((long unsigned int)(tcb->sp)+(long unsigned int)(*((tcb->sp)+3)))));
		//if((sp + (*(sp+3))) < st)
		
		if(end_checker < (st))
		{
			//sp = sp->next;
			
			tcb->sp = end_checker;
		}
		//else
		else
		{
			//sp = stack;
			
			tcb->sp = tcb->sb;
			//GC
			int *old_tcb_sb = tcb->sb;

			//GC all our top of stack dead nodes
			while(*(tcb->sp+1) == DEAD)
			{
				int *next = ((int *)((long unsigned int)((long unsigned int)tcb->sp+(long unsigned int)(*(tcb->sp+3)))));
				tcb->sp = tcb->sb = next;
				tcb->nodes_GCed++;
				
				if(tcb->sp >= st)
					goto end_thread;
			}
			//we're now pointing (sp and sb) to first live node for current thread
			//which is useless unless we also update global sb


			//find lowest sb
			int *lowest = st;
			int i;
			pthread_mutex_lock( &sb_mutex );
			for(i =0; i< num_threads; i++)
			{
				if(*(sb_tracker[i]) < lowest)
					lowest = *(sb_tracker[i]);
			}

			
			//If we had nodes on top of stack
			if(old_tcb_sb == sb)
			{
				
				sb = lowest;
			}
			//if another thread holds top of stack
			else
			{
				//then we don't need to do anything yet
				//thread that holds top of stack will do it
			}
			pthread_mutex_unlock( &sb_mutex );
			//GC end
		
		}
		//print_code_stack();
	}
	end_thread:
	//printf("Finished interpretation\n");
	return NULL;
}

void print_code_stack()
{
	int *ptr = sb;

	printf("\n\nStack starts at 0x%lx, has ive nodes\n",(long int)sp);
	//printf("Stack is bytes long\n",stack_size_bytes);
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
void *interpret_constructed(void *tcb_void)
{
	struct tcb *tcb = (struct tcb *)tcb_void;
	if(*((tcb->sp)+1) == DEAD)
	{
		return NULL;
	}
	//get number of destinations
	//*(sp + 5) is n_args
	//printf("interpreting\n");
	int dest_number = *(*((tcb->sp)+5) + (tcb->sp) + 6);
	int dest_offset = 0;
	
	//ptr is first destination address
	int *ptr;
	
	while(dest_number > 0)
	{
		
		ptr = (int *)((long int)(*(*((tcb->sp)+5) + tcb->sp + 7 + dest_offset)));
		
		//propagate destinations here
	
		if((int)ptr != code_output)
		{
	
			
			int *created_ptr = ((int *)((long unsigned int)((long unsigned int)(st)-(long unsigned int)ptr)));

			
			int *token_ptr = created_ptr;
			while(*token_ptr != 0x7FFFFFFF)
			{
				token_ptr--;
			
			}
			//TODO: Maybe this one needs to come back
			//pthread_mutex_lock( &readiness_mutex );
			if((*token_ptr + 1) != READY)
			{

				
				*created_ptr = *((tcb->sp) + 2);
		
			
				token_ptr++;
				
				(*token_ptr)--;
					
					
			}
			//pthread_mutex_unlock( &readiness_mutex );

			
		}
		else
		{
			//Must output node value
			printf("\t>> %d\n",*((tcb->sp)+2));
		}

		dest_offset++;
		//ptr++;
		dest_number--;
	}
	return NULL;
}


void *interpret_ready(void *tcb_void)
{
	
	struct tcb *tcb = (struct tcb *)tcb_void;
	//expansion is different
	if(*((tcb->sp)+4) == code_expansion)
	{
		
		

		//get addr of subgraph to expand
		int addr = ((*(*((tcb->sp)+5)*2 + tcb->sp + 7)));


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
		int number_of_inputs = *((tcb->sp) + 5);
		int *n_out_addr = (int *)((tcb->sp) + 6);
		
		n_out_addr += (long unsigned int)(number_of_inputs * 2);

		int number_of_outputs = *n_out_addr;

		

		



		//first, let's push entire code verbatim, keeping track of old stack bottom
		int offset = 0;
		//reserve stack space for new code
		//Race condition here for sb, need protection
		pthread_mutex_lock( &sb_mutex );
   		int *old_sb = sb;
   		int compensation = (int)((long unsigned int)tcb->sb - (long unsigned int)sb);
		tcb->sb = sb - size;
		sb=tcb->sb;
   		pthread_mutex_unlock( &sb_mutex );
		
		while(offset < size)
		{
			
			*((tcb->sb)+offset) = *ip;
			if(*ip == 0x7FFFFFFF)
			{
				
			}	
			ip++;
			offset++;
		}
	
		ip = tcb->sb;
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
						*ip += (int)((long unsigned int)(st)-(long unsigned int)old_sb + 4);
					}
					ip++;
				}
			}
		}

		int *input_ptr = (int *)((tcb->sp) + 6);
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

		int *output_ptr = (int *)((tcb->sp) + 8 +(*((tcb->sp) + 5)*2));
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
 
	
		old_sb--;
		while(*old_sb != 0x7FFFFFFF)
		{
			old_sb--;
		}
		*(old_sb + 3) += compensation;


	}
	//all other operators
	else
	{
		
		//get operation
		int operation = *((tcb->sp) + 4);
		switch(operation)
		{
			case code_input: 		printf("\t<< "); scanf("%d",((tcb->sp) + 2)); break;
			case code_plus: 		*((tcb->sp) + 2) = *((tcb->sp) + 6) + *((tcb->sp) + 7); break;
			case code_times:		*((tcb->sp) + 2) = *((tcb->sp) + 6) * *((tcb->sp) + 7); break;
			case code_is_equal:		(*((tcb->sp) + 6) == *((tcb->sp) + 7)) ? (*((tcb->sp) + 2)=1): (*((tcb->sp) + 2)=0); break;
			case code_is_less:		(*((tcb->sp) + 6) < *((tcb->sp) + 7)) ? (*((tcb->sp) + 2)=1): (*((tcb->sp) + 2)=0); break;
			case code_is_greater:	(*((tcb->sp) + 6) > *((tcb->sp) + 7)) ? (*((tcb->sp) + 2)=1): (*((tcb->sp) + 2)=0); break;
			case code_if:			if((*((tcb->sp) + 6) != 0))
									{ 
										(*((tcb->sp) + 2)=*((tcb->sp) + 7));
									}
									else
									{ 
										propagate_death(tcb->sp);
										*((tcb->sp)+1) = DEAD; 
									} break; 
			case code_else:			if(*((tcb->sp) + 6) == 0)
									{
										(*((tcb->sp) + 2)=*((tcb->sp) + 7));
									}
									else
									{ 
										propagate_death(tcb->sp);
										*((tcb->sp)+1) = DEAD; 
									} break; 
			case code_minus:		*((tcb->sp) + 2) = *((tcb->sp) + 6) - *((tcb->sp) + 7); break;
			//TODO: Fix merge so it has a single argument
			case code_merge:		*((tcb->sp) + 2) = (*((tcb->sp) + 6) | *((tcb->sp) + 7)); break;
			case code_identity:		*((tcb->sp) + 2) = *((tcb->sp) + 6); break;
			case code_end:			exit(0); break;
			default: printf("Error: unknown code found during interpretation\n");
		}
	}
	return NULL;
}