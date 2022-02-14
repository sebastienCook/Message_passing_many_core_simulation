#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "many_core.h"
//#include "2by2sim.h"
#include "cpu.h"


//dynamic code array (starts as copy of original code)
int *runtime_code;
//cpu status array
int *cpu_status;
//used to keep track of node numbers
int list_index;
//This is the number of dead nodes (0 destinations) that were removed (needed for node_dest allignment
int num_nodes;
//the list of all tasks (that have more than 0 destinations)
struct AGP_node *program_APG_node_list;
//main mutex
pthread_mutex_t buss_lock;
//buss master in
struct FIFO *buss_Min;
//buss master out
struct FIFO *buss_Mout;

struct FIFO **buss;

pthread_t *thread_id;

int NUM_CPU;
//FOR OUTPUT DISPLAY
int MESSAGE;
int GRAPH;
struct data_entry **data;
clock_t BEGIN;

FILE *tick_count;
FILE *tick_types;
FILE *buffer_size;
FILE *bytes_inout;

//DO NOT REMOVE THE LINE BELLOW!! File may become corrupt if it is (used to write code array in)
//CODE BEGINE//
const int code[] = {//End main:
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x184,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x17c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x174,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x16c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x164,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x15c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x154,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x14c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x144,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x13c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x134,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x12c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x124,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x11c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x114,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x10c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x104,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xfc,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xf4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xec,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xe4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xdc,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xd4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xcc,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xc4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xbc,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xb4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xac,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xa4,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x9c,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x94,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0x8c,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0x1a8,
0xffffffff,
0x7fffffff,
0x10,
0xfffffffc,
0x24,
0xd,
0x2,
0x0,
0x0,
0x0,
0x7fffffff,
0x20,
0xfffffffc,
0x1a0,
0x0,
0x20,
0x0,
0x11a8,
0x0,
0x11d4,
0x0,
0x1200,
0x0,
0x122c,
0x0,
0x1258,
0x0,
0x1284,
0x0,
0x12b0,
0x0,
0x12dc,
0x0,
0x1308,
0x0,
0x1334,
0x0,
0x1360,
0x0,
0x138c,
0x0,
0x13b8,
0x0,
0x13e4,
0x0,
0x1410,
0x0,
0x143c,
0x0,
0x1468,
0x0,
0x1494,
0x0,
0x14c0,
0x0,
0x14ec,
0x0,
0x1518,
0x0,
0x1544,
0x0,
0x1570,
0x0,
0x159c,
0x0,
0x15c8,
0x0,
0x15f4,
0x0,
0x1620,
0x0,
0x164c,
0x0,
0x1678,
0x0,
0x16a4,
0x0,
0x16d0,
0x0,
0x16fc,
0x10,
0x0,
0x1d0,
0xf24,
0x1f8,
0xf4c,
0x220,
0xf74,
0x248,
0xf9c,
0x270,
0xfc4,
0x298,
0xfec,
0x2c0,
0x1014,
0x2e8,
0x103c,
0x310,
0x1064,
0x338,
0x108c,
0x360,
0x10b4,
0x388,
0x10dc,
0x3b0,
0x1104,
0x3d8,
0x112c,
0x400,
0x1154,
0x428,
0x117c,
//Start main @(1472):
//End mtrx_multiply:
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xd04,
0xda4,
0xe44,
0xee4,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xcdc,
0xd7c,
0xe1c,
0xebc,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xcb4,
0xd54,
0xdf4,
0xe94,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xc8c,
0xd2c,
0xdcc,
0xe6c,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xa84,
0xb24,
0xbc4,
0xc64,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xa5c,
0xafc,
0xb9c,
0xc3c,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xa34,
0xad4,
0xb74,
0xc14,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0xa0c,
0xaac,
0xb4c,
0xbec,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x804,
0x8a4,
0x944,
0x9e4,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x7dc,
0x87c,
0x91c,
0x9bc,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x7b4,
0x854,
0x8f4,
0x994,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x78c,
0x82c,
0x8cc,
0x96c,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x584,
0x624,
0x6c4,
0x764,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x55c,
0x5fc,
0x69c,
0x73c,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x534,
0x5d4,
0x674,
0x714,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x50c,
0x5ac,
0x64c,
0x6ec,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x760,
0x9e0,
0xc60,
0xee0,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x6c0,
0x940,
0xbc0,
0xe40,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x620,
0x8a0,
0xb20,
0xda0,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x580,
0x800,
0xa80,
0xd00,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x738,
0x9b8,
0xc38,
0xeb8,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x698,
0x918,
0xb98,
0xe18,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x5f8,
0x878,
0xaf8,
0xd78,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x558,
0x7d8,
0xa58,
0xcd8,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x710,
0x990,
0xc10,
0xe90,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x670,
0x8f0,
0xb70,
0xdf0,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x5d0,
0x850,
0xad0,
0xd50,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x530,
0x7b0,
0xa30,
0xcb0,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x6e8,
0x968,
0xbe8,
0xe68,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x648,
0x8c8,
0xb48,
0xdc8,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x5a8,
0x828,
0xaa8,
0xd28,
0x7fffffff,
0x0,
0xfffffffc,
0x2c,
0x1,
0x0,
0x4,
0x508,
0x788,
0xa08,
0xc88,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x4e4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x4e0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x4bc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x4b8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x494,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x490,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x46c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x468,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x444,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x440,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x41c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x418,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3f4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3f0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3cc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3c8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3a4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x3a0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x37c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x378,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x354,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x350,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x32c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x328,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x304,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x300,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x2dc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x2d8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x2b4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x2b0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x28c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x288,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x264,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x260,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x23c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x238,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x214,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x210,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x1ec,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x1e8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x1c4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x1c0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x19c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x198,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x174,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x170,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x14c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x148,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x124,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x120,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xfc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xf8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xd4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xd0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xac,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xa8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x84,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x80,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x5c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x58,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x34,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x30,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0xc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1164,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1160,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x113c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1138,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1114,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1110,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x10ec,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x10e8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x10c4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x10c0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x109c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1098,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1074,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1070,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x104c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1048,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1024,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x1020,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xff8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xfd4,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xfd0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xfac,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xfa8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf84,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf80,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf5c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf58,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf34,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf30,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf0c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xf08
//Start mtrx_multiply @(0):
};
int code_size = 2001;
int main_addr = 1472;
int main_num_nodes = 50;
int dictionary[][3] = {{1472,529,50},
{0,1472,144}
};
int num_dict_entries = 2;
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//

/**
 * @brief size function
 *
 * This function is called to calculate the size of each node. (how many entries does the node have)
 * @param [in] addr is the beggining of the node.
 * @param [out] size of the node.
 * @return int
 **/
 int size(int addr){
 	//find size
 	int i = addr + 1;
 	int size = 1;
 	while(code[i] != NODE_BEGIN_FLAG && i < code_size){
 		size++;
 		i++;
 	}
 	return size;
 }

//return 1 success return 0 fail
int run(struct CPU **cores, int number_of_cores){

  int ticks=0;//number of loops accross all cores
  int tick_type[number_of_cores][5]; //0:processing 1:garbage collection 2:result propogation 3:expansion call 4:com block
  for(int i=0;i<number_of_cores;i++){
    for(int j=0;j<5;j++){
      tick_type[i][j]=0;
    }
  }
  int bytes_in_out[number_of_cores][2];
  for(int i=0;i<number_of_cores;i++){bytes_in_out[i][0]=0;bytes_in_out[i][1]=0;} //0:in 1:out
  int cpu_num=0;
  struct CPU *core;
  int pc,sp,lp,sp_top,oper,next_op,eom_count,largest_offset;
  int *stack;
  struct FIFO *expand_buffer;
  struct FIFO *broadcast;
  struct FIFO *buffer;

  int done=0;

  while(done==0){
    core = cores[cpu_num];
    pc = core->pc;
    lp = core->lp;
    sp = core->sp;
    sp_top = core->sp_top;
    oper = core->oper;
    next_op = core->next_op;
    eom_count = core->eom_count;
    largest_offset = core->largest_offset;


    expand_buffer = core->expand_buffer;
    broadcast = core->broadcast;
    buffer = core->routing_table[cpu_num];
    stack = core->stack;

    //printf("cpu_num %d pc: %d\n",cpu_num,pc);
    //if(cpu_num==3)
    //  printf("tick: %d\n",ticks);
  //  sleep(0.75);
    //printf("core %d sp_top: %d pc: %d\n",cpu_num,sp_top,pc);


    switch(pc){
      case code_input:
			{
				if(stack[sp+2] == NAV){
					printf("\t<< "); scanf("%d",&stack[sp+2]);//stack[2] or stack[sp_top+2]
				}
				pc=FND;
        tick_type[cpu_num][0]++;
				break;
			}
			//op code add
			case code_plus:
			  //add
				stack[sp+2] = stack[sp+6]+stack[sp+7];
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			case code_minus:
				stack[sp+2] = stack[sp+6]-stack[sp+7];
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			case code_times:
				stack[sp+2] = stack[sp+6]*stack[sp+7];
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			case code_is_equal:
				stack[sp+2] = (stack[sp+6] == stack[sp+7]) ? 1 : 0;
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			case code_is_less:
				stack[sp+2] = (stack[sp+6] < stack[sp+7]) ? 1 : 0;
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			case code_is_greater:
				stack[sp+2] = (stack[sp+6] > stack[sp+7]) ? 1 : 0;
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
      case code_merge:
  			{
  				stack[sp+2] = (stack[sp+ 6] | stack[sp+7]);
  				//printf("CPU %d MERGE %d & %d = %d\n",cpu_num,stack[6],stack[7],stack[2]);
  				pc = FND;
          tick_type[cpu_num][0]++;
  				break;
  			}
      case code_if:
  			{
  				if((stack[sp+6] != 0))
  				{
  					(stack[sp+2] = stack[sp+7]);
  					pc = FND;
  				}
  				else
          {
            pc = code_if_else_fail;
  				}
          tick_type[cpu_num][0]++;
  				break;
  			}
  	  case code_else:
			{
				if(stack[sp+6] == 0)
				{
					(stack[sp+2] = stack[sp+7]);
					pc = FND;
				}
				else
				{
          pc = code_if_else_fail;
				}
        tick_type[cpu_num][0]++;
				break;
			}
      case code_if_else_fail:
      {
        int	num_dest = stack[sp+6+stack[sp+5]];
        if(num_dest>0){
          int	doffset = sp+6+stack[sp+5]+1+(2*(num_dest-1));
          if(stack[doffset] != OUTPUT){
            int  m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
              pack_and_sendMessage(core->routing_table[stack[doffset+1]],stack[doffset+1],OPR,MAD);
              pack_and_sendMessage(core->routing_table[stack[doffset+1]],stack[doffset+1],m_addr,MAD);
              bytes_in_out[cpu_num][1]+=2;
          }
          stack[sp+6+stack[sp+5]]--;
          tick_type[cpu_num][4]++;//added to com time
        }else{
          tick_type[cpu_num][0]++;
          pc=SDOWN;
        }
        break;
      }
  		case code_identity:
			{
				if(stack[sp+2] == NAV){stack[sp+2] = stack[sp+6];}
				pc = FND;
        tick_type[cpu_num][0]++;
				break;
			}
      case code_end:

        //return 1;
        tick_type[cpu_num][0]++;
        done++;
        break;
      case code_expansion:
      {
       // printf("core %d calling exp: %d\n",cpu_num,largest_offset);
      //  return 0;
        //set up environment for expanding
        pack_and_sendMessage(broadcast,cpu_num,OPR,EXP);
        int func_offset = stack[sp+6+(stack[sp+5]*3)+1];
				pack_and_sendMessage(broadcast,cpu_num,func_offset,largest_offset);

        stack[ADDRASABLE_SPACE-2] = sp;
        stack[ADDRASABLE_SPACE-3] = func_offset;
        sp = core->code_size-1;
        pc = code_expansion_add;
        tick_type[cpu_num][0]++;
        break;
      }
      case code_expansion_add:
      {
        //add nodes
        int func_offset = stack[ADDRASABLE_SPACE-3];

        if(sp>0){ //adding nodes to stack
          if(core->PM[sp] == func_offset){ //check if the node is in main
            //add to stack
            lp++;
            sp_top--;
            while(core->PM[sp-1] != NODE_BEGIN_FLAG){
              stack[sp_top]=core->PM[sp];
              sp_top--;sp--;
            }
            stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted

            if(stack[sp_top+4]==code_expansion){
              stack[lp+1] = core->PM[sp+3]-stack[sp_top+5]-stack[sp_top+6+(stack[sp_top+5]*3)]; //needs to be changed to pack size and offset it same
            }else{
              stack[lp+1] = core->PM[sp+3]-stack[sp_top+6+stack[sp_top+5]]; //needs to be changed to pack size and offset it same
            }

            stack[lp+2]= largest_offset+core->PM[sp]-func_offset;
            lp+=2;sp--;
            stack[sp_top] = core->PM[sp];
            stack[sp_top+stack[sp_top+3]-1] = largest_offset;
						if(stack[sp_top+4]==code_input){stack[sp_top+1]=1;}
            sp--;
          }else{
            while(core->PM[sp] != NODE_BEGIN_FLAG){sp--;}
            sp--;
          }
        }else{ //done traversing pm
          pc = code_expansion_remap;
          sp = stack[ADDRASABLE_SPACE-2];
        }//*/
        tick_type[cpu_num][0]++;
        break;
      }
      case code_expansion_remap:
      {
        //remap dest if has it
        int num_dest = stack[sp+6+(stack[sp+5]*3)];

				if(num_dest > 0){
          int doffset = sp+6+(stack[sp+5]*3)+2+(4*(num_dest-1));
				 	int node_to_remap = largest_offset+stack[doffset+2]/4;
					//printf("node to remap %d\n",node_to_remap);
					//must remove its scope offset and add the scope offset that it wants to send too.
					//however when a result is sent, the dest offset is /4 so here we multiply by 4 to compensate
					int remap_to_this = (stack[sp+stack[sp+3]-1] + stack[doffset]/4 - largest_offset)*4;

					if(stack[doffset+3]==cpu_num){ //if local write to node in stack
						int lp_t = lp;
						int size;
						int offset;
						int found = 0;
						//now lets see if its dest is in its own stack
						while(lp_t>=0){
								size = stack[lp_t-1];
								offset = stack[lp_t];
								//if true the val is for it
								if(node_to_remap < offset && node_to_remap > offset-size){
									int ntr_num_dest = stack[stack[lp_t-2]+6+stack[stack[lp_t-2]+5]];
									for(int k=0;k<ntr_num_dest;k+=2){
										int off = stack[lp_t-2]+7+stack[stack[lp_t-2]+5]+k;
										if( stack[off] == OUTPUT){
											 stack[off] = remap_to_this;
											 stack[off+1] = stack[doffset+1];
											 break;
										}
									}
									break;
								}
							lp_t-=3;
						}
					}else{ //not local so must send to node
						pack_and_sendMessage(broadcast,cpu_num,node_to_remap,remap_to_this);
						pack_and_sendMessage(broadcast,cpu_num,stack[doffset+3],stack[doffset+1]); //cpu num to send to
					}
          stack[sp+6+(stack[sp+5]*3)]--;
				}else{
          pack_and_sendMessage(broadcast,cpu_num,OPR,EOM);
          pc = code_expansion_input;
        }
        tick_type[cpu_num][0]++;
        break;
      }
      case code_expansion_input:
      {
        //write input in if has it then broadcast expanding
        int num_args = stack[sp+5];
				if(num_args>0){
					int aoffset = sp + 6 + (3*(num_args-1));
					int input_node_offset = largest_offset+stack[aoffset+1]/4;

					if(stack[aoffset+2]==cpu_num){
						int lp_t = lp;
						int size;
						int offset;
						int found = 0;
						//now lets see if its dest is in its own stack
						while(lp_t>=0){
								size = stack[lp_t-1];
								offset = stack[lp_t];
								//if true the val is for it
								if(input_node_offset  < offset && input_node_offset  > offset-size){
									//change output dest
									stack[stack[lp_t-2]+2] = stack[aoffset];
									stack[stack[lp_t-2]+1]-=1;
									break;
								}
							lp_t-=3;
						}
					}else{
						pack_and_sendMessage(broadcast,cpu_num,input_node_offset-2,stack[aoffset]);
					}
          stack[sp+5]--;
          tick_type[cpu_num][0]++;
				}else{
          pack_and_sendMessage(broadcast,cpu_num,OPR,EOM);
          largest_offset += (code_size*(number_of_cores));
          //broadcast message
          struct Message *t;
          int eom_c = 0;
  				for(int i=0;i<number_of_cores; i++){
  					if(i!=cpu_num){
  						while(eom_c!=2){
  							t = popMessage(broadcast);
  							if(getAddr(t) == OPR && getData(t)==EOM){eom_c++;}
  							pack_and_sendMessage(core->routing_table[i],i,getAddr(t),getData(t));
  							sendMessage(broadcast,t);
                free(t);
                bytes_in_out[cpu_num][1]++;
  						}
  					}
  					eom_c=0;
  				}
          while(t!=NULL){t=popMessage(broadcast);free(t);}//empty q
          pc = SDOWN;
          tick_type[cpu_num][4]++;//added to com time
          free(t);
        }
        break;
      }
      case EXP:
      {
      //  printf("\n\nEXP CALLED %d\n\n",getFifoSize(expand_buffer));
        struct Message *t = popMessage(expand_buffer);
        stack[ADDRASABLE_SPACE-2] = sp;
        stack[ADDRASABLE_SPACE-3] = getAddr(t); //func offset

      //  printf("\n\nsp_top: %d\nADDRASABLE_SPACE-4: %d\nADDRASABLE_SPACE-5: %d\n",sp_top,stack[ADDRASABLE_SPACE-4],stack[ADDRASABLE_SPACE-5]);
        stack[ADDRASABLE_SPACE-4] = getData(t); //largest offset
      /*  printf("ADDRASABLE_SPACE-4: %d\n\n",stack[ADDRASABLE_SPACE-4]);
        for(int i = 0; i<=lp;i+=3){
              printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
              printf("CPU %d [%d][%d]\n",cpu_num,i+1,stack[i+1]);
              printf("CPU %d [%d][%d]\n",cpu_num,i+2,stack[i+2]);
        }
        for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
          printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
        }
        return 0;//*/

        sp = core->code_size-1;
        pc = EXP_add;
        tick_type[cpu_num][3]++;
        free(t);
        break;
      }
      case EXP_add:
      {
        //add nodes
        int func_offset = stack[ADDRASABLE_SPACE-3];
        int new_func_offset = stack[ADDRASABLE_SPACE-4];


        if(sp>0){ //adding nodes to stack
          if(core->PM[sp] == func_offset){ //check if the node is in main
            //add to stack'
            lp++;
            sp_top--;
            while(core->PM[sp-1] != NODE_BEGIN_FLAG){
              stack[sp_top]=core->PM[sp];
              sp_top--;sp--;
            }
            stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted

            if(stack[sp_top+4]==code_expansion){
              stack[lp+1] = core->PM[sp+3]-stack[sp_top+5]-stack[sp_top+6+(stack[sp_top+5]*3)]; //needs to be changed to pack size and offset it same
            }else{
              stack[lp+1] = core->PM[sp+3]-stack[sp_top+6+stack[sp_top+5]]; //needs to be changed to pack size and offset it same
            }
            stack[lp+2]= new_func_offset+core->PM[sp]-func_offset;

            lp+=2;sp--;
            stack[sp_top] = core->PM[sp];
            stack[sp_top+stack[sp_top+3]-1] = new_func_offset;
						if(stack[sp_top+4]==code_input){stack[sp_top+1]=1;}
            sp--;
          }else{
            while(core->PM[sp] != NODE_BEGIN_FLAG){sp--;}
            sp--;
          }
        }else{ //done traversing pm
          pc = EXP_remap;
        }//*/
        tick_type[cpu_num][3]++;
        break;
      }
      case EXP_remap:
      {
        struct Message *t = popMessage(expand_buffer);
        if(getAddr(t)!=OPR){

          int ntr_offset = getAddr(t);
          int rtt_offset = getData(t);
          free(t);
          t = popMessage(expand_buffer);
          int ntr_num = getAddr(t);
					int rtt_num = getData(t);

          if(ntr_num == cpu_num){
            int lp_t = lp;
						int size;
						int offset;
						int found = 0;
						//now lets see if its dest is in its own stack
						while(lp_t>=0){
								size = stack[lp_t-1];
								offset = stack[lp_t];
								//if true the val is for it
								if(ntr_offset < offset && ntr_offset > offset-size){
									found = 1;
									break;
								}
							lp_t-=3;
						}
						//send or write result
						if(found){
							int ntr_num_dest = stack[stack[lp_t-2]+6+stack[stack[lp_t-2]+5]];
							for(int k=0;k<ntr_num_dest;k++){
								int off = stack[lp_t-2]+7+stack[stack[lp_t-2]+5]+k;
								if( stack[off] == OUTPUT){
									 stack[off] = rtt_offset;
									 stack[off+1] = rtt_num;
									 break;
								}
							}
						}
          }
        }else{
          pc=EXP_input;
        }
        tick_type[cpu_num][3]++;
        free(t);
        break;
      }
      case EXP_input:
      {
        struct Message *t = popMessage(expand_buffer);
        int m_addr  = getAddr(t);
        if(m_addr !=OPR){

          int lp_t = lp;
          int size;
          int offset;
          int found = 0;
          lp_t = lp;
          //now lets see if its dest is in its own stack
          while(lp_t>=0){
              size = stack[lp_t-1];
              offset = stack[lp_t];
              //if true the val is for it
              if(m_addr < offset && m_addr > offset-size){
                stack[stack[lp_t-2]+2] = getData(t);
    						stack[stack[lp_t-2]+1]-=1;
                break;
              }
            lp_t-=3;
          }
        }else{
          sp = stack[ADDRASABLE_SPACE-2];
          pc = stack[ADDRASABLE_SPACE-1];
        }
        tick_type[cpu_num][0]++;
        free(t);
        break;
      }
      case MAD:
      {
        int num_dest,doffset;
        if(stack[sp+4]==code_expansion){
					num_dest = stack[sp+6+(stack[sp+5]*3)];
					doffset = sp+6+(stack[sp+5]*3)+2+(4*(num_dest-1));
				}else{
					num_dest = stack[sp+6+stack[sp+5]];
					doffset = sp+6+stack[sp+5]+1+(2*(num_dest-1));
				}
        //int	num_dest = stack[sp+6+stack[sp+5]];
        if(num_dest>0){
          //int	doffset = sp+6+stack[sp+5]+1+(2*(num_dest-1));
          //printf("doffset %d\n",doffset);
          if(stack[doffset] == OUTPUT){
          }else{
            int  m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
              pack_and_sendMessage(core->routing_table[stack[doffset+1]],stack[doffset+1],OPR,MAD);
              pack_and_sendMessage(core->routing_table[stack[doffset+1]],stack[doffset+1],m_addr,MAD);
              bytes_in_out[cpu_num][1]+=2;
          }

          if(stack[sp+4]==code_expansion){stack[sp+6+(stack[sp+5]*3)]--;}
          else{stack[sp+6+stack[sp+5]]--;}

          tick_type[cpu_num][4]++;//added to com time

        }else{

          int pointed_to_removed_node=0;
          if(sp==stack[ADDRASABLE_SPACE-2]){
            pointed_to_removed_node=1;
          }

          //remove node
          if(sp == sp_top){
  					sp_top = sp+stack[sp+3];
  					int to = sp_top-1;
  					while(to+1 != sp){
  						stack[to] = STACK_UNDEFINED;
  						to--;
  					}
  					stack[lp] = STACK_UNDEFINED;
  					stack[lp-1] = STACK_UNDEFINED;
  					stack[lp-2] = STACK_UNDEFINED;
  					lp-=3;

  				}else{

  					int lp_tmp = lp;
            while(stack[lp_tmp-2] != sp){
                lp_tmp -= 3;
            }
  					int to = sp+stack[sp+3]-1;
  					int from = sp-1;
  					while(from != sp_top-1){
  						stack[to] = stack[from];
  						if(stack[to] == NODE_BEGIN_FLAG){//fix lp entry
  							stack[lp_tmp-2] = to;
  							stack[lp_tmp-1] = stack[lp_tmp+2];
  							stack[lp_tmp] = stack[lp_tmp+3];
  							lp_tmp += 3;
  						}
              if(stack[ADDRASABLE_SPACE-2]==from){stack[ADDRASABLE_SPACE-2]=to;}
  						to--;from--;
  					}
  					sp_top = to+1;
  					stack[lp] = STACK_UNDEFINED;
  					stack[lp-1] = STACK_UNDEFINED;
  					stack[lp-2] = STACK_UNDEFINED;
  					lp-=3;
  					while(to!=from){
  						stack[to] = STACK_UNDEFINED;
  						to--;
  					}
  				}

          if(pointed_to_removed_node==1){
            pc=LFN;
            sp=sp_top;
          }else{
            pc=stack[ADDRASABLE_SPACE-1];
            sp=stack[ADDRASABLE_SPACE-2];
          }

          tick_type[cpu_num][0]++;

        }
        break;
      }
      case IDLE:
        break;
      case SDOWN: //shift down **cant be interupted**
			{
				//remove lp and sp entry of node
        //shift down
				if(sp == sp_top){
					sp_top = sp+stack[sp+3];
					int to = sp_top-1;
					while(to+1 != sp){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					stack[lp-2] = STACK_UNDEFINED;
					lp-=3;

				}else{
					int lp_tmp = lp;
          while(stack[lp_tmp-2] != sp){
              lp_tmp -= 3;
          }
					int to = sp+stack[sp+3]-1;
					int from = sp-1;
					while(from != sp_top-1){
						stack[to] = stack[from];
						if(stack[to] == NODE_BEGIN_FLAG){//fix lp entry
							stack[lp_tmp-2] = to;
							stack[lp_tmp-1] = stack[lp_tmp+2];
							stack[lp_tmp] = stack[lp_tmp+3];
							lp_tmp += 3;
						}
						to--;from--;
					}
					sp_top = to+1;
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					stack[lp-2] = STACK_UNDEFINED;
					lp-=3;
					while(to!=from){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
				}
        sp=sp_top;
				pc = LFN;
        tick_type[cpu_num][1]++;
      	break;
			}
      case FND:
      {
        int num_dest = stack[sp+6+stack[sp+5]];
        if(num_dest>0){
          int doffset = sp+6+stack[sp+5]+1+(2*(num_dest-1));
          if(stack[doffset] == OUTPUT){
						printf("CPU %d OUTPUT %d\n",cpu_num,stack[sp+2]);
					}else{
            int m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
            if(stack[doffset+1]==cpu_num){

              int lp_t = lp;
              int size;
              int offset;
              int found = 0;
              lp_t = lp;
              //now lets see if its dest is in its own stack
              while(lp_t>=0){
                  size = stack[lp_t-1];
                  offset = stack[lp_t];
                  //if true the val is for it
                  if(m_addr < offset && m_addr > offset-size){
                    if((offset-m_addr-1)>= 8){
                      stack[stack[lp_t-2]+(offset-m_addr-1)+((offset-m_addr-1)/2-3)] = stack[sp+2];
                    }else{
                      stack[stack[lp_t-2]+(offset-m_addr-1)] = stack[sp+2];
                    }
                    stack[stack[lp_t-2]+1] -= 1;
                    break;
                  }
                lp_t-=3;
              }
          //    printf("core %d wrote %d to internal node\n",cpu_num,stack[sp+2]);
            }else{
              //printf("core %d sending %d to %d\n",cpu_num,stack[sp+2],stack[doffset+1]);
              //printf("raw offset %d : offset: %d\n",stack[sp+stack[sp+3]-1],m_addr);
              pack_and_sendMessage(core->routing_table[stack[doffset+1]],stack[doffset+1],m_addr,stack[sp+2]);
              bytes_in_out[cpu_num][1]++;
            }
         }
         stack[sp+6+stack[sp+5]]--;
         tick_type[cpu_num][2]++; //propogate result
        }else{
          pc = SDOWN;
          tick_type[cpu_num][0]++;
        }
        break;
      }
      case LFN:
        {

          //if(sp >= (ADDRASABLE_SPACE-6)){sp = sp_top;pc=COM;}
          int buff_size = getFifoSize(buffer);
        //  printf("core %d buff size %d\n",cpu_num,buff_size);
  //        printf("core %d BUFF size %d\n",cpu_num,buff_size);
      		if(buff_size>0){
            tick_type[cpu_num][4]++;
      			struct Message *m = popMessage(buffer);
            bytes_in_out[cpu_num][0]++;
      			if(oper==1){
      				if(next_op==MAD){
      					//look if node is in stack
      					int found = 0;
      					int lp_t = lp;
      					int size;
      					int offset;
      					int m_addr = getAddr(m);
      					while(lp_t>=0){
      							size = stack[lp_t-1];
      							offset = stack[lp_t];
      							//if true the val is for it
      							if(m_addr < offset && m_addr > offset-size){
      								if(stack[stack[lp_t-2]+4]==code_merge){
      								}else{
                        stack[ADDRASABLE_SPACE-2] = sp;
      									stack[ADDRASABLE_SPACE-1] = pc;
                        sp = stack[lp_t-2];
      									pc = next_op;
      								}
      								break;
      							}
      						lp_t-=3;
      					}
      					oper=0;
      				}else if(next_op == EXP){
      					if(getAddr(m)==OPR && getData(m)==EOM){
      						sendMessage(expand_buffer,m);
      						eom_count++;
      						if(eom_count == 2){
      							eom_count = 0;
      							oper = 0;
      							stack[ADDRASABLE_SPACE-1] = pc;
      							pc = next_op;
      						}
      					}else{
      						sendMessage(expand_buffer,m);
      					}
      				}
      			}else if(getAddr(m)==OPR){    //there are
      				if(m->dest != cpu_num){
      					int eom_c = 0;
      					int c;
      					int m_dest = m->dest;
      					if(getData(m) == EXP){
                  sendMessage(core->routing_table[m_dest],m);
                  bytes_in_out[cpu_num][1]++;
      						while(eom_c!=2){
                    free(m);
      							m = popMessage(buffer);
                    bytes_in_out[cpu_num][0]++;
                    bytes_in_out[cpu_num][1]++;
      							sendMessage(core->routing_table[m_dest],m);
      							if(getAddr(m)==OPR && getData(m)==EOM){eom_c++;}
      						}
      					}
      					else{ //MAD
      						sendMessage(core->routing_table[m_dest],m);
                  free(m);
      						m = popMessage(buffer);
      						sendMessage(core->routing_table[m_dest],m);
                  bytes_in_out[cpu_num][1]+=2;
                  bytes_in_out[cpu_num][0]++;
      					}

      				}else{
      						if(getData(m)==code_end){
      							pc=code_end;
      						}else{
                    oper = 1;
        						next_op = getData(m);
                  }
      				}

      			}else if(m->dest != cpu_num){
  //            printf("routing message\n");
      				sendMessage(core->routing_table[m->dest],m);
              bytes_in_out[cpu_num][1]++;
      			}else{ //this would be just a write
      				//check if any entries in lp holds the receiving node
      				int lp_t = lp;
      				int size;
      				int found=0;
      				int offset;
      				int m_addr = getAddr(m);
      				while(lp_t>=0){
      						size = stack[lp_t-1];
      						offset = stack[lp_t];
      						//if true the val is for it
      						if(m_addr < offset && m_addr > offset-size){
      							found=1;
      							break;
      						}
      					lp_t-=3;
      				}
      				if(found==1){
                int doffset;
      					if((offset-m_addr-1)>= 8){
      						doffset = stack[lp_t-2]+(offset-m_addr-1)+((offset-m_addr-1)/2-3);
      					}else{
      						doffset = stack[lp_t-2]+(offset-m_addr-1);
      					}
                stack[doffset] = getData(m);
      					stack[stack[lp_t-2]+1] -= 1; //reduce number of dependants by one
      				}else{
            /*    printf("not found\n");
                printf("message: %d %d %d\n",m->dest,getAddr(m),getData(m));
                printf("sp_top %d lp %d\n",sp_top,lp);
              for(int i = 0; i<=lp;i+=3){
                			printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
                			printf("CPU %d [%d][%d]\n",cpu_num,i+1,stack[i+1]);
                			printf("CPU %d [%d][%d]\n",cpu_num,i+2,stack[i+2]);
            		}
            		for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
            			printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
            		}///*/
                //return 0;
                //printf("core %d not found. tick: %d\n",cpu_num,ticks);
                if(m->repeated < 2000){
                  m->repeated++;
                  sendMessage(buffer,m);
                  bytes_in_out[cpu_num][1]++;
                }
      				}
      			}
      			free(m);
      		}else if(sp >= (ADDRASABLE_SPACE-6)){
            tick_type[cpu_num][0]++;
            sp = sp_top;
          /*  for(int i = 0; i<=lp;i+=3){
                  printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
                  printf("CPU %d [%d][%d]\n",cpu_num,i+1,stack[i+1]);
                  printf("CPU %d [%d][%d]\n",cpu_num,i+2,stack[i+2]);
            }
            for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
              printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
            }
            return 0;*/
          }else if(stack[sp+1]==0){
            tick_type[cpu_num][0]++;
            pc = stack[sp+4];
          }else if(stack[sp+1]<0){
            printf("ERROR: negative num dependant at sp: %d\n",sp);
            return 0;
          }else{
            tick_type[cpu_num][0]++;
            sp = sp + stack[sp+3];
            //printf("sp vs ADDRASABLE_SPACE - 5 -> %d vs %d\n",sp,ADDRASABLE_SPACE-5);
            //if(sp >= (ADDRASABLE_SPACE-6)){sp = sp_top;pc=COM;}
          }
  				break;
        }
      case SSU:
        {
          if(sp==-100){sp = core->code_size-1;}
          if(sp>0){ //adding nodes to stack
            if(core->PM[sp] == main_addr){ //check if the node is in main
        			//add to stack
        			sp_top--;
        			while(core->PM[sp-1] != NODE_BEGIN_FLAG){
        				stack[sp_top]=core->PM[sp];
        				sp_top--;sp--;
        			}
        			stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted

        			if(stack[sp_top+4]==code_expansion){
        				stack[lp+1] = core->PM[sp+3]-stack[sp_top+5]-stack[sp_top+6+(stack[sp_top+5]*3)]; //needs to be changed to pack size and offset it same
        			}else{
        				stack[lp+1] = core->PM[sp+3]-stack[sp_top+6+stack[sp_top+5]]; //needs to be changed to pack size and offset it same
        			}

        			stack[lp+2]= core->PM[sp];
        			lp+=3;sp--;
        			stack[sp_top] = core->PM[sp];
        			sp--;
        		}else{
        			while(core->PM[sp] != NODE_BEGIN_FLAG){sp--;}
        			sp--;
        		}
          }else{ //done traversing pm
            pc=LFN;
            sp = sp_top;
            lp--;
          }//*/
          tick_type[cpu_num][0]++;
          break;
        }
      default:
  			{
  				printf("CPU %d unrecognized operation [%d][%d]\n",cpu_num,sp,pc);
          for(int i = 0; i<=lp;i+=3){
                printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
                printf("CPU %d [%d][%d]\n",cpu_num,i+1,stack[i+1]);
                printf("CPU %d [%d][%d]\n",cpu_num,i+2,stack[i+2]);
          }
          for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
            printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
          }
  				return 0;
          break;
  			}
    }

    core->pc = pc;
    core->sp = sp;
    core->sp_top = sp_top;
    core->lp = lp;
    core->oper = oper;
    core->next_op = next_op;
    core->eom_count = eom_count;
    core->largest_offset = largest_offset;

    //core->stack = stack;
    //core->expand_buffer = expand_buffer;
  //  fprintf(buffer_size,"%d ",getFifoSize(buffer));
    cpu_num++;
    if(cpu_num==number_of_cores){
      cpu_num=0;
      ticks++;
  //    fprintf(buffer_size,"\n");
    }
  }


  //printf("NUM TICKS: %d\n",ticks);
  fprintf(tick_count,"%d\n",ticks);
  int c=0;
  for(int i=0;i<number_of_cores;i++){
  /*  c=0;
    fprintf(tick_types,"%d ",i);
    for(int j=0;j<5;j++){
      c+=tick_type[i][j];
      fprintf(tick_types,"%d ",tick_type[i][j]);
    }
    fprintf(tick_types,"\n");
    printf("core %d total %d\n",i,c);*/
    fprintf(tick_types,"%d %d %d %d %d %d\n",i,tick_type[i][0],tick_type[i][1],tick_type[i][2],tick_type[i][3],tick_type[i][4]);
    fprintf(bytes_inout,"%d %d %d\n",i,(bytes_in_out[i][0]*8),(bytes_in_out[i][1]*8));

  }

  return 1;
}




int main(int argc, char **argv)
{
    printf("\n***SIMULATION START***\n\n");
		NUM_CPU;
		MESSAGE = 0;
		GRAPH=0;
		int KG=0;
		int h=0;
		int n = 0;
		int NODE_NUM_MAX = 2;
    int many=0;
		int opt;
		while ((opt = getopt(argc, argv, "mhngK:")) != -1) {
             switch (opt) {
             case 'm':
                 many = 1;
                 break;
						 case 'h':
 								 h=1;
 								 break;
						 case 'n':
						 		 n = 1;
								 break;
						 case 'g':
						 		 GRAPH = 1;
								 break;
             default: /* '?' */
                 printf("Usage: %s [-m] [-g] [-h] [-n] num_cpu\n",argv[0]);
                 exit(EXIT_FAILURE);
             }
    }
		if(h==1){
			printf("Usage: ./sim [-m] [-n] [-g] [-h] num_cpu  (ex: ./sim 4 or ./sim -m 4)\n[-m]: Display all core messages\n[-n]: Display node details\n[-g]: Create graphs and save them to pdf (Requires gnuplot)\n[-h]: Display all options\n\n");
			return 0;
		}
		if (optind >= argc) {
             fprintf(stderr, "Expected argument!\nhint: add -h to see all options\n\n");
             exit(EXIT_FAILURE);
  	}else{
			NUM_CPU = atoi(argv[optind]);
		}

    if(NUM_CPU < 1){
			printf("NUM CPU %d\n",NUM_CPU);
			printf("YOU MUST HAVE AT LEAST 1 CPU\n");
			return 1;
    }
    double sq = sqrt(NUM_CPU);
    if(sq != (int)sq){
      printf("There must be NxN cores defined.\nex: 1,4,9,16\n");
      return 0;
    }


    //open files
    tick_count = fopen("Data/tick_count.txt","w");
    tick_types = fopen("Data/tick_by_type.txt","w");
    buffer_size = fopen("Data/buffer_size.txt","w");
    bytes_inout = fopen("Data/bytes_in_out.txt","w");

    int iter=1;
    if(many==1){iter=1000;}

    for(iter;iter>0;iter--){
      printf("iter %d\n",iter);

      buss = (struct FIFO**) malloc(NUM_CPU*sizeof(struct FIFO*));
      for(int i = 0; i<NUM_CPU;i++){
        buss[i] = create_FIFO();
      }

      //create cpu struct
      struct CPU *cpus[NUM_CPU];
      for(int i = 0; i<NUM_CPU; i++){
          struct CPU *cpu_t = (struct CPU*) malloc(sizeof(struct CPU));
          cpu_t->PM = (int *) malloc(ADDRASABLE_SPACE*sizeof(int));
          cpu_t->stack = (int *) malloc(ADDRASABLE_SPACE*sizeof(int));
          cpu_t->code_size = 0;
          cpu_t->pc = SSU;
          cpu_t->lp = 0;
          cpu_t->sp = -100;
          cpu_t->sp_top = ADDRASABLE_SPACE-5; //minuse 4 since the 3 p=latter are reserved for saving -1:pc -2:lp -3:sp
          cpu_t->routing_table = set_up_routing_table(i+1,buss);
          cpu_t->expand_buffer = create_FIFO();
          cpu_t->broadcast = create_FIFO();
          cpu_t->oper=0;
          cpu_t->next_op=-1;
          cpu_t->eom_count = 0;
          cpu_t->largest_offset = (code_size)+(code_size*(i));
          cpus[i] = cpu_t;
      }

      /**********************************************************************************************************/
      int num_nodes_to_make = 0;
      for(int i = 0; i<num_dict_entries; i++){
          num_nodes_to_make += dictionary[i][2];
      }
      num_nodes=num_nodes_to_make;

      //using random coloruing for all nodes (including those in subgraph)\
      //double fact 256 cant do iter: 966, 864, 849, 844, 683, 673, 547, 502, 484
      //cascade cant do iter: 820, 576, 509
    /*  if(iter==922 || iter==908 || iter==842 || iter==808|| iter==792 ){
       srand(iter+1);
      }else{
        srand(iter);
      }//*/

      srand(iter);
    //  srand(998);//820,576,509

      //resetting the seed to avoid same result.
      //srand(820);
    //  printf("total number of nodes: %d\n", num_nodes_to_make);
      int colouring_random[num_nodes_to_make];//holds node allocations to cpus.
      int counter = 0;
      for(int i = 0; i< num_nodes_to_make; i++){
          colouring_random[i] = rand() % NUM_CPU; //random allocation
          //printf("cpu rand[%d]: %d\n", i, colouring_random[i]);
          counter++;
      }


      //printf("counter: %d\n", counter);

      int i = 0;
      int node_counter = 0;
      int add;
      int j;
      int rand_cpu;
      int s,si;

      int func_offset = dictionary[0][0];
      int dict_ent = 0;
      //num_nodes_to_make--;
      while(num_nodes_to_make != 0){
        rand_cpu = colouring_random[node_counter];
        j = cpus[rand_cpu]->code_size;
        s = j+4;
        cpus[rand_cpu]->PM[j] = code[i]; //this is the new node flag
        cpus[rand_cpu]->PM[j+1] = code_size - i; //this is the MM offset
        j+=2;i++;
        add = i+5;
        for(i; i<add; i++){
          cpus[rand_cpu]->PM[j] = code[i];
          j++;
        }

        if(code[i-2]==code_expansion){
          add = i + (code[i-1]*2);
          int suggraph_offset = code[i+(code[i-1]*2)+1];
          for(i; i<add; i+=2){
            cpus[rand_cpu]->PM[j] = code[i];
            cpus[rand_cpu]->PM[j+1] = code[i+1];
            cpus[rand_cpu]->PM[j+2] = colouring_random[find_cpu_num(suggraph_offset,code[i+1])];
            j+=3;
          }
          add = i +1+ (code[i]*2);
          cpus[rand_cpu]->PM[j] = code[i];
          cpus[rand_cpu]->PM[j+1] = code[i+1];
          //printf("code[%d]: %d\ncode[%d]: %d\n",i,code[i],i+1,code[i+1]);
          i+=2;j+=2;
          //printf("add %d\n",add);
          for(i; i<add; i+=2){
            //printf("code[%d]: %d\ncode[%d]: %d\n",i,code[i],i+1,code[i+1]);
            cpus[rand_cpu]->PM[j] = code[i];
            cpus[rand_cpu]->PM[j+1] = colouring_random[find_cpu_num(func_offset,code[i])];
            cpus[rand_cpu]->PM[j+2] = code[i+1];
            cpus[rand_cpu]->PM[j+3] = colouring_random[find_cpu_num(suggraph_offset,code[i+1])];
            j+=4;
          }
        }else{
          add = i + code[i-1] + 1;
          for(i; i<add; i++){
            cpus[rand_cpu]->PM[j] = code[i];
            j++;
          }
          add = i + code[i-1];
          for(i; i<add; i++){
            cpus[rand_cpu]->PM[j] = code[i];
            //printf("num_nodes_to_make %d\n",num_nodes_to_make);
            //printf("code[%d]: %d\n",i,code[i]);
            cpus[rand_cpu]->PM[j+1] = colouring_random[find_cpu_num(func_offset,code[i])];
            j+=2;
          }
        }
        cpus[rand_cpu]->PM[s] = j - cpus[rand_cpu]->code_size;
        cpus[rand_cpu]->code_size += cpus[rand_cpu]->PM[s]+1; //+1 for 1 extra entry and +1 to be one cell past
        cpus[rand_cpu]->PM[j] = func_offset; //this is the function offset this node is placed at node_size-1
        if(i == (code_size-func_offset) && i>0){
          dict_ent++;
          func_offset = dictionary[dict_ent][0];
        }
        node_counter++;
        num_nodes_to_make--;
      }

      /**********************************************************************************************************/

      int program = run(cpus,NUM_CPU);
      if(program != 1){
        printf("PROGRAM FAILED\n");
        exit(0);
      }

      for(int i = 0; i<NUM_CPU; i++){
        free(buss[i]);
        free(cpus[i]->PM);
        free(cpus[i]->stack);
        free(cpus[i]->routing_table);
        free(cpus[i]->expand_buffer);
        free(cpus[i]->broadcast);
        free(cpus[i]);

      }
      free(buss);
    }

    //close files
    fclose(tick_count);
    fclose(tick_types);
    fclose(buffer_size);
    fclose(bytes_inout);

    /***********************/
    /**** Simulation end ***/
    /***********************/


		printf("\n***SIMULATION COMPLETE***\n\n");

		clock_t finish = clock();
		double elapsed = (double)(finish - BEGIN)/CLOCKS_PER_SEC;

    //pthread_mutex_destroy(&mem_lock);

		printf("TIME ELAPSED: %f\n\n", elapsed);

    //printf("%d AGP nodes created\n",list_index-1);

    return 0;
}

struct FIFO **set_up_routing_table(int cpu_num, struct FIFO **rt){
  struct FIFO **r_table = (struct FIFO**) malloc(NUM_CPU*sizeof(struct FIFO*));
  int N = sqrt(NUM_CPU);
  //printf("NxN: %d\n",N);
  int table[N][N];
  int num = 1;
  int x,y;
  int up,down,left,right;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      table[i][j] = num;
      if(num == cpu_num){
        x=i;y=j;
      }
      num++;
    }
  }
  if(x-1>-1){up = table[x-1][y];}
  if(x+1<N){down = table[x+1][y];}
  if(y-1>-1){left = table[x][y-1];}
  if(y+1<N){right = table[x][y+1];}
  num = 0;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      if(i<x){
        r_table[num] = rt[up-1];
        //printf("Num: %d -> %d\n",num,up-1);
      }
      else if(i>x){
        r_table[num] = rt[down-1];
        //printf("Num: %d -> %d\n",num,down-1);
      }
      else if(j<y){
        r_table[num] = rt[left-1];
        //printf("Num: %d -> %d\n",num,left-1);
      }
      else if(j>y){
        r_table[num] = rt[right-1];
        //printf("Num: %d -> %d\n",num,right-1);
      }
      else{
        r_table[num] = rt[cpu_num-1];
        //printf("Num: %d -> %d\n",num,cpu_num-1);
      }
      num++;
    }
  }
  return r_table;
}



int find_cpu_num(int func_offset, int dest_offset){

  if(dest_offset == -1){return 0;}
  int end = code_size - func_offset - dest_offset/4;
  //printf("end %d\n",dest_offset);
  int count=0;
  int i=1;
  while(i!=end){
    if(code[i]==NODE_BEGIN_FLAG){count++;}
    i++;
  }
  return count;
}

struct FIFO *create_FIFO(){
	struct FIFO *fifo = (struct FIFO*)malloc(sizeof(struct FIFO));
	fifo->front = fifo->back = NULL;
	fifo->size = 0;
	return fifo;
}

void sendMessageOnBuss(int cpu_num,struct Message *m){
  struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *m;
  for(int i=0;i<NUM_CPU;i++){
    if(i!=cpu_num-1){
      sendMessage(buss[i],new);
    }
  }
}

void pack_and_sendMessage(struct FIFO *fifo,int cpu_num,int addr, int data){
  struct Message* temp = (struct Message*)malloc(sizeof(struct Message));
  temp->addr = addr;
  temp->data = data;
  temp->next = NULL;
  temp->repeated=0;
  temp->dest = cpu_num;
  if(fifo->back == NULL){
		fifo->front = fifo->back = temp;
		fifo->size+=1;
	}else{
		fifo->back->next = temp;
		fifo->back = fifo->back->next;
		fifo->size+=1;
	}
}

void sendMessage(struct FIFO *fifo, struct Message *m){
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *m;
	if(fifo->back == NULL){
		fifo->front = fifo->back = new;
		fifo->size+=1;
	}else{
		fifo->back->next = new;
		fifo->back = fifo->back->next;
		fifo->size+=1;
	}
  //free(m);
}
struct Message *peekMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *fifo->front;
	new->next = NULL;
	return new;
}
void removeMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		//return NULL;
		printf("NO MESSAGES TO REMOVE");
	}else{
		struct Message *m = fifo->front;
		fifo->front = fifo->front->next;
		m->next = NULL;

		if(fifo->front == NULL)
			fifo->back = NULL;

		fifo->size-=1;
		free(m);
	}
}
struct Message *popMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}else{
    struct Message *new = (struct Message*)malloc(sizeof(struct Message));
  	*new = *fifo->front;
  	new->next = NULL;

  	struct Message *remove = fifo->front;
  	fifo->front = fifo->front->next;
  	remove->next = NULL;
  	free(remove);

  	if(fifo->front == NULL)
  		fifo->back = NULL;

  	fifo->size-=1;
  	return new;
  }

}

int getFifoSize(struct FIFO *fifo){
	return fifo->size;
}

struct Message*  Message_packing(int cpu_num,int addr, int data){
     struct Message* temp = (struct Message*)malloc(sizeof(struct Message));
     temp->addr = addr;
     temp->data = data;
     temp->next = NULL;
     temp->repeated=0;
     temp->dest = cpu_num;
     return temp;
}

int getCpuNum(struct Message *message){
   return (int) ( message->addr >> 26 ) & 0x0000003F;
}

int getRW(struct Message *message){
   return (int) ( message->addr >> 25 ) & 0x00000001;
}

int getAddr(struct Message *message){
   //return (int) message->addr & 0x0001FFFF;
   return message->addr;
}

int getData(struct Message *message){
   return message->data;
}
