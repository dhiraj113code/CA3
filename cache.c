/*
 * 
 * cache.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
void init_cache()
{

  /* initialize the cache, and cache statistics data structures */

  //Unified Cache
  //----------------------------------------------------------
  unsigned n_blocks, block_offset, index_size;

  block_offset = LOG2(cache_block_size);
  if(!cache_split)
  {
     //Unified Cache
     c1.size = cache_usize;
     c1.associativity = cache_assoc;
     n_blocks = cache_usize/cache_block_size;
     c1.n_sets = n_blocks/c1.associativity;
     index_size = LOG2(c1.n_sets) + block_offset;
     c1.index_mask = (1<<index_size) - 1;
     c1.index_mask_offset = block_offset;
  }
  else
  {
     //Data Cache
     c1.size = cache_dsize;
     c1.associativity = cache_assoc;
     n_blocks = cache_dsize/cache_block_size;
     c1.n_sets = n_blocks/c1.associativity;
     index_size = LOG2(c1.n_sets) + block_offset;
     c1.index_mask = (1<<index_size) - 1;
     c1.index_mask_offset = block_offset;

     //Instruction Cache
     c2.size = cache_isize;
     c2.associativity = cache_assoc;
     n_blocks = cache_isize/cache_block_size;
     c2.n_sets = n_blocks/c2.associativity;
     index_size = LOG2(c2.n_sets) + block_offset;
     c2.index_mask = (1<<index_size) - 1;
     c2.index_mask_offset = block_offset;
  }

  //Printing Initialized output
  printf("-----------------------------------------------\n");
  printf("Cache 1:\number of sets = %d\nindex_size = %d\nMask = %d\nMask_offset = %d\n", c1.n_sets, index_size, c1.index_mask, c1.index_mask_offset);
  printf("Cache 2:\nnumber of sets = %d\nindex_size = %d\nMask = %d\nMask_offset = %d\n", c2.n_sets, index_size, c2.index_mask, c2.index_mask_offset);
  printf("-----------------------------------------------\n");

  //Dynamically allocating memory for LRU head, LRU tail and contents arrays
  c1.LRU_head = (Pcache_line*)malloc(sizeof(Pcache_line)*c1.n_sets);
  c1.LRU_tail = (Pcache_line*)malloc(sizeof(Pcache_line)*c1.n_sets);
  c1.set_contents = (int*)malloc(sizeof(int)*c1.n_sets);

  if(cache_split)
  {
     c2.LRU_head = (Pcache_line*)malloc(sizeof(Pcache_line)*c2.n_sets);
     c2.LRU_tail = (Pcache_line*)malloc(sizeof(Pcache_line)*c2.n_sets);
     c2.set_contents = (int*)malloc(sizeof(int)*c2.n_sets);
  }

  if(c1.LRU_head == NULL || c1.LRU_tail == NULL || c1.set_contents == NULL)
     {printf("error : Memory allocation failed for C1 LRU_head, LRU_tail\n"); exit(-1);}
  if(c2.LRU_head == NULL || c2.LRU_tail == NULL || c2.set_contents == NULL)
     {printf("error : Memory allocation failed for C2 LRU_head, LRU_tail\n"); exit(-1);}

  //Initializing set_contents
  int i;
  for(i = 0; i < c1.n_sets; i++)
     c1.set_contents[i] = 0;
  for(i = 0; i < c2.n_sets; i++)
     c2.set_contents[i] = 0;
  
}
/************************************************************/

/************************************************************/
void perform_access(unsigned addr, unsigned access_type)
{

/* handle an access to the cache */
int index_size;
unsigned int index, tag;
if(cache_split && access_type == INSTRUCTION_LOAD_REFERENCE) //Only Loads
{
   index_size = LOG2(c2.n_sets) + c2.index_mask_offset;
   index = (addr & c2.index_mask) >> c2.index_mask_offset;
   tag = addr >> index_size;

   UpAccessStats(access_type);
   if(c2.LRU_head[index] == NULL) //Miss with no Replacement
   {
      UpMissStats(access_type);
      UpFetchStats(access_type);
      c2.LRU_head[index] = (Pcache_line)malloc(sizeof(cache_line));
      c2.LRU_head[index]->tag = tag;
      c2.LRU_head[index]->dirty = FALSE;
      c2.LRU_head[index]->LRU_next = (Pcache_line)NULL;
      c2.LRU_head[index]->LRU_prev = (Pcache_line)NULL;
      c2.set_contents[index] = 1;
   }
   else if(!search(c2.LRU_head[index], tag)) //Miss with Replacement
   {
      UpMissStats(access_type);
      UpFetchStats(access_type);
  
      if(c2.set_contents[index] < c2.associativity)
      {
        //insert
      }
      else
      {
         //While evicting
         //delete and insert
         cache_stat_inst.replacements++;
      }

      //Settting the new tag
      c2.LRU_head[index]->tag = tag;
      c2.LRU_head[index]->dirty = FALSE;
   }
   //Else do nothing on a write hit
}
else
{
  index_size = LOG2(c1.n_sets) + c1.index_mask_offset;
  index = (addr & c1.index_mask) >> c1.index_mask_offset;
  tag = addr >> index_size;

  if(DEBUG) printf("debug_info : For addr = %d, tag = %d, index = %d\n", addr, tag, index);

  UpAccessStats(access_type);
  if(c1.LRU_head[index] == NULL) //Miss with no Replacement
  {
     UpMissStats(access_type);
     UpFetchStats(access_type);
     if(access_type == DATA_LOAD_REFERENCE || access_type == INSTRUCTION_LOAD_REFERENCE || cache_writealloc)
     {
        c1.LRU_head[index] = (Pcache_line)malloc(sizeof(cache_line));
        c1.LRU_head[index]->tag = tag;
        c1.LRU_head[index]->dirty = FALSE;
        if(access_type == DATA_STORE_REFERENCE && cache_writeback)
           c1.LRU_head[index]->dirty = TRUE;
     }
  }
  else if(c1.LRU_head[index]->tag != tag) //Miss with Replacement
  {
     UpMissStats(access_type);
     if(access_type == DATA_LOAD_REFERENCE || access_type == INSTRUCTION_LOAD_REFERENCE || cache_writealloc)
     {
        UpFetchStats(access_type);
        //While evicting
        UpReplaceStats(access_type);

        //If dirty
        if(c1.LRU_head[index]->dirty)
           cache_stat_data.copies_back += cache_block_size/WORD_SIZE; 

        //Settting the new tag
        c1.LRU_head[index]->tag = tag;
        c1.LRU_head[index]->dirty = FALSE;
        if(access_type == DATA_STORE_REFERENCE && cache_writeback)
           c1.LRU_head[index]->dirty = TRUE;
     }
  }
  else //Hit
  {
     if(access_type == DATA_STORE_REFERENCE && cache_writeback)
        c1.LRU_head[index]->dirty = TRUE; 
  }
}
}
/************************************************************/

/************************************************************/
void flush()
{
  /* flush the cache */
  int i;
  for(i = 0; i < c1.n_sets; i++)
  {
     if(c1.LRU_head[i] != NULL)
     {
        if(c1.LRU_head[i]->dirty)
           cache_stat_data.copies_back += cache_block_size/WORD_SIZE;
     }
  }
}

/************************************************************/

/************************************************************/
void delete(Pcache_line *head, Pcache_line *tail, Pcache_line item)
{
  if (item->LRU_prev) {
    item->LRU_prev->LRU_next = item->LRU_next;
  } else {
    /* item at head */
    *head = item->LRU_next;
  }

  if (item->LRU_next) {
    item->LRU_next->LRU_prev = item->LRU_prev;
  } else {
    /* item at tail */
    *tail = item->LRU_prev;
  }
}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(Pcache_line *head, Pcache_line *tail, Pcache_line item)
{
  item->LRU_next = *head;
  item->LRU_prev = (Pcache_line)NULL;

  if (item->LRU_next)
    item->LRU_next->LRU_prev = item;
  else
    *tail = item;

  *head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("Cache Settings:\n");
  if (cache_split) {
    printf("\tSplit I- D-cache\n");
    printf("\tI-cache size: \t%d\n", cache_isize);
    printf("\tD-cache size: \t%d\n", cache_dsize);
  } else {
    printf("\tUnified I- D-cache\n");
    printf("\tSize: \t%d\n", cache_usize);
  }
  printf("\tAssociativity: \t%d\n", cache_assoc);
  printf("\tBlock size: \t%d\n", cache_block_size);
  printf("\tWrite policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("\tAllocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("*** CACHE STATISTICS ***\n");
  printf("  INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf("  DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf("  TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
  printf(" Store instructions = %d\n", cache_stat_data.write_backs);
}
/************************************************************/


void UpMissStats(unsigned access_type)
{
switch(access_type)
{
   case DATA_LOAD_REFERENCE:
   case DATA_STORE_REFERENCE:
      cache_stat_data.misses++;
      break;
   case INSTRUCTION_LOAD_REFERENCE:
      cache_stat_inst.misses++;
      break;
   defualt:
      printf("error : Unrecognized access_type in Update miss stats\n");
      exit(-1);
     break;
}
}

void UpAccessStats(unsigned access_type)
{
switch(access_type)
{
   case DATA_LOAD_REFERENCE:
      cache_stat_data.accesses++;
      break;
   case DATA_STORE_REFERENCE:
      cache_stat_data.accesses++;
      cache_stat_data.write_backs++;
      break;
   case INSTRUCTION_LOAD_REFERENCE:
      cache_stat_inst.accesses++;
      break;
   defualt:
      printf("error : Unrecognized access_type in Update miss stats\n");
      exit(-1);
     break;
}
}

void UpReplaceStats(unsigned access_type)
{
switch(access_type)
{
   case DATA_LOAD_REFERENCE:
   case DATA_STORE_REFERENCE:
      cache_stat_data.replacements++;
      break;
   case INSTRUCTION_LOAD_REFERENCE:
      cache_stat_inst.replacements++;
      break;
   defualt:
      printf("error : Unrecognized access_type in Update miss stats\n");
      exit(-1);
     break;
}
}

void UpFetchStats(unsigned access_type)
{
switch(access_type)
{
   case DATA_LOAD_REFERENCE:
      cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE;
      break;
   case DATA_STORE_REFERENCE:
      cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE;
      break;
   case INSTRUCTION_LOAD_REFERENCE:
      cache_stat_inst.demand_fetches += cache_block_size/WORD_SIZE;
      break;
   defualt:
      printf("error : Unrecognized access_type in update fetch stats\n");
      exit(-1);
     break;
}
}


//Search whether tag is present in the double linked list cache line c
int search(Pcache_line c, unsigned tag)
{
   if(c == NULL)
   {
      printf("error : Searching an unintialized cache line\n");
      exit(-1);
   }
   else
   {
     do
     {
        if(c->tag == tag)
           return TRUE; 
     } while(c->LRU_next != NULL);
     return FALSE;
   }
}
