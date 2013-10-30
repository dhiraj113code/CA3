
/*
 * main.h
 * 
 * Donald Yeung
 */


#define TRACE_DATA_LOAD 0
#define TRACE_DATA_STORE 1
#define TRACE_INST_LOAD 2

#define PRINT_INTERVAL 100000

void parse_args(int, char **);
void play_trace(FILE *);
int read_trace_element(FILE *, unsigned *, unsigned *);

