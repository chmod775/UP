#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "up.h"

#define PERROR(A, B, ...) { printf("[" A "] Error! - " B "\n", ##__VA_ARGS__); exit(-1); }

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

s_symbol *symbol_CreateFromKeyword(char *keyword, int token) {
  s_symbol *ret = NEW(s_symbol);

  ret->hash = hashOfSymbol(keyword);
  ret->name = keyword;
  ret->length = strlen(keyword);
  ret->token = token;
  
  ret->isUppercase = isUppercase_String(keyword);
  ret->isFullcase = isFullcase_String(keyword);
  ret->startsUnderscore = startsUnderscore_String(keyword);

  return ret;
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

  // Init symbols with base types
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Char", TOKEN_Char));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Short", TOKEN_Short));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Int", TOKEN_Int));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Long", TOKEN_Long));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Float", TOKEN_Float));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Double", TOKEN_Double));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("String", TOKEN_String));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Bool", TOKEN_Bool));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Unsigned", TOKEN_Unsigned));

  // Init symbols with base keywords
  parlist_push(ret->symbols, symbol_CreateFromKeyword("else", TOKEN_Else));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("enum", TOKEN_Enum));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("if", TOKEN_If));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("return", TOKEN_Return));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("sizeof", TOKEN_Sizeof));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("while", TOKEN_While));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("for", TOKEN_For));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("switch", TOKEN_Switch));

  return ret;
}

/* ##### TYPES ##### */
s_anyvalue s_anyvalue_createPrimary(int type, void *value) {
  s_anyvalue ret;
  ret.type.isPrimary = true;
  ret.type.type = type;
  ret.content = value;
  return ret;
}

/* ##### Parser ##### */
s_parser *parse_Init(char *str) {
  s_parser *ret = NEW(s_parser);

  ret->line = 1;
  ret->ptr = ret->source = str;
  if (!ret->source) { PERROR("parse_Init", "Src is null"); return NULL; }

  return ret;
}

bool parse_IsTokenBasicType(s_token token) {
  return (token.type >= TOKEN_Char) && (token.type <= TOKEN_Unsigned);
}

int parse_Next(s_scope *scope, s_parser *parser) {
  #define src parser->ptr
  #define ret(TOKEN, VALUE, VALUETYPE) { parser->token.value.VALUETYPE = (VALUE); parser->token.type = (TOKEN); return (TOKEN); }

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

      while ((symbol_ptr != NULL) && symbol_ptr->token) {
        if (symbol_ptr->hash) {
          if (!memcmp(symbol_ptr->name, last_pos, symbol_ptr->length)) {
            // Found symbol, return it
            ret(symbol_ptr->token, symbol_ptr, symbol);
          }
        }
        symbol_ptr = (s_symbol *)parlist_read_next(scope->symbols);
      }

      // No symbol found, create one
      s_symbol *new_symbol = NEW(s_symbol);

      new_symbol->hash = hash;
      new_symbol->name = last_pos;
      new_symbol->length = src - last_pos;
      new_symbol->token = TOKEN_Symbol;
      
      new_symbol->isUppercase = isUppercase_String(last_pos);
      new_symbol->isFullcase = isFullcase_String(last_pos);
      new_symbol->startsUnderscore = startsUnderscore_String(last_pos);
      
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
        src ++;
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
      ret(TOKEN_Mul, NULL, any);
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
}

/* ##### STATEMENT ##### */
s_statement *statement_Create(s_compiler *compiler, s_statement *parent, e_statementtype type) {
  s_statement *ret = NEW(s_statement);

  if (parent == NULL) { // Root statement
    ret->parent = NULL;
    ret->scope = compiler->rootScope;
  } else {
    ret->parent = parent;
    ret->scope = scope_Create(parent->scope);
  }

  ret->type = type;
  
  return ret;
}


/* ##### Compiler ##### */
s_compiler *compiler_Init(char *content) {
  s_compiler *ret = NEW(s_compiler);

  ret->rootScope = scope_CreateAsRoot();
  if (!ret->rootScope) { PERROR("compiler_Init", "Could not create root scope"); return NULL; }

  ret->rootStatement = statement_Create(ret, NULL, STATEMENT);
  if (!ret->rootStatement) { PERROR("compiler_Init", "Could not malloc root statement"); return NULL; }

  s_statementbody_statement *statement_body = NEW(s_statementbody_statement);
  if (!statement_body) { PERROR("compiler_Init", "Could not malloc statement body"); return NULL; }

  statement_body->statements = list_create();
  ret->rootStatement->body = statement_body;

  ret->parser = parse_Init(content);

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

  compile_Statement(compiler, compiler->rootStatement);
  compile_Statement(compiler, compiler->rootStatement);
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

s_expression_operation *expression_Emit_Callback(s_list *core_operations, int type, void (*cb)(s_stack__s_anyvalue *)) {
  s_expression_operation *op = NEW(s_expression_operation);
  s_token *token_copy = NEW(s_token);
  token_copy->type = type;
  token_copy->value.any = cb;
  op->token = token_copy;
  list_push(core_operations, op);
  return op;
}

#define token (compiler->parser->token)
#define match(S, A) parse_Match((S), compiler->parser, (A))
#define emit(O, A) list_push(O, (A))


void expression_Step(s_compiler *compiler, s_statement *statement, int level) {
  s_list *core_operations = ((s_statementbody_expression *)statement->body)->core_operations;
  
  // Unary operators
  if (token.type == TOKEN_Literal_Int) {
    expression_Emit(core_operations, token);
    match(statement->scope, TOKEN_Literal_Int);
  } else if (token.type == TOKEN_Symbol) {
    expression_Emit(core_operations, token);
    match(statement->scope, TOKEN_Symbol);
  } else if (token.type == '(') {
    match(statement->scope, '(');
    expression_Step(compiler, statement, TOKEN_Assign);
    match(statement->scope, ')');
  } else if (token.type == TOKEN_Add) {
    // Convert string to number (Javascript like)

  } else if (token.type == TOKEN_Inc || token.type == TOKEN_Dec) {

  } else if (token.type == TOKEN_Lt) {
    match(statement->scope, TOKEN_Lt);
    expression_Step(compiler, statement, TOKEN_Lt);
    expression_Emit_Callback(core_operations, TOKEN_Lt, &__core_print);
  } else {
    PERROR("expression_Step", "Bad expression");
    exit(-1);
  }

  // binary operator and postfix operators.
  while (token.type >= level) {
    if (token.type == TOKEN_Add) {
      match(statement->scope, TOKEN_Add);
      expression_Step(compiler, statement, TOKEN_Mul);

      expression_Emit_Callback(core_operations, TOKEN_Add, &__core_add);
    }
  }
}

s_statement *compile_Expression(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_Create(compiler, parent, EXPRESSION);

  ret->exe_cb = &__core_expression;

  s_statementbody_expression *statement_body = NEW(s_statementbody_expression);
  statement_body->core_operations = list_create();

  ret->body = statement_body;

  expression_Step(compiler, ret, TOKEN_Assign);

  return ret;
}

/*
void compiler_ClassDefinition(s_compiler *compiler) {
  s_symbol *class_symbol = token.symbol;
  s_symbol *parent_symbol = NULL;

  match(Id);

  if (token.type == ':') { // Has parent
    match(':');

    parent_symbol = token.symbol;

    match(Id);
  }

  if (token.type == '.') { // Is children
    match('.');

    parent_symbol = class_symbol;
    class_symbol = token.symbol;

    match(Id);
  }

  compiler_Statement(compiler);
}
*/
/*
void compiler_FunctionDefinition(s_compiler *compiler, s_symbol *name) {
  match('(');



}
*/

s_anytype *compile_VariableType(s_compiler *compiler, s_statement *statement) {
  s_anytype *ret = NEW(s_anytype);

  ret->isDictionary = false;
  ret->isList = false;
  ret->type = NULL;

  if (parse_IsTokenBasicType(token)) {
    // Basic type
    ret->type = token.type;
    match(statement->scope, token.type);
  } else if (token.type == '[') {
    // List
    match(statement->scope, '[');

    s_listtype *ltype = NEW(s_listtype);
    ltype->items = compile_VariableType(compiler, statement);

    ret->isList = true;
    ret->type = ltype;

    match(statement->scope, ']');
  } else if (token.type == '{') {
    // Dictionary
    match(statement->scope, '{');

    if (!parse_IsTokenBasicType(token)) {
      PERROR("compile_VariableType", "Dictionary key can only be of basic type");
      exit(-1);
    }

    s_dictionarytype *dtype = NEW(s_dictionarytype);
    dtype->key = token.type;
    match(statement->scope, token.type);

    match(statement->scope, ',');

    dtype->value = compile_VariableType(compiler, statement);

    ret->isDictionary = true;
    ret->type = dtype;

    match(statement->scope, '}');
  } else {
    PERROR("compile_VariableType", "Unsupported complex variable definition");
    exit(-1);
  }

  return ret;
}

s_statement *compile_VariableDefinition(s_symbol *name, s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_Create(compiler, parent, VARIABLE_DEF);

  ret->exe_cb = &__core_variable_def;

  s_statementbody_variable *statement_body = NEW(s_statementbody_variable);
  statement_body->symbol = name;
  ret->body = statement_body;

  match(ret->scope, ':');

  s_symbolbody_variable *symbol_body = NEW(s_symbolbody_variable);
  name->body = symbol_body;
  name->type = VARIABLE;

  s_anytype *type = compile_VariableType(compiler, ret);
  symbol_body->value.type = *type;
  symbol_body->init_expression = NULL;

  if (token.type == TOKEN_Assign) {
    match(ret->scope, TOKEN_Assign);

    symbol_body->init_expression = compile_Expression(compiler, ret);
  }

  match(ret->scope, ';');

  return ret;
}

void compile_Statement(s_compiler *compiler, s_statement *statement) {
  // Statements types:
  // 1. if (...) <statement> [else <statement>]
  // 2. for (...) <statement>
  // 3. while (...) <statement>
  // 4. { <statement> }
  // 5. return xxx;
  // 6. <empty statement>;
  // 7. variable : type [ = <expression>];
  // 8. function(...) : type { <statement> }
  // 9. class { <statement> }
  // 9. expression; (expression end with semicolon)
  s_statementbody_statement *statement_body = (s_statementbody_statement *)statement->body;

  if (token.type == TOKEN_If) {

  } else if (token.type == TOKEN_For) {

  } else if (token.type == TOKEN_While) {

  } else if (token.type == '{') {
    // Block statement
    match(statement->scope, '{');

    while (token.type != '}') {
      s_statement *sub_statement = statement_Create(compiler, statement, STATEMENT);
      sub_statement->exe_cb = &__core_exe_statement;

      s_statementbody_statement *sub_statement_body = NEW(s_statementbody_statement);
      sub_statement_body->statements = list_create();
      sub_statement->body = sub_statement_body;

      list_push(statement_body->statements, sub_statement);
      compile_Statement(compiler, sub_statement);
    }

    match(statement->scope, '}');
  } else if (token.type == TOKEN_Return) {

  } else if (token.type == ';') {
    // Empty statement
    match(statement->scope, ';');
  } else if (token.type == TOKEN_Symbol) {
    if (token.value.symbol->isUppercase) { // Only class
      //compiler_ClassDefinition(compiler);
    } else if (token.value.symbol->isFullcase) { // Constant property

    } else if (token.value.symbol->startsUnderscore) { // Private property

    } else {
      s_symbol *name_symbol = token.value.symbol;
      match(statement->scope, TOKEN_Symbol);

      if (token.type == ':') {
        // Variable definition
        s_statement *sub_statement = compile_VariableDefinition(name_symbol, compiler, statement);
        list_push(statement_body->statements, sub_statement);
      } else if (token.type == '(') {
        // Function definition
        //compiler_FunctionDefinition(compiler, name_symbol);
      } else {
        // Expression (espresso ?)
        //compiler_Expression(compiler);
        match(statement->scope, ';');
      }
    }
  } else {
    // Expression (espresso ?)
    s_statement *sub_statement = compile_Expression(compiler, statement);
    list_push(statement_body->statements, sub_statement);
    match(statement->scope, ';');
  }
}


#undef emit
#undef match
#undef token

/* ##### CORE Libs ##### */
void __core_print(s_stack__s_anyvalue *stack) {
  printf("__core_print");

}

void __core_assign(s_symbol *symbol, s_anyvalue item) {
  if (symbol->type != VARIABLE) { PERROR("__core_assign", "Wrong symbol type"); exit(1); }

  s_symbolbody_variable *symbol_body = (s_symbolbody_variable *)symbol->body;

  bool isPrimary = symbol_body->value.type.isPrimary && item.type.isPrimary;
  if (isPrimary) {
    if (symbol_body->value.type.type == item.type.type) {
      symbol_body->value.content = item.content;
    }
  }
}

void __core_add(s_stack__s_anyvalue *stack) {
  printf("__core_add");

  s_anyvalue a = stack_pop__s_anyvalue(stack);
  s_anyvalue b = stack_pop__s_anyvalue(stack);
  s_anyvalue r;

  bool isPrimary = a.type.isPrimary && b.type.isPrimary;
  if (isPrimary) {
    if (a.type.type == b.type.type) {
      int mainType = (int)a.type.type;
      if ((mainType >= TYPE_Int8) && (mainType <= TYPE_Int64)) {
        r = s_anyvalue_createPrimary(mainType, ((int64_t)a.content + (int64_t)b.content));
      } else if ((mainType >= TYPE_UInt8) && (mainType <= TYPE_UInt64)) {
        r = s_anyvalue_createPrimary(mainType, ((u_int64_t)a.content + (u_int64_t)b.content));
      } else if (mainType == TYPE_Float32) {
        //r = s_anyvalue_createPrimary(mainType, ((float)a.content + (float)b.content));
      } else if (mainType == TYPE_Float64) {
        //r = s_anyvalue_createPrimary(mainType, ((double)a.content + (double)b.content));
      } else {
        PERROR("__core_add", "Unsupported primary type casting");
      }
    }
  } else {
    PERROR("__core_add", "Unsupported operation");
  }

  stack_push__s_anyvalue(stack, r);
}
void __core_sub(s_stack__s_anyvalue *stack) {}
void __core_mul(s_stack__s_anyvalue *stack) {}
void __core_div(s_stack__s_anyvalue *stack) {}

s_anyvalue __core_exe_expression(s_statement *statement) {
  s_stack__s_anyvalue stack = stack_create__s_anyvalue(50);
  stack.ptr = 0;

  s_statementbody_expression *statement_body = (s_statementbody_expression *)statement->body;
  s_list *core_operations = statement_body->core_operations;

  s_expression_operation *op = list_read_first(core_operations);

  do {
    if (op->token->type == TOKEN_Literal_Int) {
      stack_push__s_anyvalue(&stack, s_anyvalue_createPrimary(TYPE_Int64, op->token->value.integer));
    } else {
      void (*cb)(s_stack__s_anyvalue *stack) = op->token->value.any;
      if (cb)
        cb(&stack);
    }
    op = list_read_next(core_operations);
  } while (op != NULL);

  s_anyvalue ret = stack_pop__s_anyvalue(&stack);

  stack_free__s_anyvalue(stack);

  return ret;
}

void __core_expression(s_statement *statement) {
  s_anyvalue exp_result = __core_exe_expression(statement);
}

void __core_variable_def(s_statement *statement) {
  if (statement->type != VARIABLE_DEF) { PERROR("__core_variable_def", "Wrong statement type"); exit(1); }

  s_statementbody_variable *statement_body = (s_statementbody_variable *)statement->body;
  s_symbol *symbol = statement_body->symbol;
  s_symbolbody_variable *symbol_body = (s_symbolbody_variable *)symbol->body;

  if (symbol_body->init_expression != NULL) {
    s_anyvalue exp_result = __core_exe_expression(symbol_body->init_expression);
    __core_assign(symbol, exp_result);
  }
}

void __core_call_function(s_statement *statement) {}

void __core_exe_statement(s_statement *statement) {
  if (statement->type == STATEMENT) {
    s_statementbody_statement *statement_body = (s_statementbody_statement *)statement->body;

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

void __core_new_class_instance() {}


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
" UP Interpreter  v0.2 \n";
  printf(intro);

  char *srcFilename = "helloworld.up";

  // Compiler
  s_compiler *compiler = compiler_InitFromFile(srcFilename);
  
  compiler_Execute(compiler);

  return 0;
}