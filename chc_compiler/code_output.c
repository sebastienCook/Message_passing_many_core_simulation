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
#include "code_output.h"
#include<time.h>


extern struct code_scope *program_code;




void generate_output()
{
	FILE *fp = fopen("chc_output.c","w");
	FILE *source_fp = fopen("startup/startup.c","r");

	/////////////////////////////////////////////
	FILE *code_H_fp = fopen("../many_core.c","r");

  FILE *temp_H_fp = fopen("../temp_H.c","w");

	char buffer[256];
	while(fgets(buffer, 256, code_H_fp) != NULL ) {
	//printf("%s\n", buffer);
    		fprintf(temp_H_fp,"%s",buffer);
		if(strcmp(buffer,"//CODE BEGINE//\n") == 0){
			break;
		}
  }
	/////////////////////////////////////////////

	struct code_scope *main_finder = program_code;

	int main_addr;

	while(strcmp(main_finder->scope_name,"main")!=0)
		main_finder = main_finder->next;
	main_addr = main_finder->address;

	struct code_scope *code_ptr;


	fprintf(fp,"#include<stdio.h>\n #include<stdlib.h>\n#include<time.h>\n");

	fprintf(fp,"int code[] = {");
	fprintf(temp_H_fp,"const int code[] = {");

	int total_code_size = 0;


	//Create canonic program definition

	code_ptr = program_code;




	while(code_ptr != (struct code_scope *)0)
	{
		int length = code_ptr->length ;

		int *ip = code_ptr->code_ptr;
		int ctr = 0;

		fprintf(fp,"//End %s:\n",code_ptr->scope_name);
		fprintf(temp_H_fp,"//End %s:\n",code_ptr->scope_name);

		while(length > 0)
		{
			if((code_ptr->next == (struct code_scope *)0) && (length == 1)){
				fprintf(fp,"0x%x\n",*ip);
				fprintf(temp_H_fp,"0x%x\n",*ip);
			}else{
				fprintf(fp,"0x%x,\n",*ip);
				fprintf(temp_H_fp,"0x%x,\n",*ip);
			}
			ip++;
			length--;
			total_code_size++;
		}
		fprintf(fp,"//Start %s @(%d):\n",code_ptr->scope_name,code_ptr->address);
		fprintf(temp_H_fp,"//Start %s @(%d):\n",code_ptr->scope_name,code_ptr->address);

		code_ptr = code_ptr->next;
	}

	fprintf(fp,"};\n");
	fprintf(temp_H_fp,"};\n");
	fprintf(fp,"int code_size = %d;\n",total_code_size);
	fprintf(temp_H_fp,"int code_size = %d;\n",total_code_size);

	fprintf(temp_H_fp,"int main_addr = %d;\n",main_addr);
	fprintf(temp_H_fp,"int main_num_nodes = %d;\n",main_finder->num_nodes);


	//create subgraph dictionary
	fprintf(fp,"int dictionary[][3] = {");
	fprintf(temp_H_fp,"int dictionary[][3] = {");

	code_ptr = program_code;
	int count = 0;
	while(code_ptr != (struct code_scope *)0)
	{
		if((code_ptr->next == (struct code_scope *)0)){
			fprintf(fp,"{%d,%d,%d}\n",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
			fprintf(temp_H_fp,"{%d,%d,%d}\n",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
		}else{
			fprintf(fp,"{%d,%d,%d},\n",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
			fprintf(temp_H_fp,"{%d,%d,%d},\n",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
		}
		code_ptr = code_ptr->next;
		count++;
	}

	fprintf(fp,"};\n");

	fprintf(temp_H_fp,"};\n");

	fprintf(temp_H_fp,"int num_dict_entries = %d;\n",count);

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	while(fgets(buffer, 256, code_H_fp) != NULL ) {
		if(strcmp(buffer,"//CODE END//\n") == 0){
			fprintf(temp_H_fp,"%s",buffer);
			break;
		}
	}
	while(fgets(buffer, 256, code_H_fp) != NULL ) {
	//printf("%s\n", buffer);
    		fprintf(temp_H_fp,"%s",buffer);
    	}
	fclose(code_H_fp);
	fclose(temp_H_fp);
	code_H_fp = fopen("../many_core.c","w");
	temp_H_fp = fopen("../temp_H.c","r");
	while(fgets(buffer, 256, temp_H_fp) != NULL ) {
	//printf("%s\n", buffer);
    		fprintf(code_H_fp,"%s",buffer);
    	}
	fclose(code_H_fp);
	fclose(temp_H_fp);
	remove("../temp_H.c");
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////


	char tmp;
	do
	{
 		tmp = fgetc(source_fp);
 		if(tmp == EOF)
 			break;
  		fputc(tmp, fp);
	} while (tmp != EOF);

	fprintf(fp,"\nint main()\n{\tstartup(%d,%d);\n\tclock_t tic = clock();\n\tinterpret();clock_t toc = clock();\n\t",main_addr,main_finder->num_nodes);


	fprintf(fp,"printf(\"Elapsed: %%f seconds\\n\", (double)(toc - tic) / CLOCKS_PER_SEC);");

	fprintf(fp,"return 0;\n}");


	fclose(fp);

	system("gcc -o output chc_output.c -O3 -g");
}





/*
	This function needs to do some heavy lifting.
	First, it has to color "main" program nodes to allocate them to a thread
		This can be done through some sophisticated graph analysis algorithms, but for now we'll just do random assignment

	Then, it has to generate multithreaded code (where all threads operate on the same data structure),
	starting each thread in the first corresponding node in the main function

	(must also keep a record of where the first node in each thread is, as an offset relative to st)
*/

//keeps track of sp offset per threads
struct thread_sp
{
	int offset;
	int num_nodes;
	struct thread_sp *next;
};

struct thread_sp *thread_stack_offsets = (struct thread_sp *)0;

int *colouring;
int colour_length;

void generate_output_mt(int n_threads)
{
	FILE *fp = fopen("chc_output.c","w");
	FILE *source_fp;


	if(n_threads > 1)
	{
		source_fp = fopen("startup/startup_mt.c","r");
	}
	else
	{
		source_fp = fopen("startup/startup.c","r");
	}



	struct code_scope *main_finder = program_code;

	int main_addr;




	while(strcmp(main_finder->scope_name,"main")!=0)
		main_finder = main_finder->next;
	main_addr = main_finder->address;

	int main_length = main_finder->length ;

	//we have start address and length of "main"

	struct code_scope *code_ptr;
	fprintf(fp,"#define _GNU_SOURCE\n");
	fprintf(fp,"#include<stdio.h>\n #include<stdlib.h>\n#include<time.h>\n");

	if(n_threads > 1)
	{

		//fprintf(fp,"#include <sched.h>\n");
		fprintf(fp,"#include <unistd.h>\n#include <pthread.h>\n");

	}

	fprintf(fp,"int code[] = {");

	int total_code_size = 0;


	//Create canonic program definition

	code_ptr = program_code;




	while(code_ptr != (struct code_scope *)0)
	{
		int length = code_ptr->length ;

		int *ip = code_ptr->code_ptr;
		int ctr = 0;

		fprintf(fp,"//End %s:\n",code_ptr->scope_name);

		if(strcmp(code_ptr->scope_name,"main")==0)
		{
			//here, we'll do something different
			//we'll color each node to assign it to a thread

			int *main_ip = coloured_main(n_threads, code_ptr);

	/*******************************************************************************************
			//TODO : for test only, return here
	********************************************************************************************/
			//return;

			while(length > 0)
			{
				if((code_ptr->next == (struct code_scope *)0) && (length == 1))
					fprintf(fp,"0x%x\n",*main_ip);
				else
					fprintf(fp,"0x%x,\n",*main_ip);
				main_ip++;
				length--;
				total_code_size++;
			}


		}
		else
		{
			while(length > 0)
			{
				if((code_ptr->next == (struct code_scope *)0) && (length == 1))
					fprintf(fp,"0x%x\n",*ip);
				else
					fprintf(fp,"0x%x,\n",*ip);
				ip++;
				length--;
				total_code_size++;
			}
		}
		fprintf(fp,"//Start %s @(%d):\n",code_ptr->scope_name,code_ptr->address);


		code_ptr = code_ptr->next;
	}

	fprintf(fp,"};\n");
	fprintf(fp,"int code_size = %d;\n",total_code_size);


	//create subgraph dictionary
	fprintf(fp,"int dictionary[][3] = {");

	code_ptr = program_code;
	while(code_ptr != (struct code_scope *)0)
	{
		if((code_ptr->next == (struct code_scope *)0))
			fprintf(fp,"{%d,%d,%d}",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
		else
			fprintf(fp,"{%d,%d,%d},",code_ptr->address,code_ptr->length,code_ptr->num_nodes);
		code_ptr = code_ptr->next;
	}

	fprintf(fp,"};\n");

	if(n_threads > 1)
	{
		//create thread info (offsets, first node)
		fprintf(fp,"int thread_info[%d][2];\n",n_threads);
		/*struct thread_sp *t_ptr = thread_stack_offsets;

		while(t_ptr != (struct thread_sp *)0)
		{
			if((t_ptr->next == (struct thread_sp *)0))
				fprintf(fp,"{%d,%d}",t_ptr->offset,t_ptr->num_nodes);
			else
				fprintf(fp,"{%d,%d},",t_ptr->offset,t_ptr->num_nodes);
			t_ptr = t_ptr->next;
		}*/

		//fprintf(fp,"};\n");

		fprintf(fp,"struct tcb{\n\tint *sb;\n\tint *sp;\n\tint nodes_to_evaluate;\n\tint nodes_evaluated;\n\tint nodes_visited;\n\tint nodes_GCed;\n\tFILE *fp;\n\t};\n");

		fprintf(fp,"struct tcb tcb[%d];\n",n_threads);

		fprintf(fp,"int **sb_tracker[%d];\n",n_threads);

		fprintf(fp,"int colouring[] = {");

		while(colour_length > 0)
		{
			if(colour_length == 1)
				fprintf(fp,"%d",*colouring);
			else
				fprintf(fp,"%d,",*colouring);
			colour_length--;
			*colouring++;
		}

		fprintf(fp,"};\n");


	}

	char tmp;
	do
	{
 		tmp = fgetc(source_fp);
 		if(tmp == EOF)
 			break;
  		fputc(tmp, fp);
	} while (tmp != EOF);

	if(n_threads > 1)
	{




		fprintf(fp,"\nint main()\n{\n\tnum_threads = %d;\n",n_threads);

		int i = n_threads;

		/*while(i > 0)
		{

			fprintf(fp,"\ttcb[%d].stack_size = 0;\n",i-1);
			i--;
		}*/

		i = n_threads;

		while(i > 0)
		{

			fprintf(fp,"\tsb_tracker[%d]=&(tcb[%d].sb);\n",i-1,i-1);
			i--;
		}
		i = n_threads;

		struct thread_sp *t_ptr = thread_stack_offsets;
		while(t_ptr != (struct thread_sp *)0)
		{
			fprintf(fp,"\tthread_info[%d][0]=%d;\n",i-1,t_ptr->offset);
			fprintf(fp,"\tthread_info[%d][1]=%d;\n",i-1,t_ptr->num_nodes);
			t_ptr = t_ptr->next;
			i--;
		}

		i = n_threads;

		while(i > 0)
		{




			//fprintf(fp,"\ttcb[%d].nodes_to_evaluate = 0;\n",i-1);
			//fprintf(fp,"\ttcb[%d].nodes_evaluated = 0;\n",i-1);
			//fprintf(fp,"\ttcb[%d].nodes_visited = 0;\n",i-1);
			//fprintf(fp,"\ttcb[%d].nodes_GCed = 0;\n",i-1);
			i--;
		}


		fprintf(fp,"\n\tstartup(%d,%d);\n",main_addr,main_finder->num_nodes);

		fprintf(fp,"cpu_set_t cpuset;\nint s;\n");

		i = n_threads;

		while(i > 0)
		{

			fprintf(fp,"\ttcb[%d].sb = (int *)((long unsigned int)((long unsigned int)sb + (long unsigned int)thread_info[%d][0]));\n",i-1,i-1);
			fprintf(fp,"\ttcb[%d].sp = tcb[%d].sb;\n",i-1,i-1);
			//fprintf(fp,"\ttcb[%d].stack_size = thread_info[%d][1];\n",i-1,i-1);

			fprintf(fp,"\tpthread_t thread%d;\n",i-1);

			i--;
		}



		i = n_threads;

		while(i > 0)
		{


			//fprintf(fp,"\ttcb[%d].fp = fopen(\"results_mt%d.txt\",\"w\");\n",i-1,i-1);

			//fprintf(fp,"\tfprintf(tcb[%d].fp,\"g_to_evaluate\\tto_evaluate\\tevaluated\\tvisited\\tGCed\\n\");",i-1);
			//fprintf(fp,"\tfprintf(tcb[%d].fp,\"%%d\\t%%d\\t%%d\\t%%d\\t%%d\\n\",nodes_to_evaluate,tcb[%d].nodes_to_evaluate,tcb[%d].nodes_evaluated,tcb[%d].nodes_visited,tcb[%d].nodes_GCed);",i-1,i-1,i-1,i-1,i-1);


			i--;
		}




		fprintf(fp,"pthread_barrier_init(&bar, NULL, %d);",n_threads+1);


		i = n_threads;
		while(i > 0)
		{
			fprintf(fp,"\tif(thread_info[%d][1])",i-1);
			fprintf(fp,"\t{\n\t\tpthread_create(&thread%d, NULL, interpret, &(tcb[%d]));\n\t}\n\n",i-1,i-1);
			//fprintf(fp,"CPU_ZERO(&cpuset);");
			//fprintf(fp,"CPU_SET(%d, &cpuset);\n",i);
			//fprintf(fp,"s=pthread_setaffinity_np(thread%d, sizeof(cpuset), &cpuset);",i-1);
			//fprintf(fp,"if (s != 0) {printf(\"affinity error %%d\\n\",s);}\n");
			i--;
		}


		fprintf(fp,"pthread_barrier_wait(&bar);");

		fprintf(fp,"\n\tclock_t tic = clock();\n");

		fprintf(fp,"pthread_barrier_destroy(&bar);");

		i = n_threads;
		while(i > 0)
		{
			fprintf(fp,"\tif(thread_info[%d][1])",i-1);
			fprintf(fp,"\t{\n\t\tpthread_join( thread%d, NULL);\n\t}\n",i-1);
			i--;
		}


		fprintf(fp,"\n\tclock_t toc = clock();\n");
		fprintf(fp,"printf(\"Elapsed: %%f seconds\\n\", (double)(toc - tic) / CLOCKS_PER_SEC);");

		i = n_threads;
		while(i > 0)
		{


			//fprintf(fp,"\tfclose(tcb[%d].fp);\n",i-1);

			i--;
		}


		fprintf(fp,"\treturn 0;\n}");
	}
	else
	{
		fprintf(fp,"\nint main()\n{\tstartup(%d,%d);\n\tinterpret();\n\treturn 0;\n}",main_addr,main_finder->num_nodes);
	}
	fclose(fp);

	system("gcc -o output chc_output.c -lpthread -lrt -O3 -g");
}


/*
	Creates a copy of main code with coloured nodes,
	changes sizes of nodes in copy for N threads, returns pointer to newly created copy of code
*/
int *coloured_main(int n_threads, struct code_scope *code_ptr)
{
	srand(time(0));

	//printf("Colouring graph %s, for %d nodes:\n",code_ptr->scope_name,code_ptr->num_nodes);

	colouring = (int *)malloc((code_ptr->num_nodes)*sizeof(int));

	colour_length = code_ptr->num_nodes;

	int *colouring_ptr = colouring;
	int node_ctr = code_ptr->num_nodes;

	int done_nodes = 0;

	while(node_ctr > 0)
	{
		//Insert sophisticated graph analysis algorithm here
		*colouring_ptr = rand()%n_threads;
		colouring_ptr++;
		node_ctr--;
	}

	//just for MT 2 test
	/*
	int best[12] = {0,0,1,1,0,0,1,1,0,0,1,1};
	int i = 0;
	while(node_ctr > 0)
	{
		//Insert sophisticated graph analysis algorithm here
		*colouring_ptr = best[i];
		i++;
		colouring_ptr++;
		node_ctr--;
	}*/

	//just for MT 4 test

	/*int best[12] = {0,1,2,3,0,1,2,3,0,1,2,3};
	int i = 0;
	while(node_ctr > 0)
	{
		//Insert sophisticated graph analysis algorithm here
		*colouring_ptr = best[i];
		i++;
		colouring_ptr++;
		node_ctr--;
	}*/



	//debug prints
	/*int colour_printer = code_ptr->num_nodes;
	colouring_ptr = colouring;

	//printf("Coloured %d nodes:\n",colour_printer);

	while(colour_printer > 0)
	{
		printf("%d\n",*colouring_ptr);
		colouring_ptr++;
		colour_printer--;
	}
	colouring_ptr = colouring;
*/
	//end debug prints



	//now, based on the color of this and next node, we adjust the size
	//(threads use node size to move to next node)

	int *new_code = (int *)malloc(code_ptr->length * sizeof(int));

	memcpy((void *)new_code, (const void *)(code_ptr->code_ptr), (size_t)(code_ptr->length * sizeof(int)));

	int *ptr = new_code;
	colouring_ptr = colouring;

	node_ctr = code_ptr->num_nodes;

	while(node_ctr > 1)
	{

		//if next node is of different thread
		if(*(colouring_ptr + 1) != *(colouring_ptr))
		{
			//printf("Found size to change\n");
			//count sizes up to first new node of same thread

				//find how many nodes to skip
			int skip_number = 0;
			int *skip_ctr = colouring_ptr + 1;

			//TODO
			//Needs refactoring so doesnt go out of bounds
			do
			{
				skip_number++;
				skip_ctr++;
			} while((*skip_ctr != *colouring_ptr) && ((done_nodes + skip_number) < (code_ptr->num_nodes - 1)));

			//printf("Must skip %d nodes\n",skip_number);

			//calculate skip size
			int *size_skipper = (int *)((long unsigned int)((long unsigned int)ptr + (long unsigned int)(*(ptr + 3))));
			//printf("got ptr %lx, +3 %lx\n",(long unsigned int)ptr,(long unsigned int)(*(ptr + 3)));
			//printf("pointer is %lx, skipper is %lx\n",(long unsigned int)ptr,(long unsigned int)size_skipper);
			int added_size = 0;
			while(skip_number > 0)
			{
				added_size += *(size_skipper + 3);

				size_skipper = (int *)((long unsigned int)((long unsigned int)(size_skipper) +  (long unsigned int)(*(size_skipper + 3))));
				skip_number--;
			}
			//printf("Adding %x size\n",added_size);
			//increment size

			//*(ptr + 3) = ((long unsigned int)((long unsigned int)(*(ptr + 3))) +  (long unsigned int)(added_size));
			//ptr += ((long unsigned int)((long unsigned int)(*(ptr + 3))) -  (long unsigned int)(added_size));

			int *tmp_ptr = ptr;
			ptr = (int *)((long unsigned int)((long unsigned int)(*(ptr + 3))) +  (long unsigned int)(ptr));
			*(tmp_ptr + 3) = ((long unsigned int)((long unsigned int)(*(tmp_ptr + 3))) +  (long unsigned int)(added_size));
		}
		else
		{
		//else, next node is of same thread
			//leave unchanged
			ptr = (int *)((long unsigned int)((long unsigned int)(*(ptr + 3))) +  (long unsigned int)(ptr));
			//ptr += *(ptr + 3);
		}

		colouring_ptr++;
		node_ctr--;
		done_nodes++;
	}


	//before we overwrite canonic main code
	//create thread sp offsets

	//thread_stack_offsets = (struct thread_sp *)malloc(sizeof(struct thread_sp));

	//thread 0
	//thread_stack_offsets->offset = 0;
	//thread_stack_offsets->next = (struct thread_sp *)0;

	//struct thread_sp *thread_stack_offsets;

	struct thread_sp *thread_tmp;

	int thread_ctr = 0;

	while(thread_ctr < (n_threads))
	{
		thread_tmp = (struct thread_sp *)malloc(sizeof(struct thread_sp));
		thread_tmp->next = thread_stack_offsets;
		thread_tmp->num_nodes = 0;

		//find number of nodes assigned to this thread
		int total_nodes = code_ptr->num_nodes;
		colouring_ptr = colouring;

		while(total_nodes > 0)
		{
			if(*colouring_ptr == thread_ctr)
			{
				thread_tmp->num_nodes++;
			}

			total_nodes--;
			colouring_ptr++;
		}

		//find offset of first node assigned to this thread

		int first_element = 0;
		total_nodes = code_ptr->num_nodes;
		colouring_ptr = colouring;

		//assign as (first after) last element first
		thread_tmp->offset = code_ptr->num_nodes;

		while(total_nodes > 0)
		{

			if(*colouring_ptr == thread_ctr)
			{
				if(first_element < thread_tmp->offset)
					thread_tmp->offset = first_element;
			}

			first_element++;
			total_nodes--;
			colouring_ptr++;
		}

		//thread_tmp->offset now contains the index of first corresponding node
		//need to transform into offset in bytes (by traversing main)
		int offset = thread_tmp->offset;

		thread_tmp->offset = 0;

		int *tmp_code_ptr = code_ptr->code_ptr;

		while(offset > 0)
		{
			thread_tmp->offset += *(tmp_code_ptr + 3);

			tmp_code_ptr = (int *)((long unsigned int)((long unsigned int)(*(tmp_code_ptr + 3))) +  (long unsigned int)(tmp_code_ptr));

			offset--;
		}


		thread_stack_offsets = thread_tmp;
		thread_ctr++;
	}

	//offsets done, can get rid of original main code

	memcpy((void *)(code_ptr->code_ptr), (const void *)new_code,(size_t)(code_ptr->length * sizeof(int)));
	//printf("After MT\n");
	//print_adjusted_machine_code(program_code);
}
