#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>


static u_int32_t First0Bit(u_int32_t i) {
  i=~i;

  u_int32_t uCount;
  u_int32_t u = (i&(-i))-1;
  uCount = u
  - ((u >> 1) & 033333333333)
  - ((u >> 2) & 011111111111);
  return ((uCount + (uCount >> 3))
& 030707070707) % 63;
}

typedef struct {
  u_int64_t a;
  u_int64_t b;
  char c[13];
} s_customobj_1;

typedef struct _s_pool_chunk {
  struct _s_pool_chunk *next;
  u_int32_t occupancy;
  s_customobj_1 data[32];
} s_pool_chunk;

s_pool_chunk *pool_customobj_1;

s_pool_chunk *pool_Create() {
  s_pool_chunk *ret = (s_pool_chunk *)malloc(sizeof(s_pool_chunk));
  ret->occupancy = 0;
  return ret;
}

static s_customobj_1 *pool_Alloc(s_pool_chunk *pool) {
  // Search for free chunk
  //while ((~pool->occupancy == 0) && (pool->next != NULL)) {
  //  pool = pool->next;
  //}

  // Last chuck is all occupied, create new one
  //if (~pool->occupancy == 0) {
    //pool->next = pool_Create();
   // pool = pool->next;
  //}

  // Search last free index
  u_int32_t free_idx = First0Bit(pool->occupancy);

  // Set index as occupied
  //pool->occupancy |= 1 << free_idx;

  // Return index pointer
  return NULL;//pool->data + (free_idx * sizeof(s_customobj_1));
}


#define PERROR(A, B, ...) { printf("\033[0;31m[" A "] Error! - " B "\033[0m\n", ##__VA_ARGS__); exit(-1); }
void *t_new = NULL;
#define NEW(T) t_new = malloc(sizeof(T)); if (t_new == NULL) { PERROR("NEW(##T)", "Could not malloc"); exit(-1); }


typedef struct _s_list_item {
  struct _s_list_item *prev;
  struct _s_list_item *next;
  void *payload;
} s_list_item;

typedef struct {
  u_int64_t items_count;
  s_list_item *head_item;
  s_list_item *selected_item;
} s_list;


s_list *list_create() {
  s_list *ret = NEW(s_list);
  ret->items_count = 0;
  ret->head_item = NULL;
  ret->selected_item = NULL;
  return ret;
}
/*
void list_destroy(s_list *l) {
	PANALYSIS("list_destroy");
  void *last = list_read_last(l);
  while (last != NULL) {
    free(last);
    last = list_read_previous(l);
  }
}

void *list_read_index(s_list *l, u_int64_t index);

void list_add(s_list *l, void *value);
void *list_pull(s_list *l);
*/

void *list_read_first(s_list *l) {
  s_list_item *head = l->head_item;
  l->selected_item = head;
  if (l->head_item == NULL) return NULL;
  return l->selected_item->payload;
}
void *list_read_last(s_list *l) {
  s_list_item *head = l->head_item;
  if (l->head_item == NULL) {
    l->selected_item = head;
    return NULL;
  }

  l->selected_item = head->prev;
  return l->selected_item->payload;
}

void *list_read_next(s_list *l) {
  if (l->selected_item == NULL) return NULL;

  s_list_item *head = l->head_item;

  l->selected_item = l->selected_item->next;
  if (l->selected_item == head) {
    l->selected_item = NULL;
    return NULL;
  }

  return l->selected_item->payload;
}
void *list_read_previous(s_list *l) {
  if (l->selected_item == NULL) return NULL;

  s_list_item *head = l->head_item;

  l->selected_item = l->selected_item->prev;
  if (l->selected_item == head->prev) {
    l->selected_item = NULL;
    return NULL;
  }

  return l->selected_item->payload;
}

void list_push(s_list *l, void *value) {
  s_list_item *new = NEW(s_list_item);
  new->payload = value;

  l->items_count++;

  if (l->head_item == NULL) {
    new->prev = new;
    new->next = new;
    l->head_item = new;
    return;
  }

  s_list_item *head = l->head_item;
  s_list_item *prev = head->prev;

  prev->next = new;
  head->prev = new;

  new->next = head;
  new->prev = prev;
}
void *list_pop(s_list *l) {
  s_list_item *head = l->head_item;
  if (head == NULL) PERROR("list_pop", "Empty list.");
  s_list_item *prev = head->prev;

  void *ret = NULL;

  l->items_count--;

  if (head->next == head) {
    ret = l->head_item->payload;
    free(l->head_item);
    l->head_item = NULL;
    return ret;
  }

  head->prev = prev->prev;
  head->prev->next = head;

  ret = prev->payload;
  free(prev);

  return ret;
}



void main() {
  int i;

  s_list *list = list_create();

  list_push(list, 1);
  list_push(list, 2);
  list_push(list, 3);
  list_push(list, 4);

  i = list_read_last(list);
  while (i != NULL) {
    printf("%d\n", i);
    i = list_read_previous(list);
  }

  i = list_read_previous(list);printf("%d\n", i);
  
  i = list_pop(list);printf("%d\n", i);
  i = list_pop(list);printf("%d\n", i);
  i = list_pop(list);printf("%d\n", i);
  i = list_pop(list);printf("%d\n", i);

  s_customobj_1 *t = NULL;

  clock_t begin = clock();

  for (i = 1; i < 1000000; i++) {
    //t = (s_pool_chunk *)malloc(sizeof(s_pool_chunk));
    list_push(list, i);
  }

  clock_t end_malloc = clock();

  clock_t begin_pool = clock();

  t = list_read_first(list);
  while (t != NULL) {
    t = list_read_next(list);
  }

  clock_t end_pool = clock();

  double time_spent_malloc = (double)(end_malloc - begin) / CLOCKS_PER_SEC;
  double time_spent_pool = (double)(end_pool - begin_pool) / CLOCKS_PER_SEC;

  printf("Malloc done in: %lf\n", time_spent_malloc);
  printf("Pool done in: %lf\n", time_spent_pool);





}