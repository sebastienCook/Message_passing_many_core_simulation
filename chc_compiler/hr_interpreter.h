#ifndef HR_INTERPRETER_H
#define HR_INTERPRETER_H


void hr_interpret_IR(struct scope_ir *IR, char *subgraph);
struct datum_ir *find_subgraphnodes_inIR(struct scope_ir *IR, char *subgraph);
void push(struct datum_ir **stack,struct datum_ir *node);

struct argument_node *args_deep_copy(struct argument_node *args);
struct destination_node *destination_deep_copy(struct destination_node *destination);

void print_stack(struct datum_ir *stack, struct datum_ir *sp);

struct datum_ir *find_target_node(struct datum_ir *stack, char *dest);

int node_not_dead(struct datum_ir *sp);
int node_ready(struct datum_ir *sp);
int node_has_value(struct datum_ir *sp);

void evaluate_constructed(struct datum_ir *sp, struct datum_ir *stack);
void evaluate_ready(struct datum_ir *sp, struct datum_ir **pointer_to_stack, struct scope_ir *IR, int *stack_size);

void kill_node(struct datum_ir *sp);

#endif