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

extern FILE *yyin;

int linenum = 1;

extern struct ast_node *ast;
extern struct scope_ir *IR;
extern struct annotated_IR_scope *annotated_IR;

int main(int argc, char *argv[]) 
{
	int errors;
	if(argc != 3)
	{
		printf("Undefined input file or missing thread specification. Usage: \"chc <source file> <number of threads>\"\n");
		return 0;
	}

	if((atoi(argv[2]) < 1))
	{
		printf("Invalid number of processing threads specified (%d).\n",atoi(argv[2]));
		return 0;
	}
	int n_threads = atoi(argv[2]);

	yyin = fopen(argv[1],"r");
	if(yyin == NULL)
	{
		printf("Could not open input file %s\n",argv[1]);
		return 0;
	}

	yyparse();

	//Finished parsing: print AST for debug/test
	//print_ast(ast);
	printf("Done parsing\n");
	//semantic analysis to check validity of code
	if(errors = check_semantics(ast))
	{
		printf("%d errors found during semantic analysis.\n",errors);
		return 0;
	}
	printf("Done semantics\n");
	if(errors = generate_ir(ast))
	{
		printf("%d errors found generating IR.\n",errors);
		return 0;
	}
	printf("Done IR\n");
	//print_human_readable_IR(IR);
	
	//hr_interpret_IR(IR,"main");

	populate_annotated_IR(IR);
	printf("Done annotated\n");
	generate_machine_code(annotated_IR);
	printf("Done machine\n");



	//"machine" code interpretation
	//startup();
	//interpret();

	if(n_threads == 1)
		generate_output();
	else
		generate_output_mt(n_threads);
}
