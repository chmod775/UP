#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

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

/* ##### Symbols ##### */
typedef struct _s_symbol {
  int hash;
  char *name;
  int length;
} s_symbol;

s_symbol *sym_create(char *name) {
  s_symbol *ret = malloc(sizeof(s_symbol));

  ret->hash = hashOfSymbol(name);
  ret->length = strlen(name);

  ret->name = malloc(sizeof(char) * ret->length);
  memcpy(ret->name, name, ret->length);

  return ret;
}


/* ##### Hash table ##### */
typedef struct _s_ht {
  uint64_t capacity;
  uint64_t length;
  s_symbol **data;
} s_ht;

s_ht ht_create() {
  s_ht ret;

  ret.capacity = 16;
  ret.length = 0;
  ret.data = calloc(ret.capacity, sizeof(s_symbol *));

  return ret;
}

bool ht_set(s_ht *ht, s_symbol *sym);

void ht_expand(s_ht *ht) {
  s_symbol **old_data = ht->data;
  uint64_t old_capacity = ht->capacity;

  ht->length = 0;
  ht->capacity = ht->capacity * 2;
  ht->data = calloc(ht->capacity, sizeof(s_symbol *));

  for (int i = 0; i < old_capacity; i++) {
    s_symbol *old_sym = old_data[i];
    if (old_sym != NULL) {
      ht_set(ht, old_sym);
    }
  }

  free(old_data);
}

bool ht_exists(s_ht *ht, s_symbol *sym) {
  size_t index = (size_t)(sym->hash & (uint64_t)(ht->capacity - 1));

  if (ht->data[index] != NULL)
    for (uint64_t i = 0; i < ht->capacity; i++) {
      s_symbol *found_sym = ht->data[index];
      if (found_sym != NULL)
        if (found_sym->hash == sym->hash)
          if (memcmp(sym->name, found_sym->name, MAX(sym->length, found_sym->length)) == 0)
            return true;

      index++;
      if (index >= ht->capacity)
        index = 0;
    }

  return false;
}

s_symbol *ht_get(s_ht *ht, char *key) {
  int hash = hashOfSymbol(key);

  size_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

  if (ht->data[index] != NULL)
    for (uint64_t i = 0; i < ht->capacity; i++) {
      s_symbol *found_sym = ht->data[index];
      if (found_sym != NULL)
        if (found_sym->hash == hash)
          if (memcmp(key, found_sym->name, MAX(strlen(key), found_sym->length)) == 0)
            return found_sym;

      index++;
      if (index >= ht->capacity)
        index = 0;
    }
  
  return NULL;
}

bool ht_set(s_ht *ht, s_symbol *sym) {
  if (ht->length >= ht->capacity / 2)
    ht_expand(ht);

  size_t index = (size_t)(sym->hash & (uint64_t)(ht->capacity - 1));

  if (ht->data[index] != NULL)
    for (uint64_t i = 0; i < ht->capacity; i++) {
      if (ht_exists(ht, sym)) return true;

      if (ht->data[index] == NULL) break;

      index++;
      if (index >= ht->capacity)
        index = 0;
    }

  ht->length++;
  ht->data[index] = sym;

  return false;
}


void ht_debug(s_ht *ht) {
  for (uint64_t i = 0; i < ht->capacity; i++) {
    s_symbol *found_sym = ht->data[i];
    if (found_sym != NULL) {
      printf("%i: %s\n", i, found_sym->name);
    } else{
      printf("%i: NULL\n", i);
    }
  }
}

void main() {
  char str[50];

  s_ht ht = ht_create();

  for (int i = 0; i < 100; i++) {
    sprintf(str, "var%d", i);
    s_symbol *s1 = sym_create(str);
    bool ret = ht_set(&ht, s1);
    if (ret)
      printf("exists %s\n", str);

    s_symbol *f = ht_get(&ht, "var1");
    if (f == NULL)
      printf("missing");
  }

  ht_debug(&ht);

  s_symbol *f = ht_get(&ht, "var1");

  return;

}