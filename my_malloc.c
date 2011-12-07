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

  if (needed > 2048) return NULL; 
  if (!heap || size > 1024) init_heap();

  int index = get_index(size);

  metadata_t *front, *next;
  if (freelist[index]) {
    front = freelist[index];
    next = front->next;
    
    front->in_use = 1;
    front->next = NULL;
    front->prev = NULL;

    if (next) {
      next->prev = NULL;
      freelist[index] = next;
    } else {
      freelist[index] = NULL;
    }
    
    void *repos = (void *) ((char *) front + sizeof(metadata_t));
    
    return repos;
  }

  int available = index;
  while (!freelist[available]) {
    available++;
  }

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
  if (ptr == NULL) return new;
  if (new_size == 0) {
    my_free(ptr);
    return NULL;
  }
  
  metadata_t *old = (metadata_t *) ((char *) ptr - sizeof(metadata_t));
  my_memcpy(new, ptr, old->size - sizeof(metadata_t));
  my_free(ptr);

  return new;
}

void my_free(void* ptr)
{
  metadata_t *md = (metadata_t *) ((char *) ptr - sizeof(metadata_t));

  int m_size = 16;
  int index = 0;

  metadata_t *buddy = find_buddy(md);
  while (buddy && !buddy->in_use &&
         buddy->size < 2048 && buddy->size &&
         md->size < 2048 && 
         md->size == buddy->size) {

    if (buddy->next && buddy->prev) {
      buddy->prev = buddy->next;
      buddy->next = buddy->prev;
    } else if (buddy->next) {
      buddy->next->prev = NULL;
    } else if (buddy->prev) {
      buddy->prev->next = NULL;
    }

    if (buddy < md) {
      md = buddy;
    }
    md->size *= 2;
    buddy = find_buddy(md);
  }

  md->in_use = 0;

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
  int buddy = (int) ptr ^ ptr->size;
  metadata_t *b = (metadata_t *) buddy;

  return b;
}

void* my_memcpy(void* dest, const void* src, size_t num_bytes)
{
  metadata_t *d = dest;
  const metadata_t *s = src;
  for (int i=0; i<num_bytes; i++) {
    d[i]=s[i];
  }
  return d;
}
