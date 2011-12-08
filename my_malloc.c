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
  if (!heap) init_heap();
  if (!heap) return NULL;

  int index = get_index(needed);

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
    
    return offset_pointer(front, 1);
  }

  int available = index;
  while (!freelist[available]) {
    available++;
  }

  if (available == 8) {
    metadata_t *new_heap = my_sbrk(SBRK_SIZE);
    if (!new_heap) return NULL;
    new_heap->in_use = 0;
    new_heap->size = 2048;
    new_heap->next = NULL;
    new_heap->prev = NULL;
    if (freelist[7]) freelist[7]->next = new_heap;
    else freelist[7] = new_heap;
    available--;
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
  if (freelist[index]->next) {
    freelist[index] = freelist[index]->next;
    freelist[index]->prev = NULL;
  } else {
    freelist[index] = NULL;
  }
  ret_meta->next = NULL;
  ret_meta->in_use = 1;

  return offset_pointer(ret_meta, 1);
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

void* offset_pointer(metadata_t* ptr, int offset) {
  char *offset_ptr = (char *) ptr;
  if (offset) return offset_ptr + sizeof(metadata_t);
  else return offset_ptr - sizeof(metadata_t);
}

void print_freelist() {
  int size = 16;
  for (int i=0; i<8; i++) {
    fprintf(stderr, "[%d] -> %d: %p\n", i, size, (void *) freelist[i]);
    size *= 2;
  }
}

void print_block(metadata_t *block) {
  fprintf(stderr, "Printing block data\n");
  fprintf(stderr, "address: %p\n", (void *) block);
  fprintf(stderr, "in use: %d\n", block->in_use);
  fprintf(stderr, "size: %d\n", block->size);
  fprintf(stderr, "next: %p\n", (void *) block->next);
  fprintf(stderr, "prev: %p\n", (void *) block->prev);
  fprintf(stderr, "\n");
}

void* my_realloc(void* ptr, size_t new_size)
{
  void *new;
  if (new_size == 0) {
    my_free(ptr);
    return NULL;
  }

  new = my_malloc(new_size);
  if (ptr == NULL) return new;
 
  metadata_t *old = offset_pointer(ptr, 0);
  my_memcpy(new, ptr, old->size - sizeof(metadata_t));
  my_free(ptr);

  return new;
}

void my_free(void* ptr)
{
  metadata_t *block = offset_pointer(ptr, 0);
  block->in_use = 0;

  metadata_t *buddy = find_buddy(block);
  while (buddy && !buddy->in_use) {
    if (buddy->next && buddy->prev) {
      buddy->next->prev = buddy->prev;
      buddy->prev->next = buddy->next;
    } else if (buddy->next) {
      buddy->next->prev = NULL;
    } else if (buddy->prev) {
      buddy->prev->next = NULL;
    } else {
      freelist[get_index(buddy->size)] = NULL;
    }

    if (buddy < block) block = buddy;
    block->size *= 2;
    buddy = find_buddy(block);
  }

  int index = get_index(block->size);
  if (freelist[index]) {
    metadata_t *front = freelist[index];
    front->prev = block;
    block->next = front;
  }
  freelist[index] = block;
}

metadata_t* find_buddy(metadata_t* ptr){
  int buddy = (int) ptr ^ ptr->size;
  metadata_t *b = (metadata_t *) buddy;
  if (ptr->size == b->size) return b;
  else return NULL;
}

void* my_memcpy(void* dest, const void* src, size_t num_bytes)
{
  char *d = (char *) dest;
  char *s = (char *) src;
  for (int i=0; i<num_bytes; i++) {
    d[i]=s[i];
  }
  return d;
}
