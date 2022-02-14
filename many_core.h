#ifndef MANY_CORE_H
#define MANY_CORE_H


extern struct FIFO **buss;//buss master output
extern pthread_mutex_t buss_lock;
extern pthread_t *thread_id;

struct FIFO{
    int size;
    struct Message *front, *back;
};

struct Message{
		unsigned int addr;
		int data;
    int dest;
    int repeated;
		struct Message *next;
};

struct CPU{
  int *PM; //program mem read only
  int *stack; //active program
  int pc; //program counter
  int lp; //node list pointer
  int sp; //points to the node being executed
  int sp_top; //pointes to top of node list in stack
  int code_size; //size of PM

  int oper; //if next mesage is part of operation message
  int next_op; //next op to be executed once all the info is saved in the expand_buffer
  int eom_count; //count num end of message for expansion messages
  int largest_offset;

  struct FIFO *expand_buffer;
  struct FIFO *broadcast;

  struct FIFO **routing_table;
};

struct FIFO *create_FIFO();
void sendMessageOnBuss(int cpu_num,struct Message *m);
void sendMessage(struct FIFO *fifo, struct Message *m);
struct Message *popMessage(struct FIFO *fifo);
struct Message *peekMessage(struct FIFO *fifo);
void removeMessage(struct FIFO *fifo);
int getFifoSize(struct FIFO *fifo);
int getCpuNum(struct Message *message);
int getRW(struct Message *message);
int getAddr(struct Message *message);
int getData(struct Message *message);
int find_cpu_num(int func_offset, int dest_offset);
struct FIFO ** set_up_routing_table(int cpu_num, struct FIFO **rt);
#endif
