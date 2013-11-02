
/*
 * cache.h
 * 
 * Donald Yeung
 */


#define TRUE 1
#define FALSE 0

/* default cache parameters--can be changed */
#define WORD_SIZE 4
#define WORD_SIZE_OFFSET 2
#define DEFAULT_CACHE_SIZE (8 * 1024)
#define DEFAULT_CACHE_BLOCK_SIZE 16
#define DEFAULT_CACHE_ASSOC 1
#define DEFAULT_CACHE_WRITEBACK TRUE
#define DEFAULT_CACHE_WRITEALLOC TRUE

/* constants for settting cache parameters */
#define CACHE_PARAM_BLOCK_SIZE 0
#define CACHE_PARAM_USIZE 1
#define CACHE_PARAM_ISIZE 2
#define CACHE_PARAM_DSIZE 3
#define CACHE_PARAM_ASSOC 4
#define CACHE_PARAM_WRITEBACK 5
#define CACHE_PARAM_WRITETHROUGH 6
#define CACHE_PARAM_WRITEALLOC 7
#define CACHE_PARAM_NOWRITEALLOC 8
#define PARAM_DEBUG 9

#define DATA_LOAD_REFERENCE 0
#define DATA_STORE_REFERENCE 1
#define INSTRUCTION_LOAD_REFERENCE 2

#define DEFAULT_DEBUG FALSE

/* structure definitions */
typedef struct cache_line_ {
  unsigned tag;
  int dirty;

  struct cache_line_ *LRU_next;
  struct cache_line_ *LRU_prev;
} cache_line, *Pcache_line;

typedef struct cache_ {
  int size;			/* cache size in words*/
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  unsigned index_mask;		/* mask to find cache index */
  int index_mask_offset;	/* number of zero bits in mask */
  Pcache_line *LRU_head;	/* head of LRU list for each set */
  Pcache_line *LRU_tail;	/* tail of LRU list for each set */
  int *set_contents;		/* number of valid entries in set */
} cache, *Pcache;

typedef struct cache_stat_ {
  int accesses;			/* number of memory references */
  int misses;			/* number of cache misses */
  int replacements;		/* number of misses that cause replacments */
  int demand_fetches;		/* number of fetches */
  int copies_back;		/* number of write backs */
  int write_backs;
} cache_stat, *Pcache_stat;


/* function prototypes */
void set_cache_param();
void init_cache();
void perform_access(unsigned, unsigned);
void flush();
void delete(Pcache_line *, Pcache_line *, Pcache_line);
void insert(Pcache_line *, Pcache_line *, Pcache_line);
void dump_settings();
void print_stats();


/* macros */
#define LOG2(x) ((int)( log((double)(x)) / log(2) ))

void UpMissStats(unsigned access_type);
void UpAccessStats(unsigned access_type);
void UpReplaceStats(unsigned access_type);
int search(Pcache_line c, unsigned tag, Pcache_line *hitAt);
Pcache_line allocateCL(unsigned tag);
int search2(Pcache_line head, Pcache_line tail, unsigned tag, Pcache_line *hitAt);
void printCL(Pcache_line c_line);
void PrintICache();
