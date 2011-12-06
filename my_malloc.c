#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the 
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to my_malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */


void* my_malloc(size_t size)
{
  int needed = size + sizeof(metadata_t);

  fprintf(stderr, "Requested size: %d bytes\n", size);
  fprintf(stderr, " Metadata size: %d bytes\n", sizeof(metadata_t));
  fprintf(stderr, "   Size needed: %d bytes\n", needed);

  if (needed > 2048) return NULL; 
  if (!heap) init_heap();

  int index = get_index(size);

  fprintf(stderr, "Index of freelist: %d\n", index);
  
  if (freelist[index]) {
    metadata_t *current = freelist[index];
    metadata_t *next = freelist[index]->next;
    
    current->in_use = 1;
    current->next = NULL;
    current->prev = NULL;

    if (next) {
      next->prev = NULL;
      freelist[index] = next;
    } else {
      freelist[index] = NULL;
    }
    
    void *repos = (void *) ((char *) current + sizeof(metadata_t));
    
    return repos;
  }

  int available = index;
  while (!freelist[available]) {
    available++;
  }

  fprintf(stderr, "Next available free memory: %d\n", available);

  metadata_t *current, *new;
  while (available != index) {
    current = freelist[available];
    current->size /= 2;

    new = (metadata_t *) ((char *) current + current->size);
    new->size = current->size;

    if (freelist[available]->next) {
      freelist[available] = freelist[available]->next;
      freelist[available]->prev = NULL;
    } else {
      freelist[available] = NULL;
    }
    --available;

    current->next = new;
    new->prev = current;
    freelist[available] = current;
  }

  metadata_t *ret_meta = freelist[index];
  freelist[index] = freelist[index]->next;
  freelist[index]->prev = NULL;
  ret_meta->next = NULL;

  void *repos = (void *) ((char *) ret_meta + sizeof(metadata_t));

  return repos;
}

void init_heap() {
  heap = my_sbrk(SBRK_SIZE);
  freelist[7] = (metadata_t *) heap;
  freelist[7]->in_use = 0;
  freelist[7]->size = 2048;
  freelist[7]->next = NULL;
  freelist[7]->prev = NULL; 
}

int get_index(size_t needed) {
  int index = 0;
  int memory = 16;
  while (memory < needed) {
    memory *= 2;
    index++;
  }

  return index;
}

void* my_realloc(void* ptr, size_t new_size)
{
  void *new = my_malloc(new_size);
  metadata_t *old = (metadata_t *) ((char *) ptr - sizeof(metadata_t));
  my_memcpy(new, ptr, old->size - sizeof(metadata_t));
  my_free(ptr);

  void *repos = (void *) ((char *) new + sizeof(metadata_t));
  return repos;
}

void my_free(void* ptr)
{
  metadata_t *md = (metadata_t *) ((char *) ptr - sizeof(metadata_t));

  md->in_use = 0;

  int m_size = 16;
  int index = 0;

  while (m_size < md->size) {
    m_size *= 2;
    index++;
  }
  
  if (freelist[index]) {
    metadata_t *front = freelist[index];
    md->next = front;
    front->prev = md;
  }
  freelist[index] = md;
}

metadata_t* find_buddy(metadata_t* ptr){
  /* FIX ME*/
  return NULL;
}

void* my_memcpy(void* dest, const void* src, size_t num_bytes)
{
  metadata_t *d = dest;
  const metadata_t *s = src;
  for (int i=0; i<num_bytes; i++) {
    d[i]=s[i];
  }
  return (void *) d;
}
