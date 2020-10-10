#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "up.h"

#define PERROR(A, B, ...) printf("[" A "] Error! - " B "\n", ##__VA_ARGS__);
#define NEW(T) ((T *)malloc(sizeof(T)))

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
  s_list *ret = (s_list *)malloc(sizeof(s_list));
  if (!ret) { PERROR("list_create", "Could not malloc list"); return NULL; }

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
  s_list_item *item = (s_list_item *)malloc(sizeof(s_list_item));
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
  s_list_item *item = (s_list_item *)malloc(sizeof(s_list_item));
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
  s_parlist *ret = (s_parlist *)malloc(sizeof(s_parlist));
  if (!ret) { PERROR("parlist_create", "Could not malloc parlist"); return NULL; }

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
  s_symbol *ret = (s_symbol *)malloc(sizeof(s_symbol));
  if (!ret) { PERROR("symbol_CreateFromKeyword", "Could not malloc symbol"); return NULL; }

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
  s_scope *ret = (s_scope *)malloc(sizeof(s_scope));
  if (!ret) { PERROR("scope_Init", "Could not malloc scope"); return NULL; }

  ret->parent = parent;

  ret->symbols = parlist_create(parent == NULL ? NULL : parent->symbols);
  if (!ret->symbols) { PERROR("scope_Init", "Could not create symbols list"); return NULL; }

  return ret;
}

s_scope *scope_CreateAsRoot() {
  s_scope *ret = scope_Create(NULL);

  // Init symbols with base types
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Char", Char));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Short", Short));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Int", Int));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Long", Long));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Float", Float));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Double", Double));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("String", String));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Bool", Bool));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("Unsigned", Unsigned));

  // Init symbols with base keywords
  parlist_push(ret->symbols, symbol_CreateFromKeyword("else", Else));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("enum", Enum));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("if", If));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("return", Return));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("sizeof", Sizeof));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("while", While));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("for", For));
  parlist_push(ret->symbols, symbol_CreateFromKeyword("switch", Switch));

  return ret;
}

/* ##### Parser ##### */
s_parser *parse_Init(char *str) {
  s_parser *ret = (s_parser *)malloc(sizeof(s_parser));
  if (!ret) { PERROR("parse_Init", "Could not malloc parser"); return NULL; }

  ret->line = 1;
  ret->ptr = ret->source = str;
  if (!ret->source) { PERROR("parse_Init", "Src is null"); return NULL; }

  return ret;
}

bool parse_IsTokenBasicType(s_token token) {
  return (token.type >= Char) && (token.type <= Unsigned);
}

int parse_Next(s_scope *scope, s_parser *parser) {
  #define src parser->ptr
  #define ret(TOKEN, VALUE, SYMBOL) { parser->token.symbol = (SYMBOL); parser->token.value = (VALUE); parser->token.type = (TOKEN); return (TOKEN); }

  int token;                    // current token
  int token_val;                // value of current token (mainly for number)
  char *last_pos;
  int hash;

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
            ret(symbol_ptr->token, last_pos, symbol_ptr);
          }
        }
        symbol_ptr = (s_symbol *)parlist_read_next(scope->symbols);
      }

      // No symbol found, create one
      s_symbol *new_symbol = (s_symbol *)malloc(sizeof(s_symbol));

      new_symbol->hash = hash;
      new_symbol->name = last_pos;
      new_symbol->length = src - last_pos;
      new_symbol->token = Id;
      
      new_symbol->isUppercase = isUppercase_String(last_pos);
      new_symbol->isFullcase = isFullcase_String(last_pos);
      new_symbol->startsUnderscore = startsUnderscore_String(last_pos);
      
      parlist_push(scope->symbols, new_symbol);

      ret(Id, last_pos, new_symbol);
    }
    else if (token >= '0' && token <= '9') {
      // parse number, three kinds: dec(123) hex(0x123) oct(017)
      token_val = token - '0';
      if (token_val > 0) {
        // dec, starts with [1-9]
        while (*src >= '0' && *src <= '9') {
          token_val = token_val*10 + *src++ - '0';
        }
      } else {
        // starts with 0
        if (*src == 'x' || *src == 'X') {
          //hex
          token = *++src;
          while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
            token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
            token = *++src;
          }
        } else {
            // oct
          while (*src >= '0' && *src <= '7') {
            token_val = token_val*8 + *src++ - '0';
          }
        }
      }
      ret(Num, token_val, NULL);
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
        ret(Div, NULL, NULL);
      }
    }
    else if (token == '=') {
      // parse '==' and '='
      if (*src == '=') {
        src ++;
        ret(Eq, NULL, NULL);
      } else {
        ret(Assign, NULL, NULL);
      }
    }
    else if (token == '+') {
      // parse '+' and '++'
      if (*src == '+') {
        src ++;
        ret(Inc, NULL, NULL);
      } else {
        ret(Add, NULL, NULL);
      }
    }
    else if (token == '-') {
      // parse '-' and '--'
      if (*src == '-') {
        src ++;
        ret(Dec, NULL, NULL);
      } else {
        ret(Sub, NULL, NULL);
      }
    }
    else if (token == '!') {
      // parse '!='
      if (*src == '=') {
        src++;
        ret(Ne, NULL, NULL);
      } else {
        ret(token, NULL, NULL);
      }
    }
    else if (token == '<') {
      // parse '<=', '<<' or '<'
      if (*src == '=') {
        src ++;
        ret(Le, NULL, NULL);
      } else if (*src == '<') {
        src ++;
        ret(Shl, NULL, NULL);
      } else {
        ret(Lt, NULL, NULL);
      }
    }
    else if (token == '>') {
      // parse '>=', '>>' or '>'
      if (*src == '=') {
        src ++;
        ret(Ge, NULL, NULL);
      } else if (*src == '>') {
        src ++;
        ret(Shr, NULL, NULL);
      } else {
        ret(Gt, NULL, NULL);
      }
    }
    else if (token == '|') {
      // parse '|' or '||'
      if (*src == '|') {
        src ++;
        ret(Lor, NULL, NULL);
      } else {
        ret(Or, NULL, NULL);
      }
    }
    else if (token == '&') {
      // parse '&' and '&&'
      if (*src == '&') {
        src ++;
        ret(Lan, NULL, NULL);
      } else {
        ret(And, NULL, NULL);
      }
    }
    else if (token == '^') {
      ret(Xor, NULL, NULL);
    }
    else if (token == '%') {
      ret(Mod, NULL, NULL);
    }
    else if (token == '*') {
      ret(Mul, NULL, NULL);
    }
    else if (token == '?') {
      ret(Cond, NULL, NULL);
    } else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == '[' || token == ']' || token == ',' || token == ':' || token == '.') {
      // directly return the character as token;
      ret(token, NULL, NULL);
    }
  }

  ret(token, NULL, NULL);

  #undef ret
  #undef src
}

int parse_Match(s_scope *scope, s_parser *parser, int token) {
  if (parser->token.type == token) {
    parse_Next(scope, parser);
  } else {
    if (token < Num)
      printf("Line %d: expected token: %c\n", parser->line, token);
    else
      printf("Line %d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
}

/* ##### STATEMENT ##### */
s_statement *statement_Create(s_compiler *compiler, s_statement *parent, e_statementtype type) {
  s_statement *ret = (s_statement *)malloc(sizeof(s_statement));
  if (!ret) { PERROR("statement_Create", "Could not malloc statement"); return NULL; }

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
  s_compiler *ret = (s_compiler *)malloc(sizeof(s_compiler));
  if (!ret) { PERROR("compiler_Init", "Could not malloc compiler"); return NULL; }

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

  __core_exe_statement(compiler->rootStatement);
}


s_expression_operation *expression_Emit(s_list *core_operations, int token, void *value) {
  s_expression_operation *op = NEW(s_expression_operation);
  op->token = token;
  op->value = value;
  return op;
}

#define token (compiler->parser->token)
#define match(S, A) parse_Match((S), compiler->parser, (A))
#define emit(O, A) list_push(O, (A))


void expression_Step(s_compiler *compiler, s_statement *statement, int level) {
  s_list *core_operations = ((s_statementbody_expression *)statement->body)->core_operations;
  
  // Unary operators
  if (token.type == Num) {
    expression_Emit(core_operations, token.type, token.value);
    match(statement->scope, Num);
  } else if (token.type == '(') {
    match(statement->scope, '(');
    expression_Step(compiler, statement, Assign);
    match(statement->scope, ')');
  } else if (token.type == Add) {
    // Convert string to number (Javascript like)

  } else if (token.type == Inc || token.type == Dec) {

  } else {
    PERROR("testExpr_Step", "Bad expression");
    exit(-1);
  }

  // binary operator and postfix operators.
  while (token.type >= level) {
    if (token.type == Add) {
      match(statement->scope, Add);
      expression_Step(compiler, statement, Mul);

      expression_Emit(core_operations, Add, &__core_add);
    }
  }
}

s_statement *compile_Expression(s_compiler *compiler, s_statement *parent) {
  s_statement *ret = statement_Create(compiler, parent, EXPRESSION);

  s_statementbody_expression *statement_body = NEW(s_statementbody_expression);
  statement_body->core_operations = list_create();

  ret->body = statement_body;

  expression_Step(compiler, ret, Assign);

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
  s_anytype *ret = (s_anytype *)malloc(sizeof(s_anytype));

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

    s_listtype *ltype = (s_listtype *)malloc(sizeof(s_listtype));
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

    s_dictionarytype *dtype = (s_dictionarytype *)malloc(sizeof(s_dictionarytype));
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
  name->type != VARIABLE;

  s_anytype *type = compile_VariableType(compiler, ret);
  symbol_body->value.type = type;
  symbol_body->init_expression = NULL;

  if (token.type == Assign) {
    match(ret->scope, Assign);

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

  if (token.type == If) {

  } else if (token.type == For) {

  } else if (token.type == While) {

  } else if (token.type == '{') {
    // Block statement
    match(statement->scope, '{');

    while (token.type != '}') {
      s_statement *sub_statement = statement_Create(compiler, statement, STATEMENT);

      s_statementbody_statement *sub_statement_body = NEW(s_statementbody_statement);
      sub_statement_body->statements = list_create();
      sub_statement->body = sub_statement_body;

      sub_statement->exe_cb = &__core_exe_statement;
      list_push(statement_body->statements, sub_statement);
      compile_Statement(compiler, sub_statement);
    }

    match(statement->scope, '}');
  } else if (token.type == Return) {

  } else if (token.type == ';') {
    // Empty statement
    match(statement->scope, ';');
  } else if (token.type == Id) {
    if (token.symbol->isUppercase) { // Only class
      //compiler_ClassDefinition(compiler);
    } else if (token.symbol->isFullcase) { // Constant property

    } else if (token.symbol->startsUnderscore) { // Private property

    } else {
      s_symbol *name_symbol = token.symbol;
      match(statement->scope, Id);

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
    //compiler_Expression(compiler);
    match(statement->scope, ';');
  }
}


#undef emit
#undef match
#undef token

/* ##### CORE Libs ##### */
void __core_assign(s_symbol *symbol, s_anyvalue item) {
  if (symbol->type != VARIABLE) { PERROR("__core_assign", "Wrong symbol type"); exit(1); }

  s_symbolbody_variable *symbol_body = (s_symbolbody_variable *)symbol->body;

  if (symbol_body->value.type == item.type) {
    symbol_body->value.content = item.content;
  } else { // Casting required

  }
}

void __core_add(__core_expression_stack *stack) {
  printf("__core_add");
}
void __core_sub(__core_expression_stack *stack) {}
void __core_mul(__core_expression_stack *stack) {}
void __core_div(__core_expression_stack *stack) {}

s_anyvalue __core_exe_expression(s_statement *statement) {
  __core_expression_stack stack;
  stack.ptr = 0;

  s_statementbody_expression *statement_body = (s_statementbody_expression *)statement->body;
  s_list *core_operations = statement_body->core_operations;

  s_expression_operation *op = list_read_first(core_operations);

  do {
    if (op->token == Num) {
      stack.content[stack.ptr].content = op->value;
    } else {
      void (*cb)(__core_expression_stack *stack) = op->value;
      cb(&stack);
    }
    op = list_read_next(core_operations);
  } while (op != NULL);
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