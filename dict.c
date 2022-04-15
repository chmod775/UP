#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define PERROR(A, B, ...) { printf("\033[0;31m[" A "] Error! - " B "\033[0m\n", ##__VA_ARGS__ ); exit(-1); }
#define CERROR(C, A, B, ...) { printf("\033[0;31m[" A "] Error Line: %d - " B "\033[0m\n", (C->parser->line), ##__VA_ARGS__ ); exit(-1); }

void *t_new = NULL;
#define NEW(T) t_new = malloc(sizeof(T)); if (t_new == NULL) { PERROR("NEW(##T)", "Could not malloc"); exit(-1); }

/* HELPER SHIT */
int hashOfSymbol(char *str) {
  if (str == NULL) return 0;
  int hash = *str;

  str++;

  while ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9') || (*str == '_')) {
    hash = hash * 147 + *str;
    str++;
  }

  return hash;
}

/* ##### LINKED LIST ##### */
typedef struct _s_list_item {
  struct _s_list_item *prev;
  struct _s_list_item *next;
  void *payload;
} s_list_item;

typedef struct {
  uint64_t items_count;
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

#define list_get_first(l) (l->head_item);
#define list_get_next(li) (li->next);

void list_remove_item(s_list *l, s_list_item *item) {
  l->items_count--;

  s_list_item *head = l->head_item;

  if (item == head) {
    if (head == head->next) {
      l->head_item = NULL;
    } else {
      l->head_item = item->next;
      head->prev->next = l->head_item;
    }
  } else {
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->next = NULL;
    item->prev = NULL;
  }
}

#define LIST_READ_FIRST_FAST(L) L->selected_item = L->head_item;
#define LIST_READ_NEXT_FAST(L) L->selected_item = L->selected_item->next;

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

#define list_read_selected(l) (l->selected_item->payload);

s_list_item *list_push(s_list *l, void *value) {
  s_list_item *new = NEW(s_list_item);
  new->payload = value;

  l->items_count++;

  if (l->head_item == NULL) {
    new->prev = new;
    new->next = new;
    l->head_item = new;
    return new;
  }

  s_list_item *head = l->head_item;
  s_list_item *prev = head->prev;

  prev->next = new;
  head->prev = new;

  new->next = head;
  new->prev = prev;

  return new;
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

/* ##### DICTIONARY ##### */
#define DICT_CHUNK  32

typedef struct {
  int hash;
  char *name;
  int length;
  void *value;
} s_dict_key;

typedef struct {
  uint64_t size;
  uint64_t count;
  s_dict_key *keys;
  s_list *values;
} s_dict;

s_dict *dict_create() {
  s_dict *ret = NEW(s_dict);
  ret->count = 0;
  ret->values = list_create();

  ret->size = DICT_CHUNK;
  ret->keys = malloc(sizeof(s_dict_key) * ret->size);

  return ret;
}

s_dict_key *dict_get_fast(s_dict *dict, char *key, int hash) {
  uint64_t idx = 0;
  for (idx = 0; idx < dict->count; idx++) {
    s_dict_key *key_ptr = &dict->keys[idx];
    if (key_ptr->hash == hash) {
      if (!memcmp(key_ptr->name, key, key_ptr->length))
        return key_ptr;
    }
  }

  return NULL;
}

s_dict_key *dict_get(s_dict *dict, char *key) {
  int hash = hashOfSymbol(key);
  return dict_get_fast(dict, key, hash);
}

void dict_set(s_dict *dict, char *key, void *value) {
  int hash = hashOfSymbol(key);
  s_dict_key *found_key = dict_get_fast(dict, key, hash);
  
  if (found_key == NULL) {
    if (dict->count >= dict->size) {
      uint64_t new_size = dict->size + DICT_CHUNK;
      s_dict_key *new_keys = malloc(sizeof(s_dict_key) * new_size);
      memset(new_keys, 0, sizeof(s_dict_key) * new_size);
      memcpy(new_keys, dict->keys, dict->size * sizeof(s_dict_key));
      free(dict->keys);
      dict->keys = new_keys;
      dict->size += DICT_CHUNK;
    }

    found_key = &dict->keys[dict->count];
    dict->count++;

    found_key->hash = hash;
    found_key->length = strlen(key);

    found_key->name = malloc(sizeof(char) * found_key->length);
    strcpy(found_key->name, key);

    found_key->value = list_push(dict->values, value);
  } else {
    // replace value
    PERROR("dict_set", "NOT IMPLEMENTED");
  }
}



/* TEST MATERIAL */
typedef struct {
  int hash;
  char *name;
  int length;
} s_symbol;

s_symbol *symbol_Create(char *name) {
  s_symbol *ret = NEW(s_symbol);

  ret->hash = hashOfSymbol(name);
  ret->length = strlen(name);
  ret->name = malloc(sizeof(char) * ret->length);
  strcpy(ret->name, name);

  return ret;
}

s_symbol *symbol_FindInList(char *name, s_list *list) {
  int hash = hashOfSymbol(name);

  s_symbol *symbol_ptr = list_read_first(list);
  while (symbol_ptr != NULL) {
    if (symbol_ptr->hash == hash) {
      if (!memcmp(symbol_ptr->name, name, symbol_ptr->length)) {
        return symbol_ptr;
      }
    }

    symbol_ptr = list_read_next(list);
  }

  return NULL;
}

double calculateTime(clock_t start, clock_t stop) {
  return (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;;
}

#define ITEMS 1000
#define SEARCHS 1000

void main() {
  char str[255];
  int i;
  srand(time(0));

  clock_t start = clock();

  s_list *lst = list_create();

  clock_t stop_list_create = clock();

  for (i = 0; i < ITEMS; i++) {
    sprintf(str, "sym_%X", i * 3);
    s_symbol *symbol_found = symbol_FindInList(str, lst);
    if (symbol_found == NULL) {
      s_symbol *new_sym = symbol_Create(str);
      list_push(lst, new_sym);
    }
  }
  clock_t stop_list_insert = clock();

  int founds_list = 0;
  for (i = 0; i < SEARCHS; i++) {
    int idx = rand() % (ITEMS);
    sprintf(str, "sym_%X", idx * 3);

    s_symbol *symbol_found = symbol_FindInList(str, lst);
    if (symbol_found != NULL) founds_list++;
  }
  clock_t stop_list_search = clock();

  clock_t start_dict = clock();

  s_dict *dct = dict_create();

  clock_t stop_dict_create = clock();

  for (i = 0; i < ITEMS; i++) {
    sprintf(str, "sym_%X", i * 3);
    s_symbol *new_sym = symbol_Create(str);
    dict_set(dct, str, new_sym);
  }
  clock_t stop_dict_insert = clock();

  printf("[DICT] Count: %i\n", dct->count);
  printf("[DICT] Size: %i\n", dct->size);

  int founds_dict = 0;
  for (i = 0; i < SEARCHS; i++) {
    int idx = rand() % (ITEMS);
    sprintf(str, "sym_%X", idx * 3);

    s_dict_key *key_found = dict_get(dct, str);
    if (key_found != NULL) founds_dict++;
  }
  clock_t stop_dict_search = clock();


  // Execuatable code
  printf("[LIST] Founds: %i\n", founds_list);
  printf("[LIST] Duration create: %f\n", calculateTime(start, stop_list_create));
  printf("[LIST] Duration insert: %f\n", calculateTime(stop_list_create, stop_list_insert));
  printf("[LIST] Duration search: %f\n", calculateTime(stop_list_insert, stop_list_search));
  printf("[LIST] Duration TOTAL: %f\n", calculateTime(start, stop_list_search));

  printf("[DICT] Founds: %i\n", founds_dict);
  printf("[DICT] Duration create: %f\n", calculateTime(start_dict, stop_dict_create));
  printf("[DICT] Duration insert: %f\n", calculateTime(stop_dict_create, stop_dict_insert));
  printf("[DICT] Duration search: %f\n", calculateTime(stop_dict_insert, stop_dict_search));
  printf("[DICT] Duration TOTAL: %f\n", calculateTime(start_dict, stop_dict_search));


}