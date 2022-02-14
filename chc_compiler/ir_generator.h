#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

//Not a Value useful for IR printing
#define NAV			0xFFFFFFFC
//Dead operator: remove it
#define DEAD 		0xFFFFFFFF
//Can process corresponding operation and send result to destinations (all arguments resolved)
#define READY 		0


/*
	Operator readiness:

	if >0, waiting for that number of arguments (e.g., if 1, need 1 more argument)

	if <0, operator has become dead (atomatic GC): can be destroyed without further action

	if 0, it's ready to be processed:
		either its arguments have just been filled
		or it's a constant (already fully constructed)
		or it's an input that requires 0 arguments 
*/

//little Linked list for destination in operators (just hold a string)
//needs an inner_name for expansions, to specify which node is destination
struct destination_node
{
	char *inner_name;
	char *destination;
	struct destination_node *next;	
};

//little Linked list for argument tuples
//tuples are only used for expansions, where we need value and inner node name
//for other operations, only value is used

//expansions are also the only operators expected to have more than 2 arguments
struct argument_node
{
	char *inner_name;
	int arg_value;
	struct argument_node *next;
};


//structure for the IR representation of a single datum
struct datum_ir
{
	//"name "can disappear for code generation, useful for IR interpretation
	char *name;
	/*
	Operator readiness:

	if >0, waiting for that number of arguments (e.g., if 1, need 1 more argument)

	if <0, operator has become dead (atomatic GC): can be destroyed without further action

	if 0, it's ready to be processed:
		either its arguments have just been filled
		or it's a constant (already fully constructed)
		or it's an input that requires 0 arguments 
*/
	unsigned int created_count;
	//operation to construct the datum once operators are ready
	char *operation;
	//argument values (tuples)
	struct argument_node *args;
	//datum constructed value
	int value;
	//destination: either another datum argument, or (OUTPUT)
	//TODO: add counter to have variadic destinations
	//will require another Linked List here
	struct destination_node *destination;

	//not part of human-readable IR representation, just linking IRs per subgraph
	struct datum_ir *next;
};

//structure for the IR representation of a complete scope (subgraph)
struct scope_ir
{
	//"name "can disappear for code generation, useful for IR interpretation
	char *name;
	//all nodes in this scope
	struct datum_ir *nodes;

	//not part of human-readable IR representation, just linking IRs per program	
	struct scope_ir *next;
};

void print_human_readable_IR(struct scope_ir *IR);


int generate_ir(struct ast_node *ast);
int generate_scope_ir(struct ast_node *ast);

struct datum_ir *find_IR_node_by_name(char *name, struct datum_ir *IR);


#endif