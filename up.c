#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

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
    l->items_count--;
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

  l->items_count--;
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

s_symbol *symbol_CreateEmpty(e_symboltype type) {
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
  scope_AddSymbol(ret, symbol_CreateFromKeyword("NULL", TOKEN_NULL));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("if", TOKEN_If));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("else", TOKEN_Else));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("return", TOKEN_Return));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("while", TOKEN_While));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("for", TOKEN_For));
  scope_AddSymbol(ret, symbol_CreateFromKeyword("switch", TOKEN_Switch));

  scope_AddSymbol(ret, symbol_CreateFromKeyword("DEBUG", TOKEN_Debug));

  return ret;
}

void scope_AddSymbol(s_scope *scope, s_symbol *symbol) {
  parlist_push(scope->symbols, symbol);
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
      scope_AddSymbol(scope, new_symbol);

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

  s_class_instance *programInstance = class_CreateInstance(compiler->rootStatement->body.class_def->symbol);

  s_list *args = list_create();
  s_method_def *mainMethod = class_FindMethod(compiler->rootStatement->body.class_def->symbol, "Main", args);

  __core_exe_method(programInstance, mainMethod, args);
}

void compiler_ExecuteCLI(s_compiler *compiler, char *code) {
  compiler->parser = parse_Init(code);

  parse_Next(compiler->rootStatement->scope, compiler->parser);

  compile_Statement(compiler, compiler->rootStatement);
}

s_expression_operation *expression_Emit(s_list *core_operations, e_expression_operation_type type) {
  s_expression_operation *op = NEW(s_expression_operation);
  op->type = type;
  list_push(core_operations, op);
  return op;
}

#define token (compiler->parser->token)
#define match(S, A) parse_Match((S), compiler->parser, (A))
#define preview(S) parse_Preview((S), compiler->parser)
#define emit(O, A) list_push(O, (A))


s_expression_operation *expression_Step(s_compiler *compiler, s_statement *statement, int level) {
  s_list *operations = statement->body.expression->operations;
  s_expression_operation *op = NULL;

  // Unary operators
  if (token.type == TOKEN_Literal_Int) {
    // TODO
    match(statement->scope, TOKEN_Literal_Int);
  } else if (token.type == TOKEN_Literal_Real) {
    // TODO
    match(statement->scope, TOKEN_Literal_Real);    
  } else if (token.type == TOKEN_Literal_String) {
    // TODO
    match(statement->scope, TOKEN_Literal_String);
  } else if (token.type == TOKEN_Symbol) {
    s_symbol *symbol = token.content.symbol;
    if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "expression_Step", "Symbol not defined.");

    s_symbol *target_symbol = symbol;

    match(statement->scope, TOKEN_Symbol);

    // Descend parent hierarchy
    while (token.type == '.') {
      s_scope *parent_scope = statement->scope;

      if (symbol->type == SYMBOL_CLASS) {
        parent_scope = symbol->body.class->scope;
      } else if (symbol->type == SYMBOL_FIELD) {
        parent_scope = symbol->body.field->value.type->body.class->scope;
      } else {
        CERROR(compiler, "expression_Step", "Parent symbol is not valid.");
      }

      match(parent_scope, '.');

      symbol = token.content.symbol;
      if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "expression_Step", "Symbol not defined.");

      match(parent_scope, TOKEN_Symbol);
    }

    if (token.type == '(') { // Call of Method or Constructor
      // Arguments
      match(statement->scope, '(');

      if (symbol->type == SYMBOL_METHOD) {
        op = expression_Emit(operations, OP_AccessField);
        op->payload.field = target_symbol;
      }

      u_int64_t TEMP_arguments_count = 0;
      while (token.type != ')') {
        expression_Step(compiler, statement, TOKEN_Assign);

        if (token.type == ',')
          match(statement->scope, ',');

        TEMP_arguments_count++;
      }

      match(statement->scope, ')');

      if (symbol->type == SYMBOL_METHOD) { // Internal method call
        op = expression_Emit(operations, OP_MethodCall);

        s_method_def *found_overload = list_read_first(symbol->body.method->overloads);
        while (found_overload != NULL) {
          if (found_overload->arguments->items_count == TEMP_arguments_count) // TEMPORARY SEARCH FROM ARGUMENTS COUNT
            break;
          found_overload = list_read_next(symbol->body.method->overloads);
        }

        if (found_overload == NULL) CERROR(compiler, "expression_Step", "Method overload not found.");

        op->payload.method = found_overload;
      } else if (symbol->type == SYMBOL_CLASS) { // Constructor call
        op = expression_Emit(operations, OP_ConstructorCall);

        s_method_def *found_constructor = list_read_first(symbol->body.class->constructors);
        while (found_constructor != NULL) {
          if (found_constructor->arguments->items_count == TEMP_arguments_count) // TEMPORARY SEARCH FROM ARGUMENTS COUNT
            break;
          found_constructor = list_read_next(symbol->body.class->constructors);
        }

        if (found_constructor == NULL) CERROR(compiler, "expression_Step", "Constructor not found.");

        op->payload.method = found_constructor;
      } else {
        CERROR(compiler, "expression_Step", "Undefined symbol behaviour.");
      }
    } else {
      op = expression_Emit(operations, OP_AccessField);
      op->payload.field = symbol;
    }
  } else if (token.type == '(') {
    match(statement->scope, '(');
    op = expression_Step(compiler, statement, TOKEN_Assign);
    match(statement->scope, ')');
  } else if (token.type == TOKEN_Lt) {
    match(statement->scope, TOKEN_Lt);
  } else if (token.type == ',') {
    return NULL;
  } else {
    CERROR(compiler, "expression_Step", "Bad expression.");
    exit(-1);
  }

  // binary operator and postfix operators.
  while (token.type >= level) {
    if (token.type == TOKEN_Assign) {
      s_expression_operation *op_target = NEW(s_expression_operation);
      op_target->type = op->type;
      op_target->payload = op->payload;

      match(statement->scope, TOKEN_Assign);

      s_expression_operation *op_arg = expression_Step(compiler, statement, TOKEN_Assign);

      s_list *args = list_create();
      list_push(args, op_arg);

      op = expression_Emit(operations, OP_MethodCall);

      op->payload.method = class_FindMethod(op_target->payload.field->body.field->value.type, "Assign", args);
    } else if (token.type == TOKEN_Add) {
      s_expression_operation *op_target = NEW(s_expression_operation);
      op_target->type = op->type;
      op_target->payload = op->payload;

      match(statement->scope, TOKEN_Add);
      s_expression_operation *op_arg = expression_Step(compiler, statement, TOKEN_Mul);

      s_list *args = list_create();
      list_push(args, op_arg);

      op = expression_Emit(operations, OP_MethodCall);

      if (op_target->type == OP_AccessField) {
        op->payload.method = class_FindMethod(op_target->payload.field->body.field->value.type, "Add", args);
      } else if ((op_target->type == OP_MethodCall) || (op_target->type == OP_ConstructorCall)) {
        op->payload.method = class_FindMethod(op_target->payload.method->ret_type, "Add", args);
      } else {
        CERROR(compiler, "expression_Step", "Wrong operation type");
      }
    } else if (token.type == TOKEN_Inc) {
      match(statement->scope, TOKEN_Inc);
      
    } else if (token.type == TOKEN_Lt) {
      match(statement->scope, TOKEN_Lt);

    } else {
      CERROR(compiler, "expression_Step", "Compiler error");
      exit(-1);
    }
  }

  return op;
}

s_statement *compile_Expression(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_CreateInside(parent, STATEMENT_EXPRESSION);

  ret->exe_cb = &__core_expression;

  ret->body.expression = NEW(s_statementbody_expression);
  ret->body.expression->operations = list_create();
  
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
  symbol->body.class->constructors = list_create();

  scope_AddSymbol(scope, symbol);

  return symbol;
}

s_method_def *class_CreateConstructor(s_symbol *class, s_class_instance *(*cb)(s_class_instance *self, s_list *args), int nArguments, ...) {
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

      s_symbol *symbol_argument = symbol_CreateEmpty(SYMBOL_FIELD);
      symbol_argument->body.field = NEW(s_symbolbody_field);
      symbol_argument->body.field->value.type = symbol_argumentType;

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

s_method_def *class_CreateMethod(s_symbol *class, char *name, s_class_instance *(*cb)(s_class_instance *self, s_list *args), char *returnType, int nArguments, ...) {
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

      s_symbol *symbol_argument = symbol_CreateEmpty(SYMBOL_FIELD);
      symbol_argument->body.field = NEW(s_symbolbody_field);
      symbol_argument->body.field->value.type = symbol_argumentType;

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

s_method_def *class_FindMethod(s_symbol *class, char *name, s_list *args) {
  if (class->type != SYMBOL_CLASS) PERROR("class_FindMethod", "Destination is not a Class.");
  if (name == NULL) PERROR("class_FindMethod", "Name cannot be null.");

  s_symbol *found_symbol = symbol_Find(name, class->body.class->scope->symbols);
  if (found_symbol == NULL) PERROR("class_FindMethod", "Symbol does not exists.");

  s_method_def *found_overload = list_read_first(found_symbol->body.method->overloads);
  while (found_overload != NULL) {
    if (found_overload->arguments->items_count == args->items_count) // TEMPORARY SEARCH FROM ARGUMENTS COUNT
      break;
    found_overload = list_read_next(found_symbol->body.method->overloads);
  }

  if (found_overload == NULL) PERROR("class_FindMethod", "Method overload not found.");

  return found_overload;
}


void compile_ClassBody(s_compiler *compiler, s_statement *class) {
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

  match(ret->scope, '{');

  compile_ClassBody(compiler, ret);

  match(parent->scope, '}');

  return ret;
}

// ([arg0, arg1, ...]) { <statements> }
s_statement *compile_ConstructorMethodDefinition(s_compiler *compiler, s_statement *parent) {
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

    list_push(newMethod->arguments, arg_statement->body.field_def->symbol);

    hash = hash * 147 + arg_statement->body.field_def->symbol->body.field->value.type->hash;

    if (token.type == ',')
      match(ret_statement->scope, ',');
  }

  match(parent->scope, ')');

  newMethod->hash = hash;

  // Check already defined Method overload
  s_method_def *m = list_read_first(parent->body.class_def->symbol->body.class->constructors);
  while (m != NULL) {
    if (hash == m->hash) {
      // ### TODO: Deep check for memory content, not only hash
      CERROR(compiler, "compile_MethodDefinition", "Constructor overload already defined.");
    }
    m = list_read_next(parent->body.class_def->symbol->body.class->constructors);
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

  s_statement *ret_statement = statement_CreateBlock(parent);
  newMethod->body.type = METHODBODY_STATEMENT;
  newMethod->body.content.statement = ret_statement;

  // Read arguments
  match(ret_statement->scope, '(');

  newMethod->arguments = list_create();
  while (token.type != ')') {
    s_statement *arg_statement = compile_ArgumentDefinition(compiler, ret_statement);

    list_push(newMethod->arguments, arg_statement->body.field_def->symbol);

    hash = hash * 147 + arg_statement->body.field_def->symbol->body.field->value.type->hash;

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

  // Get body statement for method
  match(ret_statement->scope, '{');

  while (token.type != '}') {
    s_statement *statement = compile_Statement(compiler, ret_statement);
    list_push(ret_statement->body.block->statements, statement);
  }

  match(parent->scope, '}');

  // Push to overloads
  list_push(name->body.method->overloads, newMethod);

  return ret;
}

s_anytype *compile_FieldType(s_compiler *compiler, s_statement *statement) {
  s_anytype *ret = NULL;

  if (token.type == TOKEN_Symbol) {
    // Class
    s_symbol *symbol = token.content.symbol;
    if (!symbol->isUppercase) CERROR(compiler, "compile_FieldType", "Symbol is not a class");
    if (symbol->type == SYMBOL_NOTDEFINED) CERROR(compiler, "compile_FieldType", "Symbol is not defined");

    ret = symbol;

    match(statement->scope, TOKEN_Symbol);

    // Childrens
    while (token.type == '.') {
      s_symbolbody_class *symbol_body = symbol->body.class;
      match(symbol_body->scope, '.');

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

  name->body.field->value.type = compile_FieldType(compiler, ret);
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

s_statement *compile_DefinitionStatement(s_compiler *compiler, s_statement *parent) {
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
      // Expression (espresso ?)
      ret = compile_Expression(compiler, parent);
      match(parent->scope, ';');
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







void __core_expression(s_class_instance *self, s_statement *statement) {
  __core_exe_expression(self, statement);
}

void __core_field_def(s_statement *statement) {

}

void __core_call_method(s_statement *statement) {}

void __core_exe_statement(s_class_instance *self, s_statement *statement) {
  if (statement != NULL) {
    if (statement->type == STATEMENT_BLOCK) {
      s_statementbody_block *statement_body = statement->body.block;

      s_statement *sub_statement = list_read_first(statement_body->statements);
      do {
        __core_exe_statement(self, sub_statement);
        sub_statement = list_read_next(statement_body->statements);
      } while (sub_statement != NULL);
    } else {
      if (statement->exe_cb)
        statement->exe_cb(self, statement);
    }
  }
}

void __core_while(s_statement *statement) {
  if (statement->type != STATEMENT_WHILE) PERROR("__core_while", "Wrong statement type");

  s_statementbody_while *statement_body = statement->body._while;

  //s_anyvalue check_result = __core_exe_expression(statement_body->check);
  /*
  while (check_result.content) {
    __core_exe_statement(statement_body->loop);
    check_result = __core_exe_expression(statement_body->check);
  }
  */
}

void __core_new_class_instance() {}

s_class_instance *__core_exe_method(s_class_instance *self, s_method_def *method, s_list *args) {
  if (method->body.type == METHODBODY_STATEMENT) {
    __core_exe_statement(self, method->body.content.statement);
    return NULL;
  } else if (method->body.type == METHODBODY_CALLBACK) {
    return method->body.content.callback(self, args);
  } else {
    PERROR("__core_exe_expression", "Method type error.");
  }
}

s_class_instance *__core_exe_expression(s_class_instance *self, s_statement *statement) {
  if (statement->type != STATEMENT_EXPRESSION) PERROR("__core_exe_expression", "Wrong statement type");

  s_list *operations = statement->body.expression->operations;

  s_list *stack = list_create(); // <s_class_instance>

  s_expression_operation *op = list_read_first(operations);
  while (op != NULL) {
    if (op->type == OP_AccessField) {
      s_class_instance *value = self->data[op->payload.field->body.field->value.data_index];
      list_push(stack, value);
    } else if ((op->type == OP_MethodCall) || (op->type == OP_ConstructorCall)) {
      s_class_instance *value = NULL;

      // Pop arguments from stack
      s_list *args = list_create();
      u_int64_t args_count = op->payload.method->arguments->items_count;

      u_int64_t arg_idx;
      for (arg_idx = 0; arg_idx < args_count; arg_idx++) {
        s_class_instance *arg_value = list_pop(stack);
        list_push(args, arg_value);
      }

      // Pop target and call method
      s_class_instance *target = NULL;
      if (op->type == OP_MethodCall) {
        target = list_pop(stack);
        value = __core_exe_method(target, op->payload.method, args);
        list_push(stack, value);
      } else if (op->type == OP_ConstructorCall) {
        target = class_CreateInstance(op->payload.method->ret_type);
        __core_exe_method(target, op->payload.method, args);
        list_push(stack, target);
      }
    } else {
      PERROR("__core_exe_expression", "Operation not allowed.");
    }

    op = list_read_next(operations);
  }

  if (stack->items_count > 1) PERROR("__core_exe_expression", "Stack error.");
  return list_pop(stack);
}

s_class_instance *class_CreateInstance(s_symbol *class) {
  if (class->type != SYMBOL_CLASS) PERROR("class_CreateInstance", "Wrong statement type");

  s_class_instance *instance = NEW(s_class_instance);
  instance->class = class;

  // Allocate data space
  instance->data = (s_class_instance **)malloc(sizeof(s_class_instance *) * instance->class->body.class->fields->items_count);

  // Initialize fields
  s_symbol *field_symbol = list_read_first(instance->class->body.class->fields);
  while (field_symbol != NULL) {
    s_class_instance *init_value = __core_exe_expression(instance, field_symbol->body.field->init_expression);
    instance->data[field_symbol->body.field->value.data_index] = init_value;

    field_symbol = list_read_next(instance->class->body.class->fields);
  }

  return instance;
}

/* ##### STDLIB 3rd avenue ##### */
s_symbol *numberClass = NULL;

s_class_instance *number_Constructor(s_class_instance *self, s_list *args) {
  self->data = malloc(sizeof(u_int16_t));
  self->data = 0x775;
  return self;
}

s_class_instance *number_Add_Number(s_class_instance *self, s_list *args) {
  s_class_instance *ret = class_CreateInstance(numberClass);

  s_class_instance *arg_B = list_read_first(args);

  ret->data = malloc(sizeof(u_int16_t));
  ret->data = (u_int16_t)self->data + (u_int16_t)arg_B->data;

  return ret;
}

s_class_instance *number_Assign_Number(s_class_instance *self, s_list *args) {
  s_class_instance *arg_B = list_read_first(args);
  self->data = arg_B->data;
  return self;
}

s_class_instance *number_Assign_Print(s_class_instance *self, s_list *args) {
  printf("%X\n", self->data);
  return NULL;
}

void stdlib_Init(s_compiler *compiler) {
  numberClass = class_Create("Number", compiler->rootScope);
  // class_CreateField(numberClass, "raw", NULL, sizeof(u_int64_t));
  class_CreateConstructor(numberClass, &number_Constructor, 0);
  class_CreateConstructor(numberClass, &number_Constructor, 1, NULL);
  // class_CreateConstructor(numberClass, &number_Constructor, 1, "Number");
  // class_CreateMethod(numberClass, "Add", &number_Add_Literal_Literal, 2, NULL, NULL);
  // class_CreateMethod(numberClass, "Add", &number_Add_Literal_Number, 2, NULL, "Number");
  // class_CreateMethod(numberClass, "Add", &number_Add_Number_Literal, 2, "Number", NULL);
  class_CreateMethod(numberClass, "Add", &number_Add_Number, "Number", 1, "Number");
  class_CreateMethod(numberClass, "Assign", &number_Assign_Number, "Number", 1, "Number");
  class_CreateMethod(numberClass, "Print", &number_Assign_Print, NULL, 0);
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