#ifndef CPU_HW
#define CPU_HW

#define UNDEFINED -1
#define STACK_UNDEFINED -100
#define UNKNOWN 0
#define IGNORE -2
#define OUTPUT -1


#define code_expansion 	0
#define code_input		1
#define code_output		2 //output isn't really an operation, it's a destination
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
#define code_end 13
#define MAD 14
#define SDOWN 15
#define EXP 16
#define FND 17
#define IDLE 18
#define LFN 19
#define SSU 20
#define COM 21
#define code_if_else_fail 22
#define code_expansion_add 23
#define code_expansion_remap 24
#define code_expansion_input 25
#define EXP_add 26
#define EXP_remap 27
#define EXP_input 28


//#define code_output		0xFFFFFFFF	//convenient to have it set to a special value that can be tested at runtime
#define NAV			0xFFFFFFFC
#define NODE_BEGIN_FLAG 0x7fffffff
//Can process corresponding operation and send result to destinations (all arguments resolved)
#define READY 		0

#define ADDRASABLE_SPACE 640000
#define FTFN_MAX 5 //the number of time Looking for a node can fail before sending a node request broadcast





#define EOM -6  //end of message
#define OPR 131071

struct CPU_SA{
    int cpu_num;
    int *PM; //hold cpu portion of program
    int (*dictionary)[3];
    int num_dict_entries;
		int main_addr;
		int code_size;
		int num_cpu;
    struct FIFO **routing_table;
};

void *CPU_SA_start();
struct Message*  Message_packing(int cpu_num, int addr, int data );
void pack_and_sendMessage(struct FIFO *fifo,int cpu_num, int addr, int data );
int getSize(int entry);
int getOffset(int entry);
#endif
