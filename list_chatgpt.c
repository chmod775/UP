// Types unchanged (but prefer size_t for count):
// typedef struct { size_t items_count; ... } s_list;

static inline void list_clear_cursor_if_removed(s_list *l, s_list_item *item) {
    if (l->selected_item == item) l->selected_item = NULL;
}

s_list *list_create(void) {
  s_list *ret = NEW(s_list);
  ret->items_count = 0;
  ret->head_item = NULL;
  ret->selected_item = NULL;
  return ret;
}

/* --- Safer macros (no trailing ; and with parentheses) --- */
#define list_get_first(L)        ((L)->head_item)
#define list_get_next(NODE)      ((NODE)->next)
#define list_read_selected(L)    ((L)->selected_item ? (L)->selected_item->payload : NULL)
#define LIST_READ_FIRST_FAST(L)  do { (L)->selected_item = (L)->head_item; } while (0)
#define LIST_READ_NEXT_FAST(L)   do { if ((L)->selected_item) (L)->selected_item = (L)->selected_item->next; } while (0)

/* Optional: user-provided destructor for payloads */
typedef void (*s_list_free_fn)(void*);
static s_list_free_fn g_payload_dtor = NULL;
void list_set_payload_destructor(s_list_free_fn d) { g_payload_dtor = d; }

/* --- Remove arbitrary item (fixed) --- */
void list_remove_item(s_list *l, s_list_item *item) {
  if (!l || !item) return;
  s_list_item *head = l->head_item;
  if (!head) return;                       // nothing to do

  /* single element list */
  if (head == head->next && item == head) {
    l->head_item = NULL;
    list_clear_cursor_if_removed(l, item);
    if (g_payload_dtor) g_payload_dtor(item->payload);
    free(item);
    l->items_count = 0;
    return;
  }

  /* relink neighbors (works for head or non-head) */
  item->prev->next = item->next;
  item->next->prev = item->prev;

  /* if removing head, advance head */
  if (item == head) {
    l->head_item = item->next;
  }

  list_clear_cursor_if_removed(l, item);
  if (g_payload_dtor) g_payload_dtor(item->payload);
  free(item);

  if (l->items_count > 0) l->items_count--;
}

/* --- Push at tail (O(1)) --- */
void list_push(s_list *l, void *value) {
  s_list_item *node = NEW(s_list_item);
  node->payload = value;

  if (!l->head_item) {
    node->prev = node->next = node;
    l->head_item = node;
  } else {
    s_list_item *head = l->head_item;
    s_list_item *tail = head->prev;
    tail->next = node;
    node->prev = tail;
    node->next = head;
    head->prev = node;
  }
  l->items_count++;
}

/* --- Pop from tail (O(1), fixed) --- */
void *list_pop(s_list *l) {
  if (!l || !l->head_item) { PERROR("list_pop", "Empty list."); return NULL; }

  s_list_item *head = l->head_item;
  s_list_item *tail = head->prev;
  void *ret = tail->payload;

  if (tail == head) {                // only one element
    list_clear_cursor_if_removed(l, tail);
    l->head_item = NULL;
    l->items_count = 0;
    free(tail);
    return ret;
  }

  // remove tail
  s_list_item *new_tail = tail->prev;
  new_tail->next = head;
  head->prev = new_tail;

  list_clear_cursor_if_removed(l, tail);
  free(tail);
  l->items_count--;
  return ret;
}

/* --- Iteration (unchanged semantics, but guarded) --- */
void *list_read_first(s_list *l) {
  if (!l || !l->head_item) { l->selected_item = NULL; return NULL; }
  l->selected_item = l->head_item;
  return l->selected_item->payload;
}
void *list_read_last(s_list *l) {
  if (!l || !l->head_item) { l->selected_item = NULL; return NULL; }
  l->selected_item = l->head_item->prev;
  return l->selected_item->payload;
}
void *list_read_next(s_list *l) {
  if (!l || !l->selected_item) return NULL;
  s_list_item *head = l->head_item;
  l->selected_item = l->selected_item->next;
  if (l->selected_item == head) { l->selected_item = NULL; return NULL; }
  return l->selected_item->payload;
}
void *list_read_previous(s_list *l) {
  if (!l || !l->selected_item) return NULL;
  s_list_item *head = l->head_item;
  l->selected_item = l->selected_item->prev;
  if (l->selected_item == head->prev) { l->selected_item = NULL; return NULL; }
  return l->selected_item->payload;
}
