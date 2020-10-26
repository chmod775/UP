#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "up.h"

#define PERROR(A, B, ...) { printf("\033[0;31m[" A "] Error! - " B "\033[0m\n", ##__VA_ARGS__); exit(-1); }
#define CERROR(C, A, B, ...) { printf("\033[0;31m[" A "] Error Line: %d - " B "\033[0m\n", (C->parser->line) ##__VA_ARGS__); exit(-1); }

void *t_new = NULL;
#define NEW(T) t_new = malloc(sizeof(T)); if (t_new == NULL) { PERROR("NEW(##T)", "Could not malloc"); exit(-1); }

/* ##### Helpers shit ##### */
FILE *openFile(char *filename) {
  FILE *ret = fopen(filename, "r");
  if (ret == NULL) PERROR("openFile", "could not open(%s)", filename);
  return ret;
}

void closeFile(FILE *fd) {
  if (fclose(fd) != 0)
    PERROR("closeFile", "could not close file");
}

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

bool isUppercase_Char(char ch) { return (ch >= 'A' && ch <= 'Z'); }
bool isUppercase_String(char *str) { return isUppercase_Char(str[0]); }

bool isFullcase_String(char *str) { return isUppercase_Char(str[0]) && isUppercase_Char(str[1]); }

bool startsUnderscore_String(char *str) { return str[0] == '_'; }

/* ##### LINKED LIST ##### */
s_list *list_create() {
  s_list *ret = NEW(s_list);

  ret->items_count = 0;
  ret->head_item = NULL;
  ret->selected_item = NULL;
  return ret;
}
void list_destroy(s_list *l) {
  void *last = list_read_last(l);
  while (last != NULL) {
    free(last);
    last = list_read_previous(l);
  }
}

void *list_read_index(s_list *l, u_int64_t index) {
  if (l->head_item == NULL) return NULL;

  u_int64_t cnt = 0;

  s_list_item *current = l->head_item;
  while (cnt < index) {
    if (current->next == NULL) return NULL;
    current = current->next;
    cnt++;
  }

  l->selected_item = current;

  return l->selected_item->value;
}

void *list_read_first(s_list *l) {
  if (l->head_item == NULL) return NULL;

  l->selected_item = l->head_item;
  return l->selected_item->value;
}
void *list_read_last(s_list *l) {
  if (l->head_item == NULL) return NULL;

  l->selected_item = l->head_item;

  if (l->selected_item->next == NULL) { // Only one item in the list
    return l->selected_item->value;
  }

  // Get last item
  while (l->selected_item->next != NULL) {
    l->selected_item = l->selected_item->next;
  }

  return l->selected_item->value;
}

void *list_read_next(s_list *l) {
  if (l->head_item == NULL) return NULL;
  if (l->selected_item == NULL) return NULL;
  if (l->selected_item->next == NULL) return NULL;

  l->selected_item = l->selected_item->next;

  return l->selected_item->value;
}
void *list_read_previous(s_list *l) {
  if (l->head_item == NULL) return NULL;
  if (l->selected_item == NULL) return NULL;

  s_list_item *current = l->head_item;
  while (current->next != l->selected_item) {
    current = current->next;
  }

  l->selected_item = current;

  return l->selected_item->value;
}

void list_add(s_list *l, void *value) { // Add to the beginning
  s_list_item *item = NEW(s_list_item);
  item->value = value;

  s_list_item *old = l->head_item;
  l->head_item = item;
  item->next = old;

  l->items_count++;
}
void *list_pull(s_list *l) { // Pull item from the beginning
  if (l->head_item == NULL) return NULL;

  s_list_item *item = l->head_item;
  l->head_item = item->next;
  void *ret = item->value;

  free(item);

  l->items_count--;
  return ret;
}

void list_push(s_list *l, void *value) { // Push to the end
  s_list_item *item = NEW(s_list_item);
  item->value = value;
  item->next = NULL;

  if (l->head_item == NULL)
    l->head_item = item;
  else {
    s_list_item *current = l->head_item;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = item;
  }

  l->items_count++;
}
void *list_pop(s_list *l) { // Pop item from the end
  if (l->head_item == NULL) return NULL;

  void *ret = NULL;

  if (l->head_item->next == NULL) { // Only one item in the list
    ret = l->head_item->value;
    free(l->head_item);
    l->head_item = NULL;
    return ret;
  }

  // Get second to last item
  s_list_item *current = l->head_item;
  while (current->next->next != NULL) {
    current = current->next;
  }

  ret = current->next->value;
  free(current->next);
  current->next = NULL;

  return ret;
}

/* ##### PARENTABLE LINKED LIST ##### */
s_parlist *parlist_create(s_parlist *parent) {
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
void parlist_destroy(s_parlist *pl) {
  void *last = parlist_read_last(pl);
  while (last != NULL) {
    list_destroy(last);
    last = parlist_read_previous(pl);
  }
}

void *parlist_read_first(s_parlist *pl) {
  s_list *l = list_read_first(pl->lists);
  pl->selected_list = l;
  return list_read_first(l);
}
void *parlist_read_last(s_parlist *pl) {
  s_list *l = list_read_last(pl->lists);
  pl->selected_list = l;
  return list_read_last(l);
}
void *parlist_read_next(s_parlist *pl) {
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
  void *ret = list_read_previous(pl->selected_list);

  if (ret == NULL) {
    s_list *l = list_read_previous(pl->lists);
    if (l == NULL) return NULL;

    pl->selected_list = l;
    ret = list_read_last(l);
  }

  return ret;
}

void parlist_add(s_parlist *pl, void *value) {
  s_list *l = list_read_first(pl->lists);
  list_add(l, value);
}

void parlist_push(s_parlist *pl, void *value) {
  s_list *l = list_read_last(pl->lists);
  list_push(l, value);
}

/* ##### Symbols ##### */
char *symbol_GetCleanName(s_symbol *symbol) {
  char *ret = (char *)malloc(sizeof(char) * (symbol->length + 1));
  memcpy(ret, symbol->name, symbol->length);
  ret[symbol->length] = 0;
  return ret;
}

s_symbol *symbol_Create(char *name, e_symboltype type, int length) {
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

s_symbol *symbol_CreateFromKeyword(char *keyword, e_token token) {
  s_symbol *ret = symbol_Create(keyword, SYMBOL_KEYWORD, -1);

  ret->body.keyword = NEW(s_symbolbody_keyword);
  ret->body.keyword->token = token;

  return ret;
}

s_symbol *symbol_Find(char *name, s_parlist *symbols) {
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

/* ##### Scope ##### */
s_scope *scope_Create(s_scope *parent) {
  s_scope *ret = NEW(s_scope);

  ret->parent = parent;

  ret->symbols = parlist_create(parent == NULL ? NULL : parent->symbols);
  if (!ret->symbols) { PERROR("scope_Init", "Could not create symbols list"); return NULL; }

  return ret;
}

s_scope *scope_CreateAsRoot() {
  s_scope *ret = scope_Create(NULL);

  // Init symbols with base keywords
  parlist_push(ret->symbols, symbol_CreateFromKeyword("NULL", TOKEN_NULL));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("if", TOKEN_If));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("else", TOKEN_Else));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("return", TOKEN_Return));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("while", TOKEN_While));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("for", TOKEN_For));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("switch", TOKEN_Switch));

  parlist_push(ret->symbols, symbol_CreateFromKeyword("DEBUG", TOKEN_Debug));

  return ret;
}

char buffer[1000];

char *scope_Print(s_scope *scope) {
  char *buffer_ptr = buffer;

  s_symbol *symbol_ptr = (s_symbol *)parlist_read_first(scope->symbols);
  while (symbol_ptr != NULL) {
    int len = sprintf(buffer_ptr, "Symbol: %s - %d", symbol_GetCleanName(symbol_ptr), symbol_ptr->type);
    buffer_ptr += len + 1;
    symbol_ptr = (s_symbol *)parlist_read_next(scope->symbols);
  }

  return buffer;
}

/* ##### Parser ##### */
s_parser *parse_Init(char *str) {
  s_parser *ret = NEW(s_parser);

  ret->line = 1;
  ret->ptr = ret->source = str;
  if (!ret->source) { PERROR("parse_Init", "Src is null"); return NULL; }

  return ret;
}

int parse_Next(s_scope *scope, s_parser *parser) {
  #define src parser->ptr
  #define ret(TOKEN, CONTENT, CONTENTTYPE) { parser->token.content.CONTENTTYPE = (CONTENT); parser->token.type = (TOKEN); return (TOKEN); }

  int token;                    // current token
  char *last_pos;
  int hash;

  int64_t token_val_int;
  double token_val_decimal;
  char *token_val_string;

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
      parlist_push(scope->symbols, new_symbol);

      ret(TOKEN_Symbol, new_symbol, symbol);
    }
    else if (token >= '0' && token <= '9') {
      // parse number, three kinds: dec(123) hex(0x123) oct(017)
      token_val_int = token - '0';
      if (token_val_int > 0) {
        // dec, starts with [1-9]
        while (*src >= '0' && *src <= '9') {
          token_val_int = token_val_int*10 + *src++ - '0';
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
      // parse string literal, currently, the only supported escape
      // character is '\n', store the string literal into data.
      /*
      last_pos = data;
      while (*src != 0 && *src != token) {
          token_val = *src++;
          if (token_val == '\\') {
              // escape character
              token_val = *src++;
              if (token_val == 'n') {
                  token_val = '\n';
              }
          }

          if (token == '"') {
              *data++ = token_val;
          }
      }

      src++;
      // if it is a single character, return Num token
      if (token == '"') {
          token_val = (int)last_pos;
      } else {
          ret(Num, NULL, NULL);
      }
      */
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
      // parse '==' and '='
      if (*src == '=') {
        src ++;
        ret(TOKEN_Eq, NULL, any);
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
    } else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == '[' || token == ']' || token == ',' || token == ':' || token == '.') {
      // directly return the character as token;
      ret(token, NULL, any);
    }
  }

  ret(token, NULL, any);

  #undef ret
  #undef src
}

int parse_Match(s_scope *scope, s_parser *parser, int token) {
  if (parser->token.type == token) {
    parse_Next(scope, parser);
  } else {
    if (token < TOKEN_Symbol)
      printf("Line %d: expected token: %c\n", parser->line, token);
    else
      printf("Line %d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
  if (parser->token.type == TOKEN_Debug) {
    int a = 0;
    while (a);
  }
}

s_token parse_Preview(s_scope *scope, s_parser *parser) {
  s_parser _parser = *parser;
  parse_Next(scope, &_parser);
  return _parser.token;
}

/* ##### STATEMENT ##### */
s_statement *statement_Create(s_statement *parent, s_scope *scope, e_statementtype type) {
  s_statement *ret = NEW(s_statement);

  ret->parent = parent;
  ret->scope = scope;

  ret->type = type;
  
  return ret;
}

s_statement *statement_CreateInside(s_statement *parent, e_statementtype type) {
  s_statement *ret = NEW(s_statement);

  ret->parent = parent->parent;
  ret->scope = parent->scope;

  ret->type = type;
  
  return ret;
}

s_statement *statement_CreateChildren(s_statement *parent, e_statementtype type) {
  s_statement *ret = NEW(s_statement);

  ret->parent = parent;
  ret->scope = scope_Create(parent->scope);

  ret->type = type;

  return ret;
}


s_statement *statement_CreateBlock(s_statement *parent) {
  s_statement *ret = statement_CreateChildren(parent, STATEMENT_BLOCK);
  
  ret->exe_cb = &__core_exe_statement;

  ret->body.block = NEW(s_statementbody_block);
  ret->body.block->statements = list_create();

  return ret;
}

s_statement *statement_CreateRoot(s_compiler *compiler) {
  // Create a class Statement named "Program"
  s_statement *ret = NEW(s_statement);

  ret->type = STATEMENT_CLASS_DEF;
  ret->parent = NULL;
  ret->scope = compiler->rootScope;
  ret->exe_cb = NULL;

  ret->body.class_def = NEW(s_statementbody_class_def);
  ret->body.class_def->symbol = class_Create("Program", compiler->rootScope);

  return ret;
}

/* ##### Compiler ##### */
s_compiler *compiler_Init(char *content) {
  s_compiler *ret = NEW(s_compiler);

  ret->rootScope = scope_CreateAsRoot();
  if (!ret->rootScope) { PERROR("compiler_Init", "Could not create root scope"); return NULL; }

  ret->rootStatement = statement_CreateRoot(ret);
  if (!ret->rootStatement) { PERROR("compiler_Init", "Could not create root class statement"); return NULL; }

  ret->parser = parse_Init(content);
  if (!ret->parser) { PERROR("compiler_Init", "Could not initialize root parser"); return NULL; }

  return ret;
}

s_compiler *compiler_InitFromFile(char *filename) {
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
  parse_Next(compiler->rootStatement->scope, compiler->parser);

  compile_ClassBody(compiler, compiler->rootStatement);

  //__core_exe_statement(compiler->rootStatement);
}

void compiler_ExecuteCLI(s_compiler *compiler, char *code) {
  compiler->parser = parse_Init(code);

  parse_Next(compiler->rootStatement->scope, compiler->parser);

  compile_Statement(compiler, compiler->rootStatement);

  __core_exe_statement(compiler->rootStatement);
}

s_expression_operation *expression_Emit(s_list *core_operations, s_token token) {
  s_expression_operation *op = NEW(s_expression_operation);
  s_token *token_copy = NEW(s_token);
  memcpy(token_copy, &token, sizeof(s_token));
  op->token = token_copy;
  list_push(core_operations, op);
  return op;
}

s_expression_operation *expression_Emit_Callback(s_list *core_operations, int type, void (*cb)(s_stack__core_expression_item *)) {
  s_expression_operation *op = NEW(s_expression_operation);
  s_token *token_copy = NEW(s_token);
  token_copy->type = type;
  token_copy->content.any = cb;
  op->token = token_copy;
  list_push(core_operations, op);
  return op;
}

#define token (compiler->parser->token)
#define match(S, A) parse_Match((S), compiler->parser, (A))
#define preview(S) parse_Preview((S), compiler->parser)
#define emit(O, A) list_push(O, (A))


void expression_Step(s_compiler *compiler, s_statement *statement, int level) {
  s_list *core_operations = statement->body.expression->core_operations;
  
  // Unary operators
  if (token.type == TOKEN_Literal_Int) {
    expression_Emit(core_operations, token);
    match(statement->scope, TOKEN_Literal_Int);
  } else if (token.type == TOKEN_Symbol) {
    if (token.content.symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "expression_Step", "Symbol not defined.");

    s_token next_token = preview(statement->scope);

    if (next_token.type == '(') { // Method call
      match(statement->scope, TOKEN_Symbol);
      match(statement->scope, '(');
      // Call arguments
      match(statement->scope, ')');
      expression_Emit(core_operations, token);
    } else {
      expression_Emit(core_operations, token);
      match(statement->scope, TOKEN_Symbol);
    }
  } else if (token.type == '(') {
    match(statement->scope, '(');
    expression_Step(compiler, statement, TOKEN_Assign);
    match(statement->scope, ')');
  } else if (token.type == TOKEN_Add) {
    // Convert string to number (Javascript like)

  } else if (token.type == TOKEN_Inc) {

  } else if (token.type == TOKEN_Lt) {
    match(statement->scope, TOKEN_Lt);
    expression_Step(compiler, statement, TOKEN_Lt);
    expression_Emit_Callback(core_operations, TOKEN_Lt, &__core_print);
  } else {
    CERROR(compiler, "expression_Step", "Bad expression");
    exit(-1);
  }

  // binary operator and postfix operators.
  while (token.type >= level) {
    if (token.type == TOKEN_Assign) {
      match(statement->scope, TOKEN_Assign);
      expression_Step(compiler, statement, TOKEN_Assign);
      expression_Emit_Callback(core_operations, TOKEN_Assign, &__core_assign);
    } else if (token.type == TOKEN_Add) {
      match(statement->scope, TOKEN_Add);
      expression_Step(compiler, statement, TOKEN_Mul);

      expression_Emit_Callback(core_operations, TOKEN_Add, &__core_add);
    } else if (token.type == TOKEN_Inc) {
      match(statement->scope, TOKEN_Inc);
      expression_Emit_Callback(core_operations, TOKEN_Inc, &__core_inc);
    } else if (token.type == TOKEN_Lt) {
      match(statement->scope, TOKEN_Lt);
      expression_Step(compiler, statement, TOKEN_Lt);
      expression_Emit_Callback(core_operations, TOKEN_Inc, &__core_cmp);
    } else {
      CERROR(compiler, "expression_Step", "Compiler error");
      exit(-1);
    }
  }
}

s_statement *compile_Expression(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_CreateInside(parent, STATEMENT_EXPRESSION);

  ret->exe_cb = &__core_expression;

  ret->body.expression = NEW(s_statementbody_expression);
  ret->body.expression->core_operations = list_create();

  expression_Step(compiler, ret, TOKEN_Assign);

  return ret;
}


s_symbol *class_Create(char *name, s_scope *scope) {
  s_symbol *symbol = symbol_Create(name, SYMBOL_CLASS, -1);

  symbol->body.class = NEW(s_symbolbody_class);
  symbol->body.class->parent = NULL;
  symbol->body.class->scope = scope;
  symbol->body.class->fields = list_create();
  symbol->body.class->methods = list_create();

  parlist_push(scope->symbols, symbol);

  return symbol;
}

void compile_ClassBody(s_compiler *compiler, s_statement *class) {
  s_statementbody_class_def *class_body = class->body.class_def;

  while ((token.type > 0) && (token.type != '}') ) {
    s_statement *statement = compile_Statement(compiler, class);
    if (statement != NULL) { // If not empty statement
      if (statement->type == STATEMENT_FIELD_DEF) {
        list_push(class_body->symbol->body.class->fields, statement->body.field_def->symbol);
      } else if (statement->type == STATEMENT_METHOD_DEF) {
        list_push(class_body->symbol->body.class->methods, statement->body.method_def->symbol);
      } else if (statement->type == STATEMENT_CLASS_DEF) {
        // Do nothing
      } else {
        CERROR(compiler, "compile_ClassBody", "Statement not allowed in class definition.");
      }
    }
  }
}

s_statement *compile_ClassDefinition(s_compiler *compiler, s_statement *parent) {
  if (parent->type != STATEMENT_CLASS_DEF) CERROR(compiler, "compiler_ClassDefinition", "Wrong statement for class definition.");
  if (parent->body.class_def->symbol->type != SYMBOL_CLASS) CERROR(compiler, "compiler_ClassDefinition", "Wrong parent symbol.");

  s_symbol *class_symbol = token.content.symbol;
  s_symbol *parent_symbol = parent->body.class_def->symbol; // Direct Inheritance

  match(parent->scope, TOKEN_Symbol);

  if (token.type == ':') { // Derivation (multiple classes allowed)
    // MixClass : ClassA, ClassB, ClassC, ... {}
    match(parent->scope, ':');
  }

  if (token.type == '.') { // External Inheritance
    // ClassA.ClassB...ClassC.Children {}
    match(parent->scope, '.');
  }

  s_statement *ret = NULL;

  if (class_symbol->type == SYMBOL_NOTDEFINED) {
    ret = statement_CreateChildren(parent, STATEMENT_CLASS_DEF);

    class_symbol->type = SYMBOL_CLASS;
    class_symbol->body.class = NEW(s_symbolbody_class);
    class_symbol->body.class->parent = parent_symbol;
    class_symbol->body.class->fields = list_create();
    class_symbol->body.class->methods = list_create();
    class_symbol->body.class->scope = ret->scope;
  } else {
    ret = statement_Create(parent, class_symbol->body.class->scope, STATEMENT_CLASS_DEF);
  }

  ret->body.class_def = NEW(s_statementbody_class_def);
  ret->body.class_def->symbol = class_symbol;

  match(ret->scope, '{');

  compile_ClassBody(compiler, ret);

  match(parent->scope, '}');

  return ret;
}

// name([arg0, arg1, ...]) [: <return type>] { <statements> }
s_statement *compile_MethodDefinition(s_compiler *compiler, s_statement *parent) {
  if (parent->type != STATEMENT_CLASS_DEF) CERROR(compiler, "compile_MethodDefinition", "Wrong parent statement for method definition.");
  if (parent->body.class_def->symbol->type != SYMBOL_CLASS) CERROR(compiler, "compile_MethodDefinition", "Wrong parent symbol.");

  s_symbol *name = token.content.symbol;
  if (!name->isUppercase) CERROR(compiler, "compile_MethodDefinition", "Method definition must start uppercase.");
  if (name->type != SYMBOL_NOTDEFINED) CERROR(compiler, "compile_MethodDefinition", "Method already defined.");

  s_statement *ret = statement_CreateInside(parent, STATEMENT_METHOD_DEF);
  ret->body.method_def = NEW(s_statementbody_method_def);
  ret->body.method_def->symbol = name;

  match(parent->scope, TOKEN_Symbol);

  name->body.method = NEW(s_symbolbody_method);
  name->type = SYMBOL_METHOD;

  s_statement *ret_statement = statement_CreateBlock(parent);
  name->body.method->body = ret_statement;

  // Read arguments
  match(ret_statement->scope, '(');

  while (token.type != ')') {
    s_statement *arg_statement = compile_ArgumentDefinition(compiler, ret_statement);

    if (token.type == ',')
      match(ret_statement->scope, ',');
  }

  match(parent->scope, ')');

  if (token.type == ':') {
    match(parent->scope, ':');
    // Return type
    s_anytype *type = compile_FieldType(compiler, parent);
    name->body.method->ret_type = *type;
  }

  // Get body statement for method
  match(ret_statement->scope, '{');

  while (token.type != '}') {
    s_statement *statement = compile_Statement(compiler, ret_statement);
    list_push(ret_statement->body.block->statements, statement);
  }

  match(parent->scope, '}');

  list_push(parent->body.class_def->symbol->body.class->fields, name);

  return ret;
}

s_anytype *compile_FieldType(s_compiler *compiler, s_statement *statement) {
  s_anytype *ret = NEW(s_anytype);

  ret->isDictionary = false;
  ret->isList = false;
  ret->isClass = false;

  if (token.type == TOKEN_Symbol) {
    // Class
    s_symbol *symbol = token.content.symbol;
    if (!symbol->isUppercase) CERROR(compiler, "compile_FieldType", "Symbol is not a class");
    if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldType", "Symbol is not defined");

    ret->isClass = true;
    ret->type.class = symbol;

    match(statement->scope, TOKEN_Symbol);

    // Childrens
    while (token.type == '.') {
      s_symbolbody_class *symbol_body = symbol->body.class;
      match(symbol_body->scope, '.');

      symbol = token.content.symbol;
      if (!symbol->isUppercase) CERROR(compiler, "compile_FieldType", "Symbol is not a class");
      if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldType", "Symbol is not defined");

      ret->isClass = true;
      ret->type.class = symbol;

      match(symbol_body->scope, TOKEN_Symbol);
    }
  } else if (token.type == '[') {
    // List
    match(statement->scope, '[');

    s_listtype *ltype = NEW(s_listtype);
    ltype->items = compile_FieldType(compiler, statement);

    ret->isList = true;
    ret->type.list = ltype;

    match(statement->scope, ']');
  } else if (token.type == '{') {
    // Dictionary
    match(statement->scope, '{');

    s_dictionarytype *dtype = NEW(s_dictionarytype);

    dtype->key = compile_FieldType(compiler, statement);

    if (!dtype->key->isClass) {
      CERROR(compiler, "compile_FieldType", "Dictionary key can only be of primary type");
      exit(-1);
    }

    match(statement->scope, ',');

    dtype->value = compile_FieldType(compiler, statement);

    ret->isDictionary = true;
    ret->type.dictionary = dtype;

    match(statement->scope, '}');
  } else {
    CERROR(compiler, "compile_FieldType", "Unsupported complex field definition");
    exit(-1);
  }

  return ret;
}


s_statement *compile_ArgumentDefinition(s_compiler *compiler, s_statement *parent) {
  s_symbol *name = token.content.symbol;
  if (name->type != SYMBOL_NOTDEFINED) CERROR(compiler, "compile_ArgumentDefinition", "Symbol already defined.");

  s_statement *ret = statement_CreateInside(parent, STATEMENT_FIELD_DEF);
  ret->exe_cb = &__core_field_def;

  match(ret->scope, TOKEN_Symbol);

  ret->body.field_def = NEW(s_statementbody_field_def);
  ret->body.field_def->symbol = name;

  match(ret->scope, ':');

  name->body.field = NEW(s_symbolbody_field);
  name->type = SYMBOL_FIELD;

  s_anytype *type = compile_FieldType(compiler, ret);
  name->body.field->value.type = *type;
  name->body.field->init_expression = NULL;

  if (token.type == TOKEN_Assign) {
    match(ret->scope, TOKEN_Assign);

    name->body.field->init_expression = compile_Expression(compiler, ret);
  }

  return ret;
}

s_statement *compile_FieldDefinition(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = compile_ArgumentDefinition(compiler, parent);
  if (ret->body.field_def->symbol->body.field->init_expression == NULL) CERROR(compiler, "compile_FieldDefinition", "Field must have an initiliazation value.");

  return ret;
}

s_statement *compile_For(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_CreateInside(parent, STATEMENT_FOR);

  ret->body._for = NEW(s_statementbody_for);

  // Init statement
  ret->body._for->check = compile_Expression(compiler, ret);

  match(ret->scope, ';');

  return ret;
}

s_statement *compile_While(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_CreateInside(parent, STATEMENT_WHILE);
  ret->exe_cb = &__core_while;

  match(ret->scope, TOKEN_While);

  ret->body._while = NEW(s_statementbody_while);

  ret->body._while->check = compile_Expression(compiler, ret);
  ret->body._while->loop = compile_Statement(compiler, ret);

  return ret;
}

s_statement *compile_Statement(s_compiler *compiler, s_statement *parent) {
  // Statements types:
  // 1. if (...) <statement> [else <statement>]
  // 2. for (...) <statement>
  // 3. while (...) <statement>
  // 4. { <statement> }
  // 5. return xxx;
  // 6. <empty statement>;
  // 7. field : type = <expression>;
  // 8. method(...) : type { <statement> }
  // 9. class { <statement> }
  // 9. expression; (expression end with semicolon)

  s_statement *ret = NULL;

  if (token.type == TOKEN_If) {

  } else if (token.type == TOKEN_For) {

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
  } else if (token.type == TOKEN_Return) {

  } else if (token.type == ';') {
    // Empty statement
    match(parent->scope, ';');
  } else if (token.type == TOKEN_Symbol) {
    if (token.content.symbol->isUppercase) { // Method or Class
      if (token.content.symbol->type == SYMBOL_METHOD) {
        // Expression (espresso ?)
        ret = compile_Expression(compiler, parent);
        match(parent->scope, ';');
      } else {
        s_token next_token = preview(parent->scope);
        if (next_token.type == '(') {
          // Method definition
          ret = compile_MethodDefinition(compiler, parent);
        } else {
          // Class definition
          ret = compile_ClassDefinition(compiler, parent);
        }
      }

      //compiler_ClassDefinition(compiler);
    } else if (token.content.symbol->isFullcase) { // Constant property

    } else if (token.content.symbol->startsUnderscore) { // Private property

    } else {
      s_token next_token = preview(parent->scope);

      if (next_token.type == ':') {
        // Field definition
        ret = compile_FieldDefinition(compiler, parent);
        match(parent->scope, ';');
      } else {
        // Expression (espresso ?)
        ret = compile_Expression(compiler, parent);
        match(parent->scope, ';');
      }
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
core_expression_item core_expression_item_Create(s_expression_operation *op, s_anyvalue value) {
  core_expression_item ret;
  ret.op = op;
  ret.value = value;
  return ret;
}

void __core_print(s_stack__core_expression_item *stack) {
  s_anyvalue a = stack_pop__core_expression_item(stack).value;
  // T O D O
}

void __core_exe_assign(s_symbol *symbol, s_anyvalue item) {
  if (symbol->type != SYMBOL_FIELD) { PERROR("__core_assign", "Wrong symbol type"); exit(1); }

  //s_symbolbody_field *symbol_body = (s_symbolbody_field *)symbol->body;

  // T O D O
/*
  bool isPrimary = symbol_body->value.type.isPrimary && item.type.isPrimary;
  if (isPrimary) {
    if (symbol_body->value.type.type == item.type.type) {
      symbol_body->value.content = item.content;
    }
  }
*/
}

void __core_assign(s_stack__core_expression_item *stack) {
  s_anyvalue a = stack_pop__core_expression_item(stack).value;

  core_expression_item b = stack_pop__core_expression_item(stack);
  if (b.op == NULL) PERROR("__core_assign", "Stack assign destination cannot be NULL");
  if (b.op->token->type != TOKEN_Symbol) PERROR("__core_assign", "Assignment destination not valid");

  s_symbol *dest_symbol = b.op->token->content.symbol;

  __core_exe_assign(dest_symbol, a);

  stack_push__core_expression_item(stack, core_expression_item_Create(NULL, a));
}

void __core_add(s_stack__core_expression_item *stack) {
  s_anyvalue a = stack_pop__core_expression_item(stack).value;
  s_anyvalue b = stack_pop__core_expression_item(stack).value;
  s_anyvalue r;

  // T O D O

  stack_push__core_expression_item(stack, core_expression_item_Create(NULL, r));
}
void __core_sub(s_stack__core_expression_item *stack) {}
void __core_mul(s_stack__core_expression_item *stack) {}
void __core_div(s_stack__core_expression_item *stack) {}

void __core_inc(s_stack__core_expression_item *stack) {
  core_expression_item a = stack_pop__core_expression_item(stack);
  if (a.op->token->type != TOKEN_Symbol) PERROR("__core_inc", "Destination not valid");
  s_symbol *dest_symbol = a.op->token->content.symbol;

  // T O D O

  stack_push__core_expression_item(stack, core_expression_item_Create(NULL, dest_symbol->body.field->value));
}

void __core_cmp(s_stack__core_expression_item *stack) {
  s_anyvalue b = stack_pop__core_expression_item(stack).value;
  s_anyvalue a = stack_pop__core_expression_item(stack).value;
  s_anyvalue r;

  // T O D O

  stack_push__core_expression_item(stack, core_expression_item_Create(NULL, r));
}

s_anyvalue __core_exe_expression(s_statement *statement) {
  s_stack__core_expression_item stack = stack_create__core_expression_item(50);
  stack.ptr = 0;

  s_list *core_operations = statement->body.expression->core_operations;

  s_expression_operation *op = list_read_first(core_operations);

  do {
    if (op->token->type == TOKEN_Literal_Int) {
      //stack_push__core_expression_item(&stack, core_expression_item_Create(op, s_anyvalue_createPrimary(TYPE_Int64, op->token->value.integer)));
    } else if (op->token->type == TOKEN_Symbol) {
      if (op->token->content.symbol->type == SYMBOL_FIELD) {
        stack_push__core_expression_item(&stack, core_expression_item_Create(op, op->token->content.symbol->body.field->value));
      } else if (op->token->content.symbol->type == SYMBOL_METHOD) {
      } else if (op->token->content.symbol->type == SYMBOL_CLASS) {
      } else {
        PERROR("__core_exe_expression", "Symbol type not allowed for expression");
      }
    } else {
      void (*cb)(s_stack__core_expression_item *stack) = op->token->content.any;
      if (cb)
        cb(&stack);
    }
    op = list_read_next(core_operations);
  } while (op != NULL);

  s_anyvalue ret = stack_pop__core_expression_item(&stack).value;

  stack_free__core_expression_item(stack);

  return ret;
}


void __core_expression(s_statement *statement) {
  s_anyvalue exp_result = __core_exe_expression(statement);
}

void __core_field_def(s_statement *statement) {
  if (statement->type != STATEMENT_FIELD_DEF) { PERROR("__core_field_def", "Wrong statement type"); exit(1); }

  s_symbol *symbol = statement->body.field_def->symbol;

  if (symbol->body.field->init_expression != NULL) {
    s_anyvalue exp_result = __core_exe_expression(symbol->body.field->init_expression);
    __core_exe_assign(symbol, exp_result);
  }
}

void __core_call_method(s_statement *statement) {}

void __core_exe_statement(s_statement *statement) {
  if (statement->type == STATEMENT_BLOCK) {
    s_statementbody_block *statement_body = statement->body.block;

    s_statement *sub_statement = list_read_first(statement_body->statements);
    do {
      __core_exe_statement(sub_statement);
      sub_statement = list_read_next(statement_body->statements);
    } while (sub_statement != NULL);
  } else {
    if (statement->exe_cb)
      statement->exe_cb(statement);
  }
}

void __core_while(s_statement *statement) {
  if (statement->type != STATEMENT_WHILE) PERROR("__core_while", "Wrong statement type");

  s_statementbody_while *statement_body = statement->body._while;

  s_anyvalue check_result = __core_exe_expression(statement_body->check);
  
  while (check_result.content) {
    __core_exe_statement(statement_body->loop);
    check_result = __core_exe_expression(statement_body->check);
  }
}

void __core_new_class_instance() {}

/* ##### STDLIB 3rd avenue ##### */
void stdlib_Init(s_compiler *compiler) {
  s_symbol *number = class_Create("Number", compiler->rootScope);
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

  char *srcFilename = "helloworld.up";


  // Compiler
  s_compiler *compiler = compiler_InitFromFile(srcFilename);
  stdlib_Init(compiler);
  compiler_Execute(compiler);

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