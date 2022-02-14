#ifndef CODE_OUTPUT_H
#define CODE_OUTPUT_H

void generate_output();
void generate_output_mt(int n_threads);
int *coloured_main(int n_threads, struct code_scope *code_ptr);

#endif