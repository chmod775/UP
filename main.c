#include <stdio.h>
#include <stdlib.h>

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

/* ##### Scope ##### */
typedef struct {
  char *name;
  int hash;
  int type;
  int token;
  void *value;
} s_symbol;

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
  s_scope *scope;
  char *source;
  char *ptr;
  int line;
  int token;
  s_symbol *symbol;
} s_parser;

typedef enum {
  Num = 128, Fun, Sys, Glo, Loc, Id, Class,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
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
  #define ret(TOKEN, SYMBOL) { parser->symbol = (SYMBOL); parser->token = (TOKEN); return (TOKEN); }

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
            ret(symbol_ptr->token, symbol_ptr);
          }
        }
        symbol_ptr++;
      }

      // No symbol found, create one
      symbol_ptr->name = last_pos;
      symbol_ptr->hash = hash;
      symbol_ptr->token = Id;
      
      ret(token, symbol_ptr);
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

      ret(token, NULL);
    }
  }

  ret(token, NULL);
}

int parse_Match(s_parser *parser, int token) {
  if (parser->token == token) {
    parse_Next(parser);
  } else {
    printf("%d: expected token: %d\n", parser->line, token);
    exit(-1);
  }
}

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

int compiler_Next(s_compiler *compiler) {
  int token = parse_Next(compiler->parser);


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
" UP Interpreter  v0.1 \n";
  printf(intro);

  char *srcFilename = "helloworld.up";

  const int poolsize = 256 * 1024; // arbitrary size

  rootScope = scope_Init(poolsize);

  // Init scope with keywords
  const char *keywords = "char else enum if int return sizeof while";
  s_parser *_parser = parse_Init(rootScope, keywords);

  int i = Char;
  while (i <= While) {
    parse_Next(_parser);
    _parser->symbol->token = i++;
  }

  // Source code parser
  s_parser *parser = parse_InitFromFile(rootScope, srcFilename, poolsize);

  int tok = parse_Next(parser);
  printf("token: %d [%c]\n", tok, tok);

  return 0;
}