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
static int debug = DEFAULT_DEBUG;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

//
static FILE *cacheLog;

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
   case PARAM_DEBUG:
    debug = TRUE;
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
  unsigned c1_n_blocks, c2_n_blocks, block_offset, mask_size;

  block_offset = LOG2(cache_block_size);
  if(!cache_split)
  {
     //Unified Cache
     c1.size = cache_usize;
     c1.associativity = cache_assoc;
     c1_n_blocks = cache_usize/cache_block_size;
     c1.n_sets = c1_n_blocks/c1.associativity;
     mask_size = LOG2(c1.n_sets) + block_offset;
     c1.index_mask = (1<<mask_size) - 1;
     c1.index_mask_offset = block_offset;
  }
  else
  {
     //Data Cache
     c1.size = cache_dsize;
     c1.associativity = cache_assoc;
     c1_n_blocks = cache_dsize/cache_block_size;
     c1.n_sets = c1_n_blocks/c1.associativity;
     mask_size = LOG2(c1.n_sets) + block_offset;
     c1.index_mask = (1<<mask_size) - 1;
     c1.index_mask_offset = block_offset;

     //Instruction Cache
     c2.size = cache_isize;
     c2.associativity = cache_assoc;
     c2_n_blocks = cache_isize/cache_block_size;
     c2.n_sets = c2_n_blocks/c2.associativity;
     mask_size = LOG2(c2.n_sets) + block_offset;
     c2.index_mask = (1<<mask_size) - 1;
     c2.index_mask_offset = block_offset;
  }

  //Printing Initialized output
  if(debug)
  {
     printf("-----------------------------------------------\n");
     printf("Cache 1:\nnumber of blocks = %d\nnumber of sets = %d\nmask_size = %d\nMask = %d\nMask_offset = %d\n", c1_n_blocks, c1.n_sets, mask_size, c1.index_mask, c1.index_mask_offset);
     if(cache_split)
        printf("Cache 2:\nnumber of blocks = %d\nnumber of sets = %d\nmask_size = %d\nMask = %d\nMask_offset = %d\n", c2_n_blocks, c2.n_sets, mask_size, c2.index_mask, c2.index_mask_offset);
     printf("-----------------------------------------------\n");
  }

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

  if(cache_split) if(c2.LRU_head == NULL || c2.LRU_tail == NULL || c2.set_contents == NULL)
     {printf("error : Memory allocation failed for C2 LRU_head, LRU_tail\n"); exit(-1);}

  //Initializing set_contents
  int i;
  for(i = 0; i < c1.n_sets; i++)
  {
     c1.set_contents[i] = 0;
     c1.LRU_head[i] = (Pcache_line)NULL;
     c1.LRU_tail[i] = (Pcache_line)NULL;
  }
  for(i = 0; i < c2.n_sets; i++)
  {
     c2.set_contents[i] = 0;
     c2.LRU_head[i] = (Pcache_line)NULL;
     c2.LRU_tail[i] = (Pcache_line)NULL;
  }

  if(debug)
  {
     cacheLog = fopen("cache.log", "w");
     if(cacheLog == NULL) {printf("error : Unable to create cache.log file\n"); exit(-1);}
  }
}
/************************************************************/

/************************************************************/
void perform_access(unsigned addr, unsigned access_type)
{

/* handle an access to the cache */
int mask_size;
unsigned int index, tag;
Pcache_line c_line, hitAt;
if(cache_split && access_type == INSTRUCTION_LOAD_REFERENCE) //Instruction Loads
{
   mask_size = LOG2(c2.n_sets) + c2.index_mask_offset;
   index = (addr & c2.index_mask) >> c2.index_mask_offset;
   tag = addr >> mask_size;

   if(debug) fprintf(cacheLog, "addr = %x, index = %d, tag = %x -- ", addr, index, tag);

   UpAccessStats(access_type);

   if(c2.LRU_head[index] == NULL) //Miss with no Replacement
   {
      UpMissStats(access_type);
      c_line = allocateCL(tag);
      cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE; //Memory Fetch
      insert(&c2.LRU_head[index], &c2.LRU_tail[index], c_line);
      c2.set_contents[index]++;
   }
   else if(!search(c2.LRU_head[index], tag, &hitAt)) //Miss with Replacement
   //else if(!search2(c2.LRU_head[index], c2.LRU_tail[index], tag, &hitAt))
   {
      UpMissStats(access_type);
      c_line = allocateCL(tag);
      cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE; //Memory Fetch
  
      if(c2.set_contents[index] < c2.associativity)
      {
        //Inserting the cache line
        insert(&c2.LRU_head[index], &c2.LRU_tail[index], c_line);
        c2.set_contents[index]++; 
      }
      else
      {
         delete(&c2.LRU_head[index], &c2.LRU_tail[index], c2.LRU_tail[index]);
         insert(&c2.LRU_head[index], &c2.LRU_tail[index], c_line);
         //While evicting
         cache_stat_inst.replacements++;
      }
   }
   else //On a hit
   {
      if(debug) fprintf(cacheLog, "Is a hit\n");

      //LRU Implementation on a hit
      delete(&c2.LRU_head[index], &c2.LRU_tail[index], hitAt);
      insert(&c2.LRU_head[index], &c2.LRU_tail[index], hitAt);
   }
   //if(debug) printCL(c2.LRU_head[index]);
   if(debug) PrintICache();
}
else
{
  mask_size = LOG2(c1.n_sets) + c1.index_mask_offset;
  index = (addr & c1.index_mask) >> c1.index_mask_offset;
  tag = addr >> mask_size;

  //if(debug) fprintf(cacheLog, "debug_info : For addr = %d, tag = %d, index = %d\n", addr, tag, index);

  UpAccessStats(access_type);

  if(c1.LRU_head[index] == NULL) //Miss with no Replacement
  {
     UpMissStats(access_type);
     if(access_type == DATA_LOAD_REFERENCE || access_type == INSTRUCTION_LOAD_REFERENCE || cache_writealloc)
     {
        cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE; //Memory fetch
        c_line = allocateCL(tag);
        setifDirty(access_type, c_line);
        insert(&c1.LRU_head[index], &c1.LRU_tail[index], c_line);
        c1.set_contents[index]++;
     }
     else //write no allocate cache and DATA_STORE REFERENCE
     {
        cache_stat_data.copies_back++; //Only a single word is copied back
     }
  }
  else if(!search(c1.LRU_head[index], tag, &hitAt)) //Miss with Replacement
  //else if(!search2(c1.LRU_head[index], c1.LRU_tail[index], tag, &hitAt))
  {
     UpMissStats(access_type);
     if(access_type == DATA_LOAD_REFERENCE || access_type == INSTRUCTION_LOAD_REFERENCE || cache_writealloc)
     {
        cache_stat_data.demand_fetches += cache_block_size/WORD_SIZE; //Memory fetch

        //Creating the cache_line to be inserted
        c_line = allocateCL(tag);
        setifDirty(access_type, c_line);

        if(c1.set_contents[index] < c1.associativity)
        {
           //Inserting the cache line
           insert(&c1.LRU_head[index], &c1.LRU_tail[index], c_line);
           c1.set_contents[index]++;
        }
        else //While evicting
        {
           UpReplaceStats(access_type);
           //If dirty
           if(c1.LRU_tail[index]->dirty)
              cache_stat_data.copies_back += cache_block_size/WORD_SIZE;

           delete(&c1.LRU_head[index], &c1.LRU_tail[index], c1.LRU_tail[index]);
           insert(&c1.LRU_head[index], &c1.LRU_tail[index], c_line);
        }
     }
     else //write no allocate cache and DATA_STORE REFERENCE
     {
        cache_stat_data.copies_back++; //Only a single word is copied back
     }
  }
  else //Hit
  {
     //LRU Implementation on a hit
     delete(&c1.LRU_head[index], &c1.LRU_tail[index], hitAt);
     insert(&c1.LRU_head[index], &c1.LRU_tail[index], hitAt);
     setifDirty(access_type, hitAt);
  }
}
}
/************************************************************/

/************************************************************/
void flush()
{
  /* flush the cache */
  int i;
  Pcache_line c_line, n_line;
  //Only flushing c1 is sufficient as IC is never dirty
  for(i = 0; i < c1.n_sets; i++)
  {
     c_line = c1.LRU_head[i]; 
     if(c_line != NULL)
     {
        if(c_line->dirty)
           cache_stat_data.copies_back += cache_block_size/WORD_SIZE;
        while(c_line->LRU_next != NULL)
        {
           n_line = c_line->LRU_next;
           if(n_line->dirty)
              cache_stat_data.copies_back += cache_block_size/WORD_SIZE;
           c_line = n_line;
        
        }
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
  //printf(" Store instructions = %d\n", cache_stat_data.write_backs);
}
/************************************************************/


void UpMissStats(unsigned access_type)
{
if(debug) fprintf(cacheLog, "Is a miss\n");
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

//Search whether tag is present in the double linked list cache line c
int search(Pcache_line c, unsigned tag, Pcache_line *hitAt)
{
   Pcache_line n;
   if(c == NULL)
   {
      printf("error : Searching an unintialized cache line\n");
      exit(-1);
   }
   else
   {
      *hitAt = (Pcache_line)NULL;
      if(c->tag == tag)
      {
         *hitAt = c;
         return TRUE;
      }
      else
      { 
         while(c->LRU_next != NULL)
         {
            n = c->LRU_next;
            if(n->tag == tag)
            {
               *hitAt = n;
               return TRUE;
            }
            c = n;
         }
      }
      return FALSE;
   }
}


int search2(Pcache_line head, Pcache_line tail, unsigned tag, Pcache_line *hitAt)
{
   Pcache_line c_line, n_line;
   if(head == NULL || tail == NULL)
   {
      printf("error : Search2 head or tail is not NULL\n");
      exit(-1);
   }
   else
   {
      *hitAt = (Pcache_line)NULL;
      c_line = head;
      while(TRUE)
      {
         if(c_line->tag == tag)
         {
            *hitAt = c_line;
            return TRUE;
         }
         else
         {
            n_line = c_line;
            c_line = n_line->LRU_next;
         }
         if(c_line == tail) break;
      }
      return FALSE;
   }
}

//Allocate cache line

Pcache_line allocateCL(unsigned tag)
{
   Pcache_line c_line;
   c_line = (Pcache_line)malloc(sizeof(cache_line));
   c_line->tag = tag;
   c_line->dirty = FALSE;
   c_line->LRU_next = (Pcache_line)NULL;
   c_line->LRU_prev = (Pcache_line)NULL;
   return c_line;
}

//Debug functions
void printCL(Pcache_line c_line)
{
Pcache_line n_line;
while(c_line)
{
   fprintf(cacheLog, "|%x|", c_line->tag);
   n_line = c_line->LRU_next;
   c_line = n_line;
}
fprintf(cacheLog, "\n");
}

void PrintICache()
{
   int i;
   fprintf(cacheLog, "*********************************************************************\n");
   for(i = 0; i < c2.n_sets; i++)
   {
      if(c2.LRU_head[i] != NULL)
      {
         fprintf(cacheLog, "Line %d : ", i);
         printCL(c2.LRU_head[i]);
      }
   }
   fprintf(cacheLog, "*********************************************************************\n");
}



void setifDirty(unsigned access_type, Pcache_line c_line)
{
if(access_type == DATA_STORE_REFERENCE)
{
   if(cache_writeback)
      c_line->dirty = TRUE;
   else
      cache_stat_data.copies_back++; //Only a single word is copied back in write through
}
}
