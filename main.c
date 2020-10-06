#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

/* ##### Symbols ##### */
typedef struct {
  int hash;
  char *name;
  int length;

  int type;
  int token;
  void *value;

  bool isUppercase;
  bool isFullcase;
  bool startsUnderscore;
} s_symbol;

char *symbol_GetCleanName(s_symbol *symbol) {
  char *ret = (char *)malloc(sizeof(char) * (symbol->length + 1));
  memcpy(ret, symbol->name, symbol->length);
  ret[symbol->length] = 0;
  return ret;
}

/* ##### Scope ##### */
typedef struct {
  s_symbol *symbols;
} s_scope;

s_scope *scope_Init(int sizeLimit) {
  s_scope *ret = (s_scope *)malloc(sizeof(s_scope));
  if (!ret) { PERROR("scope_Init", "Could not malloc scope"); return NULL; }

  ret->symbols = (s_symbol *)malloc(sizeof(s_symbol) * sizeLimit);
  if (!ret->symbols) { PERROR("scope_Init", "Could not malloc symbols table"); return NULL; }

  return ret;
}

/* ##### Parser ##### */
typedef struct {
  int type;
  int64_t value;
  s_symbol *symbol;
} s_token;

typedef struct {
  s_scope *scope;
  char *source;
  char *ptr;
  int line;
  s_token token;
} s_parser;

typedef enum {
  Num = 128, Fun, Sys, Glo, Loc, Id, Class, Var,
  Char, Short, Int, Long, Float, Double, String, Bool, Unsigned,
  Else, Enum, If, Return, Sizeof, While, For, Switch,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
} e_token;

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
      s_symbol *symbol_ptr = parser->scope->symbols;
      while (symbol_ptr->token) {
        if (symbol_ptr->hash) {
          if (!memcmp(symbol_ptr->name, last_pos, src - last_pos)) {
            // Found symbol, return it
            ret(symbol_ptr->token, last_pos, symbol_ptr);
          }
        }
        symbol_ptr++;
      }

      // No symbol found, create one
      symbol_ptr->hash = hash;
      symbol_ptr->name = last_pos;
      symbol_ptr->length = src - last_pos;
      symbol_ptr->token = Id;
      
      symbol_ptr->isUppercase = isUppercase_String(last_pos);
      symbol_ptr->isFullcase = isFullcase_String(last_pos);
      symbol_ptr->startsUnderscore = startsUnderscore_String(last_pos);
      
      ret(Id, last_pos, symbol_ptr);
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

      ret(token, token_val, NULL);
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
      printf("%d: expected token: %c\n", parser->line, token);
    else
      printf("%d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
}

/* ##### Types ##### */
typedef struct {
  bool isDictionary;
  bool isList;
  void *type;
} s_anytype;

typedef struct {
  int key;
  s_anytype *value;
} s_dictionarytype;

typedef struct {
  s_anytype *items;
} s_listtype;

/* ##### Expression ##### /*

/* ##### Compiler ##### */
typedef struct {
  s_parser *parser;
  void *text;
  void *data;
} s_compiler;

s_compiler *compiler_Init(s_parser *parser, int sizeLimit) {
  s_compiler *ret = (s_compiler *)malloc(sizeof(s_compiler));
  if (!ret) { PERROR("compiler_Init", "Could not malloc compiler"); return NULL; }

  ret->parser = parser;

  return ret;
}

#define token (compiler->parser->token)
#define match(A) parse_Match(compiler->parser, (A))

void compiler_Expression(s_compiler *compiler) {

}

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
      PERROR("compiler_VariableType", "Dictionary key can only be a Basic type");
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

  s_anytype *type = compiler_VariableType(compiler);

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
  // 7. variable : type [ = initialization];
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