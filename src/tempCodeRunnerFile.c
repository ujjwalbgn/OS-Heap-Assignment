// Written by Ujjwal Bajagain
// ID 1001643752

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>   // used for memcpy 

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

int temp_size;   // initializing temp_size which will be used to store size of best fit and worst fit 

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit DONE    
 * \TODO Implement Best Fit DONE
 * \TODO Implement Worst Fit DONE
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
//    printf("TODO: Implement best fit here\n");
    struct _block *best = NULL;
    temp_size = 10000;      
    
    while (curr)
    {
        *last = curr;
        if ((curr -> free == true)  && (curr->size >= size))
        {
            if(temp_size >= ((curr->size)-size))
            {
                best = curr;
                temp_size = (curr->size) - size;
            }
        }
        curr = curr -> next;
    }

    if (best != NULL)
    {
        curr = best;
    }
    
#endif

#if defined WORST && WORST == 0
    struct _block *worst = NULL;
    temp_size = 0;
    while (curr)
    {
        if ((curr -> free == true)  && (curr->size >= size))
        {
            if(temp_size < ((curr->size)-size))
            {
                worst = curr;
                temp_size = (curr->size) - size;
            }
        }
        curr = curr -> next;
    }
    if (worst!= NULL )
    {
        curr = worst;
    }

#endif

#if defined NEXT && NEXT == 0
//    printf("TODO: Implement next fit here\n");
    if( *last != NULL)
    {
        curr = *last;
        curr = curr->next;
    }

    while (curr && !(curr->free && curr->size >= size))
    {
        *last = curr;
        curr = curr->next;
    }
    
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
    num_blocks++;
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

    if( max_heap < size)
    {
        max_heap += size;  // As the heap grows the max size of the heap also increases
    }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr; 
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   num_mallocs++;     // for every malloc the num_mallocs is incremented. 
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);
   num_requested += size;    // the requested 


   /* TODO: Split free _block if possible */ 

   if (next && next->size > size)
  {
       num_splits++;
       num_blocks++;
       u_int8_t * newptr;
       newptr = (u_int8_t *) next + next->size + sizeof(struct _block);
       struct _block * temp = next->next;
       newptr = newptr- (next->size -size);
       next -> next = (struct _block *) newptr;
       next->next->next = temp;
       next->next->size = next->size - size - sizeof(struct _block);
       next->next->free = 1; 

   }
   
   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_grows++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
   
   /* TODO: Coalesce free _blocks if needed DONE*/ 
    while (curr != NULL && (curr->next )!= NULL)
    {
        // if (curr && curr->free && curr->next && curr->next->free)

        if ( curr->free && curr->next->free)
      {
         curr->size = curr->size + curr->next->size + sizeof(struct _block);
         curr->next = curr->next->next;
      }
      curr = curr->next;

        num_coalesces++;

    }
    num_blocks--;
    num_reuses++;
    
}

void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
    return ptr;
}
void *realloc(void *oldptr, size_t size)
{
    void *ptr = malloc(size);
    memcpy(ptr, oldptr, size);
    return ptr;
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/