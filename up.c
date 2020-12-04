#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "up.h"

//#define ANALYSIS

FILE *fanalysis;

#ifdef ANALYSIS
#define PANALYSIS(A) fprintf(fanalysis, A "\n");
#else
#define PANALYSIS 
#endif

#define PERROR(A, B, ...) { printf("\033[0;31m[" A "] Error! - " B "\033[0m\n", ##__VA_ARGS__); exit(-1); }
#define CERROR(C, A, B, ...) { printf("\033[0;31m[" A "] Error Line: %d - " B "\033[0m\n", (C->parser->line) ##__VA_ARGS__); exit(-1); }

void *t_new = NULL;
#define NEW(T) t_new = malloc(sizeof(T)); if (t_new == NULL) { PERROR("NEW(##T)", "Could not malloc"); exit(-1); }

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* ##### Helpers shit ##### */
FILE *openFile(char *filename) {
	PANALYSIS("openFile");
  FILE *ret = fopen(filename, "r");
  if (ret == NULL) PERROR("openFile", "could not open(%s)", filename);
  return ret;
}

void closeFile(FILE *fd) {
	PANALYSIS("closeFile");
  if (fclose(fd) != 0)
    PERROR("closeFile", "could not close file");
}

int hashOfSymbol(char *str) {
	PANALYSIS("hashOfSymbol");
  if (str == NULL) return 0;
  int hash = *str;

  str++;

  while ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9') || (*str == '_')) {
    hash = hash * 147 + *str;
    str++;
  }

  return hash;
}

bool isUppercase_Char(char ch) { return (ch >= 'A' && ch <= 'Z'); }
bool isUppercase_String(char *str) { return isUppercase_Char(str[0]); }

bool isFullcase_String(char *str) { return isUppercase_Char(str[0]) && isUppercase_Char(str[1]); }

bool startsUnderscore_String(char *str) { return str[0] == '_'; }

/* ##### LINKED LIST ##### */
s_list *list_create() {
	PANALYSIS("list_create");
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
	PANALYSIS("list_read_first");
  s_list_item *head = l->head_item;
  l->selected_item = head;
  if (l->head_item == NULL) return NULL;
  return l->selected_item->payload;
}
void *list_read_last(s_list *l) {
	PANALYSIS("list_read_last");
  s_list_item *head = l->head_item;
  if (l->head_item == NULL) {
    l->selected_item = head;
    return NULL;
  }

  l->selected_item = head->prev;
  return l->selected_item->payload;
}

void *list_read_next(s_list *l) {
	PANALYSIS("list_read_next");
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
	PANALYSIS("list_read_previous");
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

void list_push(s_list *l, void *value) {
	PANALYSIS("list_push");
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
	PANALYSIS("list_pop");
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

/* ##### PARENTABLE LINKED LIST ##### */
s_parlist *parlist_create(s_parlist *parent) {
	PANALYSIS("parlist_create");
  s_parlist *ret = NEW(s_parlist);

  ret->lists = list_create();

  if (parent != NULL) {
    s_list *l = (s_list *)list_read_first(parent->lists);

    while (l != NULL) {
      list_push(ret->lists, l);
      l = list_read_next(parent->lists);
    }
  }

  s_list *emptyList = list_create();
  list_push(ret->lists, emptyList);

  ret->selected_list = emptyList;

  return ret;
}
/*
void parlist_destroy(s_parlist *pl) {
	PANALYSIS("parlist_destroy");
  void *last = parlist_read_last(pl);
  while (last != NULL) {
    list_destroy(last);
    last = parlist_read_previous(pl);
  }
}
*/
void *parlist_read_first(s_parlist *pl) {
	PANALYSIS("parlist_read_first");
  s_list *l = list_read_first(pl->lists);
  pl->selected_list = l;
  return list_read_first(l);
}
void *parlist_read_last(s_parlist *pl) {
	PANALYSIS("parlist_read_last");
  s_list *l = list_read_last(pl->lists);
  pl->selected_list = l;
  return list_read_last(l);
}
void *parlist_read_next(s_parlist *pl) {
	PANALYSIS("parlist_read_next");
  void *ret = list_read_next(pl->selected_list);

  if (ret == NULL) {
    s_list *l = list_read_next(pl->lists);
    if (l == NULL) return NULL;

    pl->selected_list = l;
    ret = list_read_first(l);
  }

  return ret;
}
void *parlist_read_previous(s_parlist *pl) {
	PANALYSIS("parlist_read_previous");
  void *ret = list_read_previous(pl->selected_list);

  if (ret == NULL) {
    s_list *l = list_read_previous(pl->lists);
    if (l == NULL) return NULL;

    pl->selected_list = l;
    ret = list_read_last(l);
  }

  return ret;
}
/*
void parlist_add(s_parlist *pl, void *value) {
	PANALYSIS("parlist_add");
  s_list *l = list_read_first(pl->lists);
  list_add(l, value);
}
*/
void parlist_push(s_parlist *pl, void *value) {
	PANALYSIS("parlist_push");
  s_list *l = list_read_last(pl->lists);
  list_push(l, value);
}

/* ##### Symbols ##### */
char *symbol_GetCleanName(s_symbol *symbol) {
	PANALYSIS("symbol_GetCleanName");
  char *ret = (char *)malloc(sizeof(char) * (symbol->length + 1));
  memcpy(ret, symbol->name, symbol->length);
  ret[symbol->length] = 0;
  return ret;
}

s_symbol *symbol_Create(char *name, e_symboltype type, int length) {
	PANALYSIS("symbol_Create");
  s_symbol *ret = NEW(s_symbol);

  ret->hash = hashOfSymbol(name);
  ret->name = name;
  ret->length = (length > -1) ? length : strlen(name);
  ret->type = type;

  ret->isUppercase = isUppercase_String(name);
  ret->isFullcase = isFullcase_String(name);
  ret->startsUnderscore = startsUnderscore_String(name);

  return ret;
}

s_symbol *symbol_CreateEmpty(e_symboltype type) {
	PANALYSIS("symbol_CreateEmpty");
  s_symbol *ret = NEW(s_symbol);

  ret->hash = 0;
  ret->name = NULL;
  ret->length = 0;
  ret->type = type;

  ret->isUppercase = false;
  ret->isFullcase = false;
  ret->startsUnderscore = false;

  return ret;  
}

s_symbol *symbol_CreateFromKeyword(char *keyword, e_token token) {
	PANALYSIS("symbol_CreateFromKeyword");
  s_symbol *ret = symbol_Create(keyword, SYMBOL_KEYWORD, -1);

  ret->body.keyword = NEW(s_symbolbody_keyword);
  ret->body.keyword->token = token;

  return ret;
}

s_symbol *symbol_Find(char *name, s_parlist *symbols) {
	PANALYSIS("symbol_Find");
  int hash = hashOfSymbol(name);
  s_symbol *symbol_ptr = (s_symbol *)parlist_read_first(symbols);
  
  while (symbol_ptr != NULL) {
    if (symbol_ptr->hash == hash) {
      if (!memcmp(symbol_ptr->name, name, symbol_ptr->length))
        return symbol_ptr;
    }
    symbol_ptr = (s_symbol *)parlist_read_next(symbols);
  }

  return NULL;
}

bool symbol_Equal(s_symbol *a, s_symbol *b) {
  if (a->hash == b->hash) {
    if (!memcmp(a->name, b->name, MAX(a->length, b->length)))
      return true;
  }
  return false;
}

/* ##### Scope ##### */
s_scope *scope_Create(s_scope *parent) {
	PANALYSIS("scope_Create");
  s_scope *ret = NEW(s_scope);

  ret->parent = parent;

  ret->symbols = parlist_create(parent == NULL ? NULL : parent->symbols);
  if (!ret->symbols) { PERROR("scope_Init", "Could not create symbols list"); return NULL; }

  return ret;
}

s_scope *scope_CreateAsRoot() {
	PANALYSIS("scope_CreateAsRoot");
  s_scope *ret = scope_Create(NULL);

  // Init symbols with base keywords
  scope_AddSymbol(ret, symbol_CreateFromKeyword("NULL", TOKEN_NULL));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("if", TOKEN_If));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("else", TOKEN_Else));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("while", TOKEN_While));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("for", TOKEN_For));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("switch", TOKEN_Switch));

  scope_AddSymbol(ret, symbol_CreateFromKeyword("this", TOKEN_This));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("return", TOKEN_Return));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("super", TOKEN_Super));

  scope_AddSymbol(ret, symbol_CreateFromKeyword("DEBUG", TOKEN_Debug));

  return ret;
}

void scope_AddSymbol(s_scope *scope, s_symbol *symbol) {
	PANALYSIS("scope_AddSymbol");
  parlist_push(scope->symbols, symbol);
}

char buffer[1000];
char *scope_Print(s_scope *scope) {
	PANALYSIS("scope_Print");
  char *buffer_ptr = buffer;

  s_symbol *symbol_ptr = (s_symbol *)parlist_read_first(scope->symbols);
  while (symbol_ptr != NULL) {
    int len = sprintf(buffer_ptr, "%s - %d,", symbol_GetCleanName(symbol_ptr), symbol_ptr->type);
    buffer_ptr += len;
    symbol_ptr = (s_symbol *)parlist_read_next(scope->symbols);
  }

  return buffer;
}

/* ##### Parser ##### */
s_parser *parse_Init(char *str) {
	PANALYSIS("parse_Init");
  s_parser *ret = NEW(s_parser);

  ret->line = 1;
  ret->ptr = ret->source = str;
  if (!ret->source) { PERROR("parse_Init", "Src is null"); return NULL; }

  return ret;
}

int parse_Next(s_scope *scope, s_parser *parser) {
	PANALYSIS("parse_Next");
  #define src parser->ptr
  #define ret(TOKEN, CONTENT, CONTENTTYPE) { parser->token.content.CONTENTTYPE = (CONTENT); parser->token.type = (TOKEN); return (TOKEN); }

  int token;                    // current token
  char *last_pos;
  int hash;

  int64_t token_val_int;
  double token_val_decimal;
  char *token_val_string = NULL;

  while (token = *src) {
    ++src;

    // parse token here
    if (token == '\n') {
      ++parser->line;
    }
    else if (token == '#') {
      // skip macro, because we will not support it
      while (*src != 0 && *src != '\n') {
        src++;
      }
    }
    else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
      // parse identifier
      last_pos = src - 1;
      hash = token;

      while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
        hash = hash * 147 + *src;
        src++;
      }

      // Search for define symbol
      s_symbol *symbol_ptr = (s_symbol *)parlist_read_first(scope->symbols);

      while (symbol_ptr != NULL) {
        if (symbol_ptr->hash == hash) {
          if (!memcmp(symbol_ptr->name, last_pos, symbol_ptr->length)) {
            if (symbol_ptr->type == SYMBOL_KEYWORD) {
              ret(symbol_ptr->body.keyword->token, NULL, any);
            } else {
              ret(TOKEN_Symbol, symbol_ptr, symbol);
            }
          }
        }
        symbol_ptr = (s_symbol *)parlist_read_next(scope->symbols);
      }

      // No symbol found, create one
      s_symbol *new_symbol = symbol_Create(last_pos, SYMBOL_NOTDEFINED, src - last_pos);
      scope_AddSymbol(scope, new_symbol);

      ret(TOKEN_Symbol, new_symbol, symbol);
    }
    else if (token >= '0' && token <= '9') {
      // parse number, three kinds: dec(123) hex(0x123) oct(017)
      last_pos = src - 1;
      token_val_int = token - '0';
      if (token_val_int > 0) {
        // dec, starts with [1-9]
        while ((*src >= '0' && *src <= '9') || (*src == '.')) {
          if (*src == '.')
            break;
          token_val_int = token_val_int*10 + *src++ - '0';
        }
        if (*src == '.') { // Float type contains a point
          src = last_pos;
          token_val_decimal = strtod(src, &src);
          ret(TOKEN_Literal_Real, token_val_decimal, decimal);
        }
      } else {
        // starts with 0
        if (*src == 'x' || *src == 'X') {
          //hex
          token = *++src;
          while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
            token_val_int = token_val_int * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
            token = *++src;
          }
        } else {
            // oct
          while (*src >= '0' && *src <= '7') {
            token_val_int = token_val_int*8 + *src++ - '0';
          }
        }
      }

      
      ret(TOKEN_Literal_Int, token_val_int, integer);
    }
    else if (token == '"' || token == '\'') {
      token_val_string = (char *)malloc(sizeof(char) * 128);
      char *token_val_string_ptr = token_val_string;
      while (*src != 0 && *src != token) {
        char ch = *src++;
        if (ch == '\\') {
          // escape character
          ch = *src++;
          if (ch == 'n') {
            ch = '\n';
          }
        }

        *token_val_string_ptr = ch;
        token_val_string_ptr++;
      }

      src++;

      ret(TOKEN_Literal_String, token_val_string, string);
    }
    else if (token == '/') {
      if (*src == '/') {
        // skip comments
        while (*src != 0 && *src != '\n') {
          ++src;
        }
      } else if (*src == '*') {
        // Block comment
        int _token = 0;
        do {
          _token = parse_Next(scope, parser);
        } while (_token != TOKEN_CommentBlock_End);
      } else {
        // divide operator
        ret(TOKEN_Div, NULL, any);
      }
    }
    else if (token == '=') {
      // parse '==', '=' or '=>'
      if (*src == '=') {
        src ++;
        ret(TOKEN_Eq, NULL, any);
      } else if (*src == '>') {
        src ++;
        ret(TOKEN_Link, NULL, any);
      } else {
        ret(TOKEN_Assign, NULL, any);
      }
    }
    else if (token == '+') {
      // parse '+' and '++'
      if (*src == '+') {
        src++;
        ret(TOKEN_Inc, NULL, any);
      } else {
        ret(TOKEN_Add, NULL, any);
      }
    }
    else if (token == '-') {
      // parse '-' and '--'
      if (*src == '-') {
        src ++;
        ret(TOKEN_Dec, NULL, any);
      } else {
        ret(TOKEN_Sub, NULL, any);
      }
    }
    else if (token == '!') {
      // parse '!='
      if (*src == '=') {
        src++;
        ret(TOKEN_Ne, NULL, any);
      } else {
        ret(token, NULL, any);
      }
    }
    else if (token == '<') {
      // parse '<=', '<<' or '<'
      if (*src == '=') {
        src ++;
        ret(TOKEN_Le, NULL, any);
      } else if (*src == '<') {
        src ++;
        ret(TOKEN_Shl, NULL, any);
      } else {
        ret(TOKEN_Lt, NULL, any);
      }
    }
    else if (token == '>') {
      // parse '>=', '>>' or '>'
      if (*src == '=') {
        src ++;
        ret(TOKEN_Ge, NULL, any);
      } else if (*src == '>') {
        src ++;
        ret(TOKEN_Shr, NULL, any);
      } else {
        ret(TOKEN_Gt, NULL, any);
      }
    }
    else if (token == '|') {
      // parse '|' or '||'
      if (*src == '|') {
        src ++;
        ret(TOKEN_Lor, NULL, any);
      } else {
        ret(TOKEN_Or, NULL, any);
      }
    }
    else if (token == '&') {
      // parse '&' and '&&'
      if (*src == '&') {
        src ++;
        ret(TOKEN_Lan, NULL, any);
      } else {
        ret(TOKEN_And, NULL, any);
      }
    }
    else if (token == '^') {
      ret(TOKEN_Xor, NULL, any);
    }
    else if (token == '%') {
      ret(TOKEN_Mod, NULL, any);
    }
    else if (token == '*') {
      if (*src == '/') {
        src ++;
        ret(TOKEN_CommentBlock_End, NULL, any);
      } else {
        ret(TOKEN_Mul, NULL, any);
      }
    }
    else if (token == '?') {
      ret(TOKEN_Cond, NULL, any);
    }
    else if (token == '.') {
      ret(TOKEN_Dot, NULL, any);
    }
    else if (token == ':') {
      if (*src == ':') {
        src ++;
        ret(TOKEN_DirectChildren, NULL, any);
      } else {
        ret(token, NULL, any);
      }
    } else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == '[' || token == ']' || token == ',') {
      // directly return the character as token;
      ret(token, NULL, any);
    }
  }

  ret(token, NULL, any);

  #undef ret
  #undef src
}

int parse_Match(s_scope *scope, s_parser *parser, int token) {
	PANALYSIS("parse_Match");
  if (parser->token.type == token) {
    parse_Next(scope, parser);
  } else {
    if (token < TOKEN_Symbol)
      printf("Line %d: expected token: %c\n", parser->line, token);
    else
      printf("Line %d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
}

s_token parse_Preview(s_scope *scope, s_parser *parser) {
	PANALYSIS("parse_Preview");
  s_parser _parser = *parser;
  parse_Next(scope, &_parser);
  return _parser.token;
}

/* ##### STATEMENT ##### */
s_statement *statement_Create(s_statement *parent, s_scope *scope, e_statementtype type) {
	PANALYSIS("statement_Create");
  s_statement *ret = NEW(s_statement);

  ret->parent = parent;
  ret->scope = scope;

  ret->type = type;
  ret->temporaries = list_create();

  return ret;
}

s_statement *statement_CreateInside(s_statement *parent, e_statementtype type) {
	PANALYSIS("statement_CreateInside");
  s_statement *ret = NEW(s_statement);

  ret->parent = parent;
  ret->scope = parent->scope;

  ret->type = type;
  ret->temporaries = list_create();
  
  return ret;
}

s_statement *statement_CreateChildren(s_statement *parent, e_statementtype type) {
	PANALYSIS("statement_CreateChildren");
  s_statement *ret = NEW(s_statement);

  ret->parent = parent;
  ret->scope = scope_Create(parent->scope);

  ret->type = type;
  ret->temporaries = list_create();

  return ret;
}


s_statement *statement_CreateBlock(s_statement *parent) {
	PANALYSIS("statement_CreateBlock");
  s_statement *ret = statement_CreateChildren(parent, STATEMENT_BLOCK);
  
  ret->exe_cb = &__core_exe_statement;

  ret->body.block = NEW(s_statementbody_block);
  ret->body.block->statements = list_create();

  return ret;
}

s_statement *statement_CreateRoot(s_compiler *compiler) {
	PANALYSIS("statement_CreateRoot");
  // Create a class Statement named "Program"
  s_statement *ret = NEW(s_statement);

  ret->type = STATEMENT_CLASS_DEF;
  ret->parent = NULL;
  ret->exe_cb = NULL;

  ret->body.class_def = NEW(s_statementbody_class_def);
  ret->body.class_def->symbol = class_Create("Program", compiler->rootScope);

  ret->scope = ret->body.class_def->symbol->body.class->scope;

  ret->temporaries = list_create();

  return ret;
}

/* ##### Compiler ##### */
s_compiler *compiler_Init(char *content) {
	PANALYSIS("compiler_Init");
  s_compiler *ret = NEW(s_compiler);

  ret->rootScope = scope_CreateAsRoot();
  if (!ret->rootScope) { PERROR("compiler_Init", "Could not create root scope"); return NULL; }

  ret->rootStatement = statement_CreateRoot(ret);
  if (!ret->rootStatement) { PERROR("compiler_Init", "Could not create root class statement"); return NULL; }

  ret->parser = parse_Init(content);
  if (!ret->parser) { PERROR("compiler_Init", "Could not initialize root parser"); return NULL; }

  stack = stack_create__s_class_instance_ptr(256);

  return ret;
}

s_compiler *compiler_InitFromFile(char *filename) {
	PANALYSIS("compiler_InitFromFile");
  const int sizeLimit = 256 * 1024; // arbitrary size

  // Open source file
  FILE *fd = openFile(filename);
  if (!fd) { PERROR("compiler_InitFromFile", "Error opening file"); return NULL; }

  // Malloc source code area
  char *content = (char *)malloc(sizeLimit * sizeof(char));
  if (!content) { PERROR("compiler_InitFromFile", "Could not malloc source code area"); return NULL; }

  // Read the source file
  int l;
  if ((l = fread(content, 1, sizeLimit-1, fd)) <= 0) { PERROR("compiler_InitFromFile", "fread() returned %d", l); return NULL; }
  content[l] = 0; // add EOF character

  // Close file
  closeFile(fd);

  return compiler_Init(content);
}

void compiler_Execute(s_compiler *compiler) {
	PANALYSIS("compiler_Execute");
  parse_Next(compiler->rootStatement->scope, compiler->parser);

  compile_ClassBody(compiler, compiler->rootStatement);

  s_class_instance *programInstance = class_CreateInstance(compiler->rootStatement->body.class_def->symbol);

  s_list *args = list_create();
  s_method_def *mainMethod = class_FindMethodByName(compiler->rootStatement->body.class_def->symbol, "Main", args);

	PANALYSIS("compiler_Execute - RUN");
  __exe_method(programInstance, mainMethod, NULL, NULL);
}

void compiler_ExecuteCLI(s_compiler *compiler, char *code) {
	PANALYSIS("compiler_ExecuteCLI");
  compiler->parser = parse_Init(code);

  parse_Next(compiler->rootStatement->scope, compiler->parser);

  compile_Statement(compiler, compiler->rootStatement);
}

s_expression_operation *expression_Emit(s_list *core_operations, e_expression_operation_type type) {
	PANALYSIS("expression_Emit");
  s_expression_operation *op = NEW(s_expression_operation);
  op->type = type;
  list_push(core_operations, op);
  return op;
}

#define token (compiler->parser->token)
#define match(S, A) parse_Match((S), compiler->parser, (A))
#define preview(S) parse_Preview((S), compiler->parser)
#define emit(O, A) list_push(O, (A))

s_symbol *expression_GetClassOfOperation(s_expression_operation *operation) {
  s_symbol *ret = NULL;

  if (operation->type == OP_AccessSymbol) {
    ret = operation->payload.symbol->body.field->value.type;
  } else if ((operation->type == OP_LoadThis) || (operation->type == OP_LoadReturn)) {
    ret = operation->payload.symbol;
  } else if (operation->type == OP_UseTemporaryInstance) {
    ret = operation->payload.temporary->class;
  } else if ((operation->type == OP_MethodCall) || (operation->type == OP_ConstructorCall)) {
    ret = operation->payload.method->ret_type;
  } else {
    PERROR("expression_GetClassOfOperation", "Wrong operation type");
  }

  return ret;
}

s_method_def *expression_FindMethodOverloadInOperation(s_expression_operation *operation, char *name, s_list *args) {
  s_symbol *symbol_target = expression_GetClassOfOperation(operation);
  if (symbol_target == NULL) PERROR("expression_FindMethodOverloadInOperation", "No target symbol found");

  s_method_def *ret = class_FindMethodByName(symbol_target, name, args);
  if (ret == NULL) PERROR("expression_FindMethodOverloadInOperation", "No method found");

  return ret;
}

s_expression_operation *expression_MethodCall(s_list *operations, s_method_def *method) {
  // Allocate return type
  if (method->ret_type != NULL) {
    s_expression_operation *op_temporaryDestination = expression_Emit(operations, OP_UseTemporaryInstance);
    s_class_instance *new_returnInstance = class_CreateInstance(method->ret_type);
    op_temporaryDestination->payload.temporary = new_returnInstance;
  }

  s_expression_operation *ret = expression_Emit(operations, OP_MethodCall);
  ret->payload.method = method;

  return ret;
}

s_expression_operation *expression_Step(s_compiler *compiler, s_statement *statement, s_scope *scope, s_list *operations, int level) {
	PANALYSIS("expression_Step");
  s_expression_operation *op = NULL;

  // Unary operators
  if (token.type == TOKEN_Literal_Int) {
    s_class_instance *number = class_CreateInstance(LIB_NumberClass);

    s_number *new = NEW(s_number);
    new->isDecimal = false;
    new->content.integer = token.content.integer;

    number->data = new;

    op = expression_Emit(operations, OP_UseTemporaryInstance);
    op->payload.temporary = number;

    match(scope, TOKEN_Literal_Int);

  } else if (token.type == TOKEN_Literal_Real) {
    s_class_instance *number = class_CreateInstance(LIB_NumberClass);

    s_number *new = NEW(s_number);
    new->isDecimal = true;
    new->content.decimal = token.content.decimal;

    number->data = new;

    op = expression_Emit(operations, OP_UseTemporaryInstance);
    op->payload.temporary = number;
    match(scope, TOKEN_Literal_Real);

  } else if (token.type == TOKEN_Literal_String) {
    s_class_instance *string = class_CreateInstance(LIB_StringClass);

    string->data = NEW(s_string);
    s_string *str = (s_string *)string->data;

    string_resize(str, strlen(token.content.string));
    strcpy(str->content, token.content.string);

    op = expression_Emit(operations, OP_UseTemporaryInstance);
    op->payload.temporary = string;

    match(scope, TOKEN_Literal_String);

  } else if (token.type == TOKEN_Return) {
    // Find parent method (return keyword)
    s_statement *parent_statement = statement;
    while (parent_statement->type != STATEMENT_METHOD_DEF) {
      parent_statement = parent_statement->parent;
      if (parent_statement == NULL)
        CERROR(compiler, "expression_Step", "Unable to find parent method.");
    }

    s_method_def *method = parent_statement->body.method_def->method;

    if (method->ret_type == NULL)
      CERROR(compiler, "expression_Step", "Cannot return in a void method.");

    op = expression_Emit(operations, OP_LoadReturn);
    op->payload.symbol = method->ret_type; // Useless ?

    match(scope, TOKEN_Return);
  } else if (token.type == TOKEN_This) {
    // Find parent class (this keyword)
    s_statement *parent_statement = statement;
    while (parent_statement->type != STATEMENT_CLASS_DEF) {
      parent_statement = parent_statement->parent;
      if (parent_statement == NULL)
        CERROR(compiler, "expression_Step", "Unable to find parent class.");
    }

    op = expression_Emit(operations, OP_LoadThis);
    op->payload.symbol = parent_statement->body.class_def->symbol;

    match(scope, TOKEN_This);
  } else if (token.type == TOKEN_Super) {

    match(scope, TOKEN_Super);
  } else if (token.type == TOKEN_Symbol) {
    s_symbol *symbol = token.content.symbol;
    if (symbol->type == SYMBOL_NOTDEFINED)
      CERROR(compiler, "expression_Step", "Symbol not defined.");

    match(scope, TOKEN_Symbol);

    if (token.type == '(') { // Call of Method or Constructor
      // Arguments
      match(scope, '(');

      s_list *args = list_create();

      while (token.type != ')') {
        s_expression_operation *ret = expression_Step(compiler, statement, scope, operations, TOKEN_Assign);
        list_push(args, ret);
      }

      match(scope, ')');

      if (symbol->type == SYMBOL_METHOD) { // Method call
        // Search method overload
        s_method_def *found_overload = method_FindOverload(symbol, args);

        // Emit method call
        op = expression_MethodCall(operations, found_overload);
      } else if (symbol->type == SYMBOL_CLASS) { // Constructor call
        // Search constructor overload
        s_method_def *found_constructor = list_read_first(symbol->body.class->constructors);
        while (found_constructor != NULL) {
          if (found_constructor->arguments->items_count == args->items_count) // TEMPORARY SEARCH FROM ARGUMENTS COUNT
            break;
          found_constructor = list_read_next(symbol->body.class->constructors);
        }

        if (found_constructor == NULL) CERROR(compiler, "expression_Step", "Constructor not found.");

        op = expression_Emit(operations, OP_ConstructorCall);
        op->payload.method = found_constructor;
      } else {
        CERROR(compiler, "expression_Step", "Undefined symbol behaviour.");
      }
    } else { // Symbol access
      op = expression_Emit(operations, OP_AccessSymbol);
      op->payload.symbol = symbol;
    }
  } else if (token.type == '(') {
    match(scope, '(');
    op = expression_Step(compiler, statement, scope, operations, TOKEN_Assign);
    match(scope, ')');
  } else if (token.type == ',') {
    return NULL;
  } else {
    CERROR(compiler, "expression_Step", "Bad expression.");
    exit(-1);
  }

  // binary operator and postfix operators.
  while (token.type >= level) {
    if ((token.type >= TOKEN_Assign) && (token.type <= TOKEN_Dec)) {
      s_token_operator token_operator = token_operators[token.type - TOKEN_Assign];
      match(scope, token.type);

      s_expression_operation *op_arg = expression_Step(compiler, statement, scope, operations, token_operator.sub_token);

      s_list *args = list_create();
      list_push(args, op_arg);

      s_method_def *found_methodOverload = expression_FindMethodOverloadInOperation(op, token_operator.method_name, args);
      op = expression_MethodCall(operations, found_methodOverload);
    } else if (token.type == TOKEN_Link) {
      match(scope, TOKEN_Link);
      s_expression_operation *op_arg = expression_Step(compiler, statement, scope, operations, TOKEN_Cond);

      op = expression_Emit(operations, OP_Link);
    } else if (token.type == TOKEN_Dot) {
      // Descend parent hierarchy
      s_scope *parent_scope = scope;

      s_expression_operation *last_op = op;
      if ((last_op->type == OP_AccessSymbol) || (last_op->type == OP_LoadThis) || (last_op->type == OP_LoadReturn)) {
        s_symbol *op_symbol = last_op->payload.symbol;
        if (op_symbol->type == SYMBOL_FIELD) {
          parent_scope = op_symbol->body.field->value.type->body.class->scope;
        } else if (op_symbol->type == SYMBOL_ARGUMENT) {
          parent_scope = op_symbol->body.argument->value.type->body.class->scope;
        } else if (op_symbol->type == SYMBOL_CLASS) {
          parent_scope = op_symbol->body.class->scope;
        } else if (op_symbol->type == SYMBOL_LOCAL) {
          parent_scope = op_symbol->body.local->value.type->body.class->scope;
        } else {
          CERROR(compiler, "expression_Step", "Access symbol is not valid.");
        }
      } else if (last_op->type == OP_UseTemporaryInstance) {
        parent_scope = last_op->payload.temporary->class->body.class->scope;
      } else if ((last_op->type == OP_MethodCall) || (last_op->type == OP_ConstructorCall)) {
        parent_scope = last_op->payload.method->ret_type->body.class->scope;
      } else {
        CERROR(compiler, "expression_Step", "Nested operation is not valid.");
      }

      match(parent_scope, TOKEN_Dot);
      op = expression_Step(compiler, statement, parent_scope, operations, TOKEN_Dot);
    } else {
      CERROR(compiler, "expression_Step", "Compiler error");
      exit(-1);
    }
  }

  return op;
}

s_statement *compile_Expression(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_Expression");
  s_statement *ret = statement_CreateInside(parent, STATEMENT_EXPRESSION);

  ret->exe_cb = &__core_expression;

  ret->body.expression = NEW(s_statementbody_expression);
  ret->body.expression->operations = list_create();
  
  expression_Step(compiler, ret, ret->scope, ret->body.expression->operations, TOKEN_Assign);

  return ret;
}


s_symbol *class_Create(char *name, s_scope *scope) {
	PANALYSIS("class_Create");
  s_symbol *symbol = symbol_Create(name, SYMBOL_CLASS, -1);

  symbol->body.class = NEW(s_symbolbody_class);
  symbol->body.class->parent = NULL;
  symbol->body.class->scope = scope_Create(scope);
  symbol->body.class->fields = list_create();
  symbol->body.class->methods = list_create();
  symbol->body.class->constructors = list_create();

  scope_AddSymbol(scope, symbol);

  return symbol;
}

s_method_def *class_CreateConstructor(s_symbol *class, void (*cb)(s_class_instance *ret, s_class_instance *self, s_class_instance **args), int nArguments, ...) {
  if (class->type != SYMBOL_CLASS) PERROR("class_CreateConstructor", "Destination is not a Class.");

  va_list valist;
  va_start(valist, nArguments);

  int hash = 1;

  s_method_def *newMethod = NEW(s_method_def);

  newMethod->body.type = METHODBODY_CALLBACK;
  newMethod->body.content.callback = cb;

  // Constructor return itself
  newMethod->ret_type = class;

  // Add arguments
  newMethod->arguments = list_create();
  int i;
  for (i = 0; i < nArguments; i++) {
    char *typeClassName = va_arg(valist, char *);

    if (typeClassName == NULL) { // Literal only, used for Primary classes like Number, String
      list_push(newMethod->arguments, NULL);
      hash = hash * 147 + 255;
    } else {
      s_symbol *symbol_argumentType = symbol_Find(typeClassName, class->body.class->scope->symbols);
      if (symbol_argumentType == NULL) PERROR("class_CreateConstructor", "Class type not found.");

      s_symbol *symbol_argument = symbol_CreateEmpty(SYMBOL_ARGUMENT);
      symbol_argument->body.argument = NEW(s_symbolbody_argument);
      symbol_argument->body.argument->value.type = symbol_argumentType;

      hash = hash * 147 + symbol_argumentType->hash;

      list_push(newMethod->arguments, symbol_argument);
    }
  }

  newMethod->hash = hash;

  // Check already defined Method overload
  s_method_def *m = list_read_first(class->body.class->constructors);
  while (m != NULL) {
    if (hash == m->hash) {
      // ### TODO: Deep check for memory content, not only hash
      PERROR("class_CreateConstructor", "Constructor overload already defined.");
    }
    m = list_read_next(class->body.class->constructors);
  }

  // Add to class constructors
  list_push(class->body.class->constructors, newMethod);

  va_end(valist);
}

s_method_def *class_CreateMethod(s_symbol *class, char *name, void (*cb)(s_class_instance *ret, s_class_instance *self, s_class_instance **args), char *returnType, int nArguments, ...) {
  if (class->type != SYMBOL_CLASS) PERROR("class_CreateMethod", "Destination is not a Class.");
  if (name == NULL) PERROR("class_CreateMethod", "Name cannot be null.");

  s_symbol *symbol_name = symbol_Find(name, class->body.class->scope->symbols);
  if (symbol_name == NULL) {
    // Method symbol not found, create one
    symbol_name = symbol_Create(name, SYMBOL_METHOD, -1);
    symbol_name->body.method = NEW(s_symbolbody_method);
    symbol_name->body.method->overloads = list_create();
    // Add to class scope
    scope_AddSymbol(class->body.class->scope, symbol_name);
  }
  
  if (!symbol_name->isUppercase) PERROR("class_CreateMethod", "Method name must start uppercase.");

  va_list valist;
  va_start(valist, nArguments);

  int hash = symbol_name->hash * 147;

  s_method_def *newMethod = NEW(s_method_def);

  newMethod->body.type = METHODBODY_CALLBACK;
  newMethod->body.content.callback = cb;

  // Add arguments
  newMethod->arguments = list_create();
  int i;
  for (i = 0; i < nArguments; i++) {
    char *typeClassName = va_arg(valist, char *);

    if (typeClassName == NULL) { // Literal only, used for Primary classes like Number, String
      list_push(newMethod->arguments, NULL);
      hash = hash * 147 + 255;
    } else {
      s_symbol *symbol_argumentType = symbol_Find(typeClassName, class->body.class->scope->symbols);
      if (symbol_argumentType == NULL) PERROR("class_CreateMethod", "Class type not found.");

      s_symbol *symbol_argument = symbol_CreateEmpty(SYMBOL_ARGUMENT);
      symbol_argument->body.argument = NEW(s_symbolbody_argument);
      symbol_argument->body.argument->value.type = symbol_argumentType;

      hash = hash * 147 + symbol_argumentType->hash;

      list_push(newMethod->arguments, symbol_argument);
    }
  }

  // Find return type
  s_symbol *symbol_returnType = NULL;
  if (returnType != NULL) {
    symbol_returnType = symbol_Find(returnType, class->body.class->scope->symbols);
    hash = hash * 147 + symbol_returnType->hash;
  }

  newMethod->ret_type = symbol_returnType;

  newMethod->hash = hash;

  // Check already defined Method overload
  s_method_def *m = list_read_first(symbol_name->body.method->overloads);
  while (m != NULL) {
    if (hash == m->hash) {
      // ### TODO: Deep check for memory content, not only hash
      PERROR("class_CreateConstructor", "Constructor overload already defined.");
    }
    m = list_read_next(symbol_name->body.method->overloads);
  }

  // Push to overloads
  list_push(symbol_name->body.method->overloads, newMethod);

  // Add to class methods
  list_push(class->body.class->methods, newMethod);

  va_end(valist);
}

s_class_instance *class_CreateInstance(s_symbol *class) {
	PANALYSIS("class_CreateInstance");
  if (class->type != SYMBOL_CLASS) PERROR("class_CreateInstance", "Wrong statement type");

  s_class_instance *instance = NEW(s_class_instance);
  instance->class = class;

  // Allocate data space
  instance->data = (s_class_instance **)malloc(sizeof(s_class_instance *) * instance->class->body.class->fields->items_count);

  // Initialize fields
  s_symbol *field_symbol = list_read_first(instance->class->body.class->fields);
  while (field_symbol != NULL) {
    s_class_instance *init_value = __core_exe_expression(EXE_SCOPE(NULL, instance, field_symbol->body.field->init_expression));
    instance->data[field_symbol->body.field->value.data_index] = init_value;

    field_symbol = list_read_next(instance->class->body.class->fields);
  }

  return instance;
}


s_method_def *method_FindOverload(s_symbol *method, s_list *args) {
	PANALYSIS("method_FindOverload");
  if (method == NULL) PERROR("method_FindOverload", "Symbol cannot be null.");
  if (method->type != SYMBOL_METHOD) PERROR("method_FindOverload", "Symbol is not a Method.");
  if (args == NULL) PERROR("method_FindOverload", "Args cannot be null.");

  s_method_def *found_overload = list_read_first(method->body.method->overloads);
  while (found_overload != NULL) {
    if (found_overload->arguments->items_count == args->items_count) { // TEMPORARY SEARCH FROM ARGUMENTS COUNT
      bool argsMatching = true;

      s_list_item *pos_MethodArg = list_get_first(found_overload->arguments);
      s_list_item *pos_CallArg = list_get_first(args);
      u_int64_t idx;
      for (idx = 0; idx < args->items_count; idx++) {
        s_symbol *methodArg_symbol = pos_MethodArg->payload;
        pos_MethodArg = list_get_next(pos_MethodArg);
        
        s_expression_operation *callArg_expressionOperation = pos_CallArg->payload;
        s_symbol *callArg_symbol = expression_GetClassOfOperation(callArg_expressionOperation);
        pos_CallArg = list_get_next(pos_CallArg);

        if (methodArg_symbol->body.argument->value.type != callArg_symbol) {
          argsMatching = false;
          break;
        }
      }

      if (argsMatching)
        break;
    }
    found_overload = list_read_next(method->body.method->overloads);
  }

  if (found_overload == NULL)
    PERROR("method_FindOverload", "Method overload not found.");

  return found_overload;
}

s_method_def *class_FindMethodByName(s_symbol *class, char *name, s_list *args) {
	PANALYSIS("class_FindMethodByName");
  if (class == NULL) PERROR("method_FindOverload", "Symbol cannot be null.");
  if (class->type != SYMBOL_CLASS) PERROR("class_FindMethodByName", "Symbol \"%s\" is not a class.", name);
  if (name == NULL) PERROR("class_FindMethodByName", "Name cannot be null.");

  s_symbol *found_symbol = symbol_Find(name, class->body.class->scope->symbols);
  if (found_symbol == NULL) PERROR("class_FindMethodByName", "Symbol \"%s\" does not exists.", name);
  
  return method_FindOverload(found_symbol, args);
}

void compile_ClassBody(s_compiler *compiler, s_statement *class) {
	PANALYSIS("compile_ClassBody");
  // Only 3 definition statements are allowed in class body
  // 1. field : type = <expression>;
  // 2. method(...) : type { <statement> }
  // 3. class { <statement> }

  s_statementbody_class_def *class_body = class->body.class_def;

  while ((token.type > 0) && (token.type != '}') ) {
    s_statement *statement = compile_DefinitionStatement(compiler, class);
    if (statement != NULL) { // If not empty statement
      if (statement->type == STATEMENT_FIELD_DEF) {
        s_symbol *field_symbol = statement->body.field_def->symbol;
        s_list *fields_list = class_body->symbol->body.class->fields;
        field_symbol->body.field->value.data_index = fields_list->items_count;
        list_push(fields_list, field_symbol);
      } else if (statement->type == STATEMENT_METHOD_DEF) {
        list_push(class_body->symbol->body.class->methods, statement->body.method_def->symbol);
      } else if (statement->type == STATEMENT_CONSTRUCTOR_DEF) {
        list_push(class_body->symbol->body.class->constructors, statement->body.constructor_def->method);
      } else if (statement->type == STATEMENT_CLASS_DEF) {
        // Do nothing
      } else {
        CERROR(compiler, "compile_ClassBody", "Statement not allowed in class definition.");
      }
    }
  }
}

s_statement *compile_ClassDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_ClassDefinition");
  if (parent->type != STATEMENT_CLASS_DEF) CERROR(compiler, "compiler_ClassDefinition", "Wrong statement for class definition.");
  if (parent->body.class_def->symbol->type != SYMBOL_CLASS) CERROR(compiler, "compiler_ClassDefinition", "Wrong parent symbol.");

  s_symbol *class_symbol = token.content.symbol;
  s_symbol *parent_symbol = parent->body.class_def->symbol; // Direct Inheritance

  match(parent->scope, TOKEN_Symbol);

  if (token.type == TOKEN_Dot) { // External Inheritance
    // ClassA.ClassB...ClassC.Children {}
    match(parent->scope, TOKEN_Dot);
  }

  s_statement *ret = NULL;

  if ((class_symbol->type > SYMBOL_NOTDEFINED) && (class_symbol->type != SYMBOL_CLASS)) CERROR(compiler, "compiler_ClassDefinition", "Symbol already defined.");

  if (class_symbol->type == SYMBOL_NOTDEFINED) {
    ret = statement_CreateChildren(parent, STATEMENT_CLASS_DEF);

    class_symbol->type = SYMBOL_CLASS;
    class_symbol->body.class = NEW(s_symbolbody_class);
    class_symbol->body.class->parent = parent_symbol;
    class_symbol->body.class->fields = list_create();
    class_symbol->body.class->methods = list_create();
    class_symbol->body.class->constructors = list_create();
    class_symbol->body.class->scope = ret->scope;
  } else {
    ret = statement_Create(parent, class_symbol->body.class->scope, STATEMENT_CLASS_DEF);
  }

  ret->body.class_def = NEW(s_statementbody_class_def);
  ret->body.class_def->symbol = class_symbol;

  if (token.type == ':') { // Derivation (multiple classes allowed)
    // MixClass : ClassA, ClassB, ClassC, ... {}
    match(parent->scope, ':');

    while (token.type != '{') {
      s_symbol *ex_class = token.content.symbol;
      if (ex_class->type != SYMBOL_CLASS) CERROR(compiler, "compile_ClassDefinition", "Derivation symbol is not a class.");

      s_list *l = NULL;
      s_list_item *l_item = NULL;
      u_int64_t idx;

      // Create sub scope
      ret->scope = scope_Create(ex_class->body.class->scope);
      class_symbol->body.class->scope = ret->scope;

      // Copy fields
      l = ex_class->body.class->fields;
      l_item = list_get_first(l);
      for (idx = 0; idx < l->items_count; idx++) {
        list_push(class_symbol->body.class->fields, l_item->payload);
        l_item = list_get_next(l_item);
      }

      // Copy methods
      l = ex_class->body.class->methods;
      l_item = list_get_first(l);
      for (idx = 0; idx < l->items_count; idx++) {
        list_push(class_symbol->body.class->methods, l_item->payload);
        l_item = list_get_next(l_item);
      }

      // Copy constructors
      l = ex_class->body.class->constructors;
      l_item = list_get_first(l);
      for (idx = 0; idx < l->items_count; idx++) {
        list_push(class_symbol->body.class->constructors, l_item->payload);
        l_item = list_get_next(l_item);
      }

      match(parent->scope, TOKEN_Symbol);
      if (token.type == ',')
        match(parent->scope, ',');
    }
  }

  match(ret->scope, '{');

  compile_ClassBody(compiler, ret);

  match(parent->scope, '}');

  return ret;
}

// ([arg0, arg1, ...]) { <statements> }
s_statement *compile_ConstructorMethodDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_ConstructorMethodDefinition");
  if (parent->type != STATEMENT_CLASS_DEF) CERROR(compiler, "compile_MethodDefinition", "Wrong parent statement for method definition.");
  if (parent->body.class_def->symbol->type != SYMBOL_CLASS) CERROR(compiler, "compile_MethodDefinition", "Wrong parent symbol.");

  int hash = 1;

  s_statement *ret = statement_CreateInside(parent, STATEMENT_CONSTRUCTOR_DEF);
  ret->body.constructor_def = NEW(s_statementbody_constructor_def);

  s_method_def *newMethod = NEW(s_method_def);
  ret->body.constructor_def->method = newMethod;

  s_statement *ret_statement = statement_CreateBlock(parent);
  newMethod->body.type = METHODBODY_STATEMENT;
  newMethod->body.content.statement = ret_statement;

  // Constructor return itself
  newMethod->ret_type = parent->body.class_def->symbol;

  // Read arguments
  match(ret_statement->scope, '(');

  newMethod->arguments = list_create();
  while (token.type != ')') {
    s_statement *arg_statement = compile_ArgumentDefinition(compiler, ret_statement);

    list_push(newMethod->arguments, arg_statement->body.argument_def->symbol);

    hash = hash * 147 + arg_statement->body.argument_def->symbol->body.field->value.type->hash;

    if (token.type == ',')
      match(ret_statement->scope, ',');
  }

  match(parent->scope, ')');

  newMethod->hash = hash;

  // Check already defined Method overload
  s_list *l = parent->body.class_def->symbol->body.class->constructors;
  s_list_item *li = list_get_first(l);
  u_int64_t idx;
  for (idx = 0; idx < l->items_count; idx++) {
    s_method_def *m = li->payload;

    if (hash == m->hash) {
      // ### TODO: Deep check for memory content, not only hash
      // CERROR(compiler, "compile_MethodDefinition", "Constructor overload already defined.");

      // Removed old overload and exit
      list_remove_item(l, li);
      break;
    }

    li = list_get_next(li);
  }

  // Get body statement for method
  match(ret_statement->scope, '{');

  while (token.type != '}') {
    s_statement *statement = compile_Statement(compiler, ret_statement);
    list_push(ret_statement->body.block->statements, statement);
  }

  match(parent->scope, '}');

  return ret;
}

// name([arg0, arg1, ...]) [: <return type>] { <statements> }
s_statement *compile_MethodDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_MethodDefinition");
  if (parent->type != STATEMENT_CLASS_DEF) CERROR(compiler, "compile_MethodDefinition", "Wrong parent statement for method definition.");
  if (parent->body.class_def->symbol->type != SYMBOL_CLASS) CERROR(compiler, "compile_MethodDefinition", "Wrong parent symbol.");

  s_symbol *name = token.content.symbol;
  if (!name->isUppercase) CERROR(compiler, "compile_MethodDefinition", "Method definition must start uppercase.");

  if ((name->type > SYMBOL_NOTDEFINED) && (name->type != SYMBOL_METHOD)) CERROR(compiler, "compile_MethodDefinition", "Symbol already defined.");

  int hash = name->hash * 147;

  s_statement *ret = statement_CreateInside(parent, STATEMENT_METHOD_DEF);
  ret->body.method_def = NEW(s_statementbody_method_def);
  ret->body.method_def->symbol = name;

  match(parent->scope, TOKEN_Symbol);

  if (name->type == SYMBOL_NOTDEFINED) {
    name->type = SYMBOL_METHOD;
    name->body.method = NEW(s_symbolbody_method);
    name->body.method->overloads = list_create();
  }

  s_method_def *newMethod = NEW(s_method_def);
  ret->body.method_def->method = newMethod;

  s_statement *ret_statement = statement_CreateBlock(ret);
  newMethod->body.type = METHODBODY_STATEMENT;
  newMethod->body.content.statement = ret_statement;

  // Read arguments
  match(ret_statement->scope, '(');

  newMethod->arguments = list_create();
  while (token.type != ')') {
    s_statement *arg_statement = compile_ArgumentDefinition(compiler, ret_statement);

    list_push(newMethod->arguments, arg_statement->body.argument_def->symbol);

    hash = hash * 147 + arg_statement->body.argument_def->symbol->body.field->value.type->hash;

    if (token.type == ',')
      match(ret_statement->scope, ',');
  }

  match(parent->scope, ')');

  if (token.type == ':') {
    match(parent->scope, ':');
    // Return type
    newMethod->ret_type = compile_FieldType(compiler, parent);

    hash = hash * 147 + newMethod->ret_type->hash;
  }

  newMethod->hash = hash;

  // Check already defined Method overload
  s_method_def *m = list_read_first(name->body.method->overloads);
  while (m != NULL) {
    if (hash == m->hash) {
      // ### TODO: Deep check for memory content, not only hash
      CERROR(compiler, "compile_MethodDefinition", "Method overload already defined.");
    }
    m = list_read_next(name->body.method->overloads);
  }

  // Push to overloads
  list_push(name->body.method->overloads, newMethod);

  // Get body statement for method
  match(ret_statement->scope, '{');

  while (token.type != '}') {
    s_statement *statement = compile_Statement(compiler, ret_statement);
    list_push(ret_statement->body.block->statements, statement);
  }

  match(parent->scope, '}');

  return ret;
}

s_anytype *compile_FieldType(s_compiler *compiler, s_statement *statement) {
	PANALYSIS("compile_FieldType");
  s_anytype *ret = NULL;

  if (token.type == TOKEN_Symbol) {
    // Class
    s_symbol *symbol = token.content.symbol;
    if (!symbol->isUppercase) CERROR(compiler, "compile_FieldType", "Symbol is not a class");
    if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldType", "Symbol is not defined");

    ret = symbol;

    match(statement->scope, TOKEN_Symbol);

    // Childrens
    while (token.type == TOKEN_Dot) {
      s_symbolbody_class *symbol_body = symbol->body.class;
      match(symbol_body->scope, TOKEN_Dot);

      symbol = token.content.symbol;
      if (!symbol->isUppercase) CERROR(compiler, "compile_FieldType", "Symbol is not a class");
      if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldType", "Symbol is not defined");

      ret = symbol;

      match(symbol_body->scope, TOKEN_Symbol);
    }
  } else if (token.type == '[') {
    // List
    match(statement->scope, '[');

    // TODO

    match(statement->scope, ']');
  } else if (token.type == '{') {
    // Dictionary
    match(statement->scope, '{');

    // TODO

    match(statement->scope, ',');

    // TODO

    match(statement->scope, '}');
  } else {
    CERROR(compiler, "compile_FieldType", "Unsupported complex field definition");
    exit(-1);
  }

  return ret;
}


s_statement *compile_ArgumentDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_ArgumentDefinition");
  s_symbol *name = token.content.symbol;
  if (name->type != SYMBOL_NOTDEFINED) CERROR(compiler, "compile_ArgumentDefinition", "Symbol already defined.");

  s_statement *ret = statement_CreateInside(parent, STATEMENT_ARGUMENT_DEF);
  ret->exe_cb = &__core_argument_def;

  match(ret->scope, TOKEN_Symbol);

  ret->body.argument_def = NEW(s_statementbody_argument_def);
  ret->body.argument_def->symbol = name;

  match(ret->scope, ':');

  name->body.argument = NEW(s_symbolbody_argument);
  name->type = SYMBOL_ARGUMENT;

  name->body.argument->value.type = compile_FieldType(compiler, ret);
  name->body.argument->value.data = NULL;
  name->body.argument->init_expression = NULL;

  if (token.type == TOKEN_Assign) {
    match(ret->scope, TOKEN_Assign);

    name->body.argument->init_expression = compile_Expression(compiler, ret);
  }

  return ret;
}

s_statement *compile_FieldDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_FieldDefinition");

  s_symbol *name = token.content.symbol;
  if (name->type != SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldDefinition", "Symbol already defined.");

  s_statement *ret = statement_CreateInside(parent, STATEMENT_FIELD_DEF);
  ret->exe_cb = &__core_field_def;

  match(ret->scope, TOKEN_Symbol);

  ret->body.field_def = NEW(s_statementbody_field_def);
  ret->body.field_def->symbol = name;

  match(ret->scope, ':');

  name->body.field = NEW(s_symbolbody_field);
  name->type = SYMBOL_FIELD;

  name->body.field->value.type = compile_FieldType(compiler, ret);
  name->body.field->init_expression = NULL;

  if (token.type == TOKEN_Assign) {
    match(ret->scope, TOKEN_Assign);

    name->body.field->init_expression = compile_Expression(compiler, ret);
  }

  if (ret->body.field_def->symbol->body.field->init_expression == NULL) CERROR(compiler, "compile_FieldDefinition", "Field must have an initiliazation value.");

  return ret;
}

s_statement *compile_LocalFieldDefinition(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_LocalFieldDefinition");

  s_symbol *name = token.content.symbol;
  if (name->type != SYMBOL_NOTDEFINED) CERROR(compiler, "compile_LocalFieldDefinition", "Symbol already defined.");

  s_statement *ret = statement_CreateInside(parent, STATEMENT_LOCAL_DEF);
  ret->exe_cb = &__core_local_def;

  match(ret->scope, TOKEN_Symbol);

  // Create statement body
  ret->body.local_def = NEW(s_statementbody_local_def);
  ret->body.local_def->symbol = name;

  match(ret->scope, ':');

  // Create symbol body
  name->body.local = NEW(s_symbolbody_local);
  name->type = SYMBOL_LOCAL;

  name->body.local->value.type = compile_FieldType(compiler, ret);
  name->body.local->value.instances = list_create();
  list_push(name->body.local->value.instances, class_CreateInstance(name->body.local->value.type));

  // Init expression
  name->body.local->init_expression = NULL;
  if (token.type == TOKEN_Assign) {
    match(ret->scope, TOKEN_Assign);

    name->body.local->init_expression = compile_Expression(compiler, ret);
  }
  if (ret->body.local_def->symbol->body.local->init_expression == NULL) CERROR(compiler, "compile_LocalFieldDefinition", "Field must have an initiliazation value.");

  // Push to temporaries list in parent statement (used for garbage collection)
  list_push(parent->temporaries, name);

  return ret;
}

s_statement *compile_If(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_If");
  s_statement *ret = statement_CreateInside(parent, STATEMENT_IF);
  ret->exe_cb = &__core_if;

  match(ret->scope, TOKEN_If);

  ret->body._if = NEW(s_statementbody_if);

  ret->body._if->check = compile_Expression(compiler, ret);
  ret->body._if->_true = compile_Statement(compiler, ret);

  return ret;
}

s_statement *compile_For(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_For");
  s_statement *ret = statement_CreateInside(parent, STATEMENT_FOR);

  ret->body._for = NEW(s_statementbody_for);

  // Init statement
  ret->body._for->check = compile_Expression(compiler, ret);

  match(ret->scope, ';');

  return ret;
}

s_statement *compile_While(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_While");
  s_statement *ret = statement_CreateInside(parent, STATEMENT_WHILE);
  ret->exe_cb = &__core_while;

  match(ret->scope, TOKEN_While);

  ret->body._while = NEW(s_statementbody_while);

  ret->body._while->check = compile_Expression(compiler, ret);
  ret->body._while->loop = compile_Statement(compiler, ret);

  return ret;
}

s_statement *compile_DefinitionStatement(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_DefinitionStatement");
  // Definition statements types:
  // 1. field : type = <expression>;
  // 2. method(...) : type { <statement> }
  // 3. class { <statement> }
  // 4. <empty statement>;

  s_statement *ret = NULL;

  if (token.type == ';') {
    // Empty statement
    match(parent->scope, ';');
  } else if (token.type == TOKEN_Symbol) {
    if (token.content.symbol->isUppercase) { // Method or Class
      s_token next_token = preview(parent->scope);
      if (next_token.type == '(') {
        // Method definition
        ret = compile_MethodDefinition(compiler, parent);
      } else {
        // Class definition
        ret = compile_ClassDefinition(compiler, parent);
      }
    } else if (token.content.symbol->isFullcase) { // Constant property

    } else if (token.content.symbol->startsUnderscore) { // Private property

    } else {
      // Field definition
      ret = compile_FieldDefinition(compiler, parent);
      match(parent->scope, ';');
    }
  } else if (token.type == '(') {
    // Constructor method definition
    ret = compile_ConstructorMethodDefinition(compiler, parent);
  } else {
    CERROR(compiler, "compile_DefinitionStatement", "Statement not valid.");
  }

  return ret;
}

s_statement *compile_Statement(s_compiler *compiler, s_statement *parent) {
	PANALYSIS("compile_Statement");
  // Statements types:
  // 1. if (...) <statement> [else <statement>]
  // 2. for (...) <statement>
  // 3. while (...) <statement>
  // 4. { <statement> }
  // 5. return xxx;
  // 6. <empty statement>;
  // 7. expression; (expression end with semicolon)
  // 8. field : type = <expression>;

  s_statement *ret = NULL;

  if (token.type == TOKEN_If) {
    ret = compile_If(compiler, parent);
  } else if (token.type == TOKEN_For) {

  } else if (token.type == TOKEN_Debug) {
    ret = statement_CreateInside(parent, STATEMENT_DEBUG);
    ret->exe_cb = &__core_debug;
    match(ret->scope, TOKEN_Debug);
  } else if (token.type == TOKEN_While) {
    ret = compile_While(compiler, parent);
  } else if (token.type == '{') {
    // Block statement
    match(parent->scope, '{');

    ret = statement_CreateBlock(parent);

    while (token.type != '}') {
      s_statement *statement = compile_Statement(compiler, ret);
      list_push(ret->body.block->statements, statement);
    }

    match(parent->scope, '}');
  } else if (token.type == ';') {
    // Empty statement
    match(parent->scope, ';');
  } else if (token.type == TOKEN_Symbol) {
    s_token next_token = preview(parent->scope);

    if (next_token.type == ':') {
      // Field definition
      ret = compile_LocalFieldDefinition(compiler, parent);
      match(parent->scope, ';');
    } else {
      // Expression (espresso ?)
      ret = compile_Expression(compiler, parent);
      match(parent->scope, ';');
    }
  } else {
    // Expression (espresso ?)
    ret = compile_Expression(compiler, parent);
    match(parent->scope, ';');
  }

  return ret;
}


#undef emit
#undef preview
#undef match
#undef token

/* ##### CORE Libs ##### */
e_statementend __core_field_def(s_exe_scope exe) {
	PANALYSIS("__core_field_def");
  return STATEMENT_END_CONTINUE;
}

e_statementend __core_argument_def(s_exe_scope exe) {
	PANALYSIS("__core_argument_def");
  return STATEMENT_END_CONTINUE;
}

e_statementend __core_local_def(s_exe_scope exe) {
	PANALYSIS("__core_local_def");

  s_list *instances = exe.statement->body.local_def->symbol->body.local->value.instances;

  if (instances->selected_item == NULL) {
    list_read_first(instances);
  } else {
    s_class_instance *ret = list_read_next(instances);
    if (ret == NULL) {
      ret = class_CreateInstance(exe.statement->body.local_def->symbol->body.local->value.type);
      list_push(instances, ret);
      list_read_last(instances);
    }
  }

  return STATEMENT_END_CONTINUE;
}

e_statementend __core_expression(s_exe_scope exe) {
	PANALYSIS("__core_expression");
  __core_exe_expression(exe);
  return STATEMENT_END_CONTINUE;
}

e_statementend __core_exe_statement(s_exe_scope exe) {
	PANALYSIS("__core_exe_statement");
  u_int64_t pre_stack_ptr = stack.ptr;

  e_statementend ret = STATEMENT_END_CONTINUE;

  if (exe.statement != NULL) {
    if (exe.statement->type == STATEMENT_BLOCK) {
      s_statementbody_block *statement_body = exe.statement->body.block;
      
      s_list_item *pos = list_get_first(statement_body->statements);
      u_int64_t idx;
      for (idx = 0; idx < statement_body->statements->items_count; idx++) {
        s_statement *sub_statement = pos->payload;
        pos = list_get_next(pos);

        __core_exe_statement(SUB_EXE_SCOPE(exe, sub_statement));
        if (ret != STATEMENT_END_CONTINUE)
          break;
      }
    } else {
      if (exe.statement->exe_cb)
        ret = exe.statement->exe_cb(exe);
    }

    s_symbol *temp_symbol = list_read_first(exe.statement->temporaries);
    while (temp_symbol != NULL) {
      if (temp_symbol->type != SYMBOL_LOCAL) PERROR("__core_exe_statement", "Symbol is not local.");

      list_read_previous(temp_symbol->body.local->value.instances);

      temp_symbol = list_read_next(exe.statement->temporaries);
    }
  }

  stack.ptr = pre_stack_ptr;

  return ret;
}

e_statementend __core_debug(s_exe_scope exe) {
	PANALYSIS("__core_debug");
  int a = 0;
  return STATEMENT_END_CONTINUE;
}

e_statementend __core_if(s_exe_scope exe) {
	PANALYSIS("__core_if");
  if (exe.statement->type != STATEMENT_IF) PERROR("__core_while", "Wrong statement type");

  s_statementbody_if *statement_body = exe.statement->body._if;

  s_class_instance *check_result = __core_exe_expression(SUB_EXE_SCOPE(exe, statement_body->check));

  if (check_result->data) {
    e_statementend ret = __core_exe_statement(SUB_EXE_SCOPE(exe, statement_body->_true));
  } else {
    if (statement_body->_false) {
      e_statementend ret = __core_exe_statement(SUB_EXE_SCOPE(exe, statement_body->_false));
    }
  }

  return STATEMENT_END_CONTINUE;
}

e_statementend __core_while(s_exe_scope exe) {
	PANALYSIS("__core_while");
  if (exe.statement->type != STATEMENT_WHILE) PERROR("__core_while", "Wrong statement type");

  s_statementbody_while *statement_body = exe.statement->body._while;

  s_class_instance *check_result = __core_exe_expression(SUB_EXE_SCOPE(exe, statement_body->check));

  s_number *num_result = (s_number *)check_result->data;

  while (num_result->content.integer) {
    e_statementend ret = __core_exe_statement(SUB_EXE_SCOPE(exe, statement_body->loop));
    if (ret != STATEMENT_END_CONTINUE)
      break;

    check_result = __core_exe_expression(SUB_EXE_SCOPE(exe, statement_body->check));
  }

  return STATEMENT_END_CONTINUE;
}

void __exe_method(s_class_instance *self, s_method_def *method, s_class_instance *return_instance, s_class_instance **args) {
	PANALYSIS("__exe_method");
  if (method->body.type == METHODBODY_STATEMENT) {
    // Map arguments value
    u_int64_t arg_index = 0;
    s_symbol *arg = list_read_first(method->arguments);
    while (arg != NULL) {
      arg->body.argument->value.data = args[arg_index];
      arg = list_read_next(method->arguments);
      arg_index++;
    }
    e_statementend ret = __core_exe_statement(EXE_SCOPE(return_instance, self, method->body.content.statement));
  } else if (method->body.type == METHODBODY_CALLBACK) {
    method->body.content.callback(return_instance, self, args);
  } else {
    PERROR("__exe_method", "Method type error.");
  }
}


void __exe_initializeFieldInInstance(s_class_instance *instance, s_symbol *field_symbol) {
  
}

s_class_instance *__core_exe_expression(s_exe_scope exe) {
	PANALYSIS("__core_exe_expression");
  if (exe.statement->type != STATEMENT_EXPRESSION) PERROR("__core_exe_expression", "Wrong statement type");

  s_list *operations = exe.statement->body.expression->operations;

  s_class_instance *args[32];

  u_int64_t pre_stack_ptr = stack.ptr;

  s_list_item *pos = list_get_first(operations);
  u_int64_t idx = 0;
  for (idx = 0; idx < operations->items_count; idx++) {
    s_expression_operation *op = pos->payload;
    pos = list_get_next(pos);

    if (op->type == OP_AccessSymbol) {
      s_class_instance *value = NULL;
      if (op->payload.symbol->type == SYMBOL_ARGUMENT) {
        value = op->payload.symbol->body.argument->value.data;
      } else if (op->payload.symbol->type == SYMBOL_FIELD) {
        s_class_instance *target = stack_pop__s_class_instance_ptr(&stack);
        value = target->data[op->payload.symbol->body.field->value.data_index];
      } else if (op->payload.symbol->type == SYMBOL_LOCAL) {
        value = (s_class_instance *)list_read_selected(op->payload.symbol->body.local->value.instances);
      } else {
        PERROR("__core_exe_expression", "Access symbol not allowed.");
      }

      stack_push__s_class_instance_ptr(&stack, value);
    } else if (op->type == OP_UseTemporaryInstance) {
      stack_push__s_class_instance_ptr(&stack, op->payload.temporary);
    } else if (op->type == OP_LoadReturn) {
      stack_push__s_class_instance_ptr(&stack, exe.ret);
    } else if (op->type == OP_LoadThis) {
      stack_push__s_class_instance_ptr(&stack, exe.self);
    } else if (op->type == OP_Link) {
      s_class_instance *src = stack_pop__s_class_instance_ptr(&stack);
      s_class_instance *target = stack_pop__s_class_instance_ptr(&stack);

      if (src->class != target->class) PERROR("__core_exe_expression", "Cast not allowed for link.");

      target->data = src->data;
    } else if ((op->type == OP_MethodCall) || (op->type == OP_ConstructorCall)) {
      // Pop return instance from stack (or create new one in case of constructor)
      s_class_instance *ret = NULL;

      if (op->type == OP_MethodCall) {
        if (op->payload.method->ret_type != NULL) {
          ret = stack_pop__s_class_instance_ptr(&stack);
        }
      } else if (op->type == OP_ConstructorCall) {
        ret = class_CreateInstance(op->payload.method->ret_type);
      }

      // Pop arguments from stack
      u_int64_t args_count = op->payload.method->arguments->items_count;
      
      int64_t arg_idx;
      for (arg_idx = 0; arg_idx < args_count; arg_idx++) {
        s_class_instance *arg_value = stack_pop__s_class_instance_ptr(&stack);
        args[arg_idx] = arg_value;
      }

      // Pop target and call method
      s_class_instance *target = NULL;
      if (op->type == OP_MethodCall) {
        target = stack_pop__s_class_instance_ptr(&stack);
        __exe_method(target, op->payload.method, ret, args);
      } else if (op->type == OP_ConstructorCall) {
        __exe_method(ret, op->payload.method, ret, args);
      }

      // Push return instance (if present)
      if (ret != NULL)
        stack_push__s_class_instance_ptr(&stack, ret);
    } else {
      PERROR("__core_exe_expression", "Operation not allowed.");
    }
  }

  if ((stack.ptr - pre_stack_ptr) > 1)
    PERROR("__core_exe_expression", "Stack error.");

  if (stack.ptr > 0)
    return stack_pop__s_class_instance_ptr(&stack);
  else
    return NULL;
}

/* ##### SDK teardown ##### */
s_class_instance *sdk_execute_method(s_class_instance *target, s_method_def *method) {

}

/* ##### Generic object LIB ##### */
void object_Assign(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("object_Assign");

}

void object_Print(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("object_Print");
}

void object_ToString(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("object_ToString");
}

/* ##### STDLIB 3rd avenue ##### */
void number_Constructor(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Constructor");
  self->data = NEW(s_number);
}
void number_Constructor_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Constructor");
  s_number *num_B = (s_number *)args[0]->data;

  s_number *new = NEW(s_number);
  new->isDecimal = num_B->isDecimal;
  new->content = num_B->content;

  self->data = new;
}

void number_Add_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Add_Number");

  s_number *num_B = (s_number *)args[0]->data;
  s_number *num_self = (s_number *)self->data;
  s_number *num_ret = (s_number *)ret->data;

  bool anyDecimal = num_self->isDecimal || num_B->isDecimal;
  bool allDecimal = num_self->isDecimal && num_B->isDecimal;

  num_ret->isDecimal = anyDecimal;

  if (!num_ret->isDecimal) {
    num_ret->content.integer = num_self->content.integer + num_B->content.integer;
  } else if (allDecimal) {
    num_ret->content.decimal = num_self->content.decimal + num_B->content.decimal;
  } else {
    double dec_self = num_self->isDecimal ? num_self->content.decimal : (double)num_self->content.integer;
    double dec_B = num_B->isDecimal ? num_B->content.decimal : (double)num_B->content.integer;
    num_ret->content.decimal = dec_self + dec_B;
  }
}

void number_Sub_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Sub_Number");
  s_number *num_B = (s_number *)args[0]->data;
  s_number *num_self = (s_number *)self->data;
  s_number *num_ret = (s_number *)ret->data;

}

void number_Mul_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Mul_Number");
  s_number *num_B = (s_number *)args[0]->data;
  s_number *num_self = (s_number *)self->data;
  s_number *num_ret = (s_number *)ret->data;

}

void number_Less_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Less_Number");
  s_number *num_B = (s_number *)args[0]->data;
  s_number *num_self = (s_number *)self->data;
  s_number *num_ret = (s_number *)ret->data;

  bool anyDecimal = num_self->isDecimal || num_B->isDecimal;

  num_ret->isDecimal = false;

  if (!anyDecimal) {
    num_ret->content.integer = num_self->content.integer < num_B->content.integer;
  } else {
    double dec_self = num_self->isDecimal ? num_self->content.decimal : (double)num_self->content.integer;
    double dec_B = num_B->isDecimal ? num_B->content.decimal : (double)num_B->content.integer;
    num_ret->content.decimal = dec_self < dec_B;
  }
}

void number_Assign_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Assign_Number");
  s_number *num_B = (s_number *)args[0]->data;
  s_number *num_self = (s_number *)self->data;
  s_number *num_ret = (s_number *)ret->data;

  num_self->isDecimal = num_B->isDecimal;
  num_self->content = num_B->content;
}

void number_Print(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_Print");
  s_number *num_self = (s_number *)self->data;
  if (num_self->isDecimal)
    printf("%lf\n", num_self->content.decimal);
  else
    printf("%lu\n", num_self->content.integer);
}

void number_ToString(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("number_ToString");
  s_number *num_self = (s_number *)self->data;

  s_string *str_ret = (s_string *)ret->data;
  string_resize(str_ret, 20);

  if (num_self->isDecimal)
    sprintf(str_ret->content, "%lf", num_self->content.decimal);
  else
    sprintf(str_ret->content, "%ld", num_self->content.integer);
}

void string_resize(s_string *str, u_int64_t len) {
  u_int32_t old_blocks = (str->len / 2048) + 1;
  u_int32_t new_blocks = (len / 2048) + 1;

  str->len = len;

  if ((old_blocks < new_blocks) || (str->content == NULL)) {
    free(str->content);
    str->content = (char *)malloc(new_blocks * 2048);
  }
}

void string_Constructor(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Constructor");
  self->data = NEW(s_string);
}
void string_Constructor_String(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Constructor");

  self->data = NEW(s_string);
  s_string *str_self = (s_string *)self->data;
  s_string *str_B = (s_string *)args[0]->data;

  string_resize(str_self, str_B->len);
  strcpy(str_self->content, str_B->content);
}

void string_Assign_String(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Assign_String");
  s_string *str_self = (s_string *)self->data;
  s_string *str_B = (s_string *)args[0]->data;

  string_resize(str_self, str_B->len);
  strcpy(str_self->content, str_B->content);
}

void string_Add_String(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Add_String");
  s_string *str_ret = (s_string *)ret->data;
  s_string *str_self = (s_string *)self->data;
  s_string *str_B = (s_string *)args[0]->data;

  u_int64_t len = str_self->len + str_B->len;

  string_resize(str_ret, len);
  strcpy(str_ret->content, str_self->content);
  strcat(str_ret->content, str_B->content);
}

void string_Add_Number(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Add_Number");

  s_string *str_ret = (s_string *)ret->data;
  s_string *str_self = (s_string *)self->data;

  char tmp[32];
  int ret_len = sprintf(tmp, "%ld", args[0]->data) + 1;

  u_int64_t len = str_self->len + ret_len;

  string_resize(str_ret, len);
  strcpy(str_ret->content, str_self->content);
  strcat(str_ret->content, tmp);
}

void string_Print(s_class_instance *ret, s_class_instance *self, s_class_instance **args) {
	PANALYSIS("string_Print");
  s_string *str_self = (s_string *)self->data;
  printf("%s\n", str_self->content);
}

void stdlib_Init(s_compiler *compiler) {
	PANALYSIS("stdlib_Init");

  /* ##### Primary classes ##### */
  LIB_NumberClass = class_Create("Number", compiler->rootScope);
  LIB_StringClass = class_Create("String", compiler->rootScope);

  /* ##### Number class ##### */
  class_CreateConstructor(LIB_NumberClass, &number_Constructor, 0);
  class_CreateConstructor(LIB_NumberClass, &number_Constructor_Number, 1, "Number");

  class_CreateMethod(LIB_NumberClass, "Add", &number_Add_Number, "Number", 1, "Number");
  class_CreateMethod(LIB_NumberClass, "Sub", &number_Sub_Number, "Number", 1, "Number");
  class_CreateMethod(LIB_NumberClass, "Less", &number_Less_Number, "Number", 1, "Number");
  class_CreateMethod(LIB_NumberClass, "Assign", &number_Assign_Number, "Number", 1, "Number");
  //class_CreateMethod(LIB_NumberClass, "Print", &number_Print, NULL, 0);
  class_CreateMethod(LIB_NumberClass, "ToString", &number_ToString, "String", 0);

  /* ##### String class ##### */
  class_CreateConstructor(LIB_StringClass, &string_Constructor, 0);
  class_CreateConstructor(LIB_StringClass, &string_Constructor_String, 1, "String");

  class_CreateMethod(LIB_StringClass, "Assign", &string_Assign_String, "String", 1, "String");
  class_CreateMethod(LIB_StringClass, "Add", &string_Add_String, "String", 1, "String");
  //class_CreateMethod(LIB_StringClass, "Add", &string_Add_Number, "String", 1, "Number");
  class_CreateMethod(LIB_StringClass, "Print", &string_Print, NULL, 0);
}



/* ##### MAIN town ##### */
s_scope *rootScope;

int main() {
  const char *intro =
"  ____ _____________  \n"
" |    |   \\______   \\ \n"
" |    |   /|     ___/ \n"
" |    |  / |    |     \n"
" |______/  |____|     \n"
"                      \n"
" UP Interpreter  v0.3 \n";
  printf("%s", intro);

  char *srcFilename = "classworld.up";

  fanalysis = fopen("analysis.txt", "w");

  // Compiler
  s_compiler *compiler = compiler_InitFromFile(srcFilename);
  stdlib_Init(compiler);
  compiler_Execute(compiler);

  fclose(fanalysis);

  return 0;

  // Interactive CLI (dream)
  char *line = NULL;
  size_t len = 0;
  ssize_t lineSize = 0;

  while (1) {
    lineSize = getline(&line, &len, stdin);
    
    compiler_ExecuteCLI(compiler, line);
  }



  return 0;
}