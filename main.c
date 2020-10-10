#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "up.h"

#define PERROR(A, B, ...) printf("[" A "] Error! - " B "\n", ##__VA_ARGS__);

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

bool isUppercase_Char(char ch) { return (ch >= 'A' && ch <= 'Z'); }
bool isUppercase_String(char *str) { return isUppercase_Char(str[0]); }

bool isFullcase_String(char *str) { return isUppercase_Char(str[0]) && isUppercase_Char(str[1]); }

bool startsUnderscore_String(char *str) { return str[0] == '_'; }

s_list *list_create() {
  s_list *ret = (s_list *)malloc(sizeof(s_list));
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

/* ##### Symbols ##### */
char *symbol_GetCleanName(s_symbol *symbol) {
  char *ret = (char *)malloc(sizeof(char) * (symbol->length + 1));
  memcpy(ret, symbol->name, symbol->length);
  ret[symbol->length] = 0;
  return ret;
}

/* ##### Scope ##### */
s_scope *scope_Init(int sizeLimit) {
  s_scope *ret = (s_scope *)malloc(sizeof(s_scope));
  if (!ret) { PERROR("scope_Init", "Could not malloc scope"); return NULL; }

  ret->symbols = list_create();
  if (!ret->symbols) { PERROR("scope_Init", "Could not create symbols list"); return NULL; }

  return ret;
}

/* ##### Parser ##### */
s_parser *parse_Init(s_scope *scope, char *str) {
  s_parser *ret = (s_parser *)malloc(sizeof(s_parser));
  if (!ret) { PERROR("parse_Init", "Could not malloc parser"); return NULL; }

  ret->scope = scope;
  ret->line = 1;
  ret->ptr = ret->source = str;
  if (!ret->source) { PERROR("parse_Init", "Src is null"); return NULL; }

  return ret;
}

bool parse_IsTokenBasicType(s_token token) {
  return (token.type >= Char) && (token.type <= Unsigned);
}

s_parser *parse_InitFromFile(s_scope *scope, char *filename, int sizeLimit) {
  // Open source file
  FILE *fd = openFile(filename);
  if (!fd) { PERROR("parse_InitFromFile", "Error opening file"); return NULL; }

  // Malloc source code area
  char *content = (char *)malloc(sizeLimit * sizeof(char));
  if (!content) { PERROR("parse_InitFromFile", "Could not malloc source code area"); return NULL; }

  // Read the source file
  int l;
  if ((l = fread(content, 1, sizeLimit-1, fd)) <= 0) { PERROR("parse_InitFromFile", "fread() returned %d", l); return NULL; }
  content[l] = 0; // add EOF character

  // Close file
  closeFile(fd);

  // Create parser with source code
  s_parser *ret = parse_Init(scope, content);

  return ret;
}

int parse_Next(s_parser *parser) {
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
      s_symbol *symbol_ptr = (s_symbol *)list_read_first(parser->scope->symbols);

      while ((symbol_ptr != NULL) && symbol_ptr->token) {
        if (symbol_ptr->hash) {
          if (!memcmp(symbol_ptr->name, last_pos, src - last_pos)) {
            // Found symbol, return it
            ret(symbol_ptr->token, last_pos, symbol_ptr);
          }
        }
        symbol_ptr = (s_symbol *)list_read_next(parser->scope->symbols);
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
      
      list_push(parser->scope->symbols, new_symbol);

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

int parse_Match(s_parser *parser, int token) {
  if (parser->token.type == token) {
    parse_Next(parser);
  } else {
    if (token < Num)
      printf("Line %d: expected token: %c\n", parser->line, token);
    else
      printf("Line %d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
}

/* ##### Compiler ##### */
s_compiler *compiler_Init(s_parser *parser, int sizeLimit) {
  s_compiler *ret = (s_compiler *)malloc(sizeof(s_compiler));
  if (!ret) { PERROR("compiler_Init", "Could not malloc compiler"); return NULL; }

  ret->parser = parser;

  return ret;
}

#define token (compiler->parser->token)
#define match(A) parse_Match(compiler->parser, (A))



#define emit(A) list_push(exp->core_operations, (A))

s_expression *compiler_Expression(s_compiler *compiler) {
  s_expression *ret = (s_expression *)malloc(sizeof(s_expression));
  ret->core_operations = list_create();

  return compiler_ExpressionStep(compiler, ret, Assign);
}

s_expression *compiler_ExpressionStep(s_compiler *compiler, s_expression *exp, int level) {
  // Unary operators
  if (token.type == Num) {
    emit(token.value);
    match(Num);
  } else if (token.type == '(') {
    match('(');
    compiler_ExpressionStep(compiler, exp, Assign);
    match(')');
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
      match(Add);
      compiler_ExpressionStep(compiler, exp, Mul);

      emit(Add + 1000);
    }
  }
}

#undef emit

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

s_anytype *compiler_VariableType(s_compiler *compiler) {
  s_anytype *ret = (s_anytype *)malloc(sizeof(s_anytype));

  ret->isDictionary = false;
  ret->isList = false;
  ret->type = NULL;

  if (parse_IsTokenBasicType(token)) {
    // Basic type
    ret->type = token.type;
    match(token.type);
  } else if (token.type == '[') {
    // List
    match('[');

    s_listtype *ltype = (s_listtype *)malloc(sizeof(s_listtype));
    ltype->items = compiler_VariableType(compiler);

    ret->isList = true;
    ret->type = ltype;

    match(']');
  } else if (token.type == '{') {
    // Dictionary
    match('{');

    if (!parse_IsTokenBasicType(token)) {
      PERROR("compiler_VariableType", "Dictionary key can only be of basic type");
      exit(-1);
    }

    s_dictionarytype *dtype = (s_dictionarytype *)malloc(sizeof(s_dictionarytype));
    dtype->key = token.type;
    match(token.type);

    match(',');

    dtype->value = compiler_VariableType(compiler);

    ret->isDictionary = true;
    ret->type = dtype;

    match('}');
  } else {
    PERROR("compiler_VariableType", "Unsupported complex variable definition");
    exit(-1);
  }

  return ret;
}

void compiler_FunctionDefinition(s_compiler *compiler, s_symbol *name) {
  match('(');



}

void compiler_VariableDefinition(s_compiler *compiler, s_symbol *name) {
  match(':');

  s_symbolbody_variable *symbol_body = (s_symbolbody_variable *)malloc(sizeof(s_symbolbody_variable));
  name->body = symbol_body;

  s_anytype *type = compiler_VariableType(compiler);
  symbol_body->type = type;

  if (token.type == Assign) {
    symbol_body->init_expression = compiler_Expression(compiler);
  }

  match(';');

  return;
}

void compiler_Statement(s_compiler *compiler) {
  // Statements types:
  // 1. if (...) <statement> [else <statement>]
  // 2. for (...) <statement>
  // 3. while (...) <statement>
  // 4. { <statement> }
  // 5. return xxx;
  // 6. <empty statement>;
  // 7. variable : type [ = expression];
  // 8. function(...) : type { statement }
  // 9. expression; (expression end with semicolon)

  if (token.type == If) {

  } else if (token.type == For) {

  } else if (token.type == While) {

  } else if (token.type == '{') {
    // Block statement
    match('{');

    while (token.type != '}') {
      compiler_Statement(compiler);
    }

    match('}');
  } else if (token.type == Return) {

  } else if (token.type == ';') {
    // Empty statement
    match(';');
  } else if (token.type == Id) {
    if (token.symbol->isUppercase) { // Only class
      compiler_ClassDefinition(compiler);
    } else if (token.symbol->isFullcase) { // Constant property

    } else if (token.symbol->startsUnderscore) { // Private property

    } else {
      s_symbol *name_symbol = token.symbol;
      match(Id);

      if (token.type == ':') {
        // Variable definition
        compiler_VariableDefinition(compiler, name_symbol);
      } else if (token.type == '(') {
        // Function definition
        compiler_FunctionDefinition(compiler, name_symbol);
      } else {
        // Expression (espresso ?)
        compiler_Expression(compiler);
        match(';');
      }
    }
  } else {
    // Expression (espresso ?)
    compiler_Expression(compiler);
    match(';');
  }
}

void compiler_Next(s_compiler *compiler) {
  compiler_Statement(compiler);
  parse_Next(compiler->parser);
}

#undef match
#undef token


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
" UP Interpreter  v0.1 \n";
  printf(intro);

  char *srcFilename = "helloworld.up";

  const int poolsize = 256 * 1024; // arbitrary size

  rootScope = scope_Init(poolsize);

  // Init scope with keywords
  s_parser *_parser;
  int i;

  const char *keywords_types = "Char Short Int Long Float Double String Bool Unsigned";
  _parser = parse_Init(rootScope, keywords_types);

  i = Char;
  while (i <= Unsigned) {
    parse_Next(_parser);
    _parser->token.symbol->token = i++;
  }

  const char *keywords = "else enum if return sizeof while for switch";
  _parser = parse_Init(rootScope, keywords);

  i = Else;
  while (i <= Switch) {
    parse_Next(_parser);
    _parser->token.symbol->token = i++;
  }

  // Source code parser
  s_parser *parser = parse_InitFromFile(rootScope, srcFilename, poolsize);

  // Compiler
  s_compiler *compiler = compiler_Init(parser, poolsize);

  int tok = parse_Next(parser);
  
  compiler_Next(compiler);

  return 0;
}