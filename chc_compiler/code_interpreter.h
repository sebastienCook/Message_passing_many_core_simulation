#ifndef CODE_INTERPRETER_H
#define CODE_INTERPRETER_H

#define STACK_SIZE 65536

void startup();
void push_subgraph(int addr,int size);
void print_code_stack();
void interpret();
void interpret_constructed();
void interpret_ready();

#endif
