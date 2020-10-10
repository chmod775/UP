#ifndef UP
#define UP

#define PERROR(A, B, ...) printf("[" A "] Error! - " B "\n", ##__VA_ARGS__);

/* ##### Helpers shit ##### */
FILE *openFile(char *filename);

void closeFile(FILE *fd);

bool isUppercase_Char(char ch);
bool isUppercase_String(char *str);

bool isFullcase_String(char *str);

bool startsUnderscore_String(char *str);

typedef struct list_item {
  void *value;
  struct list_item *next;
} s_list_item;

typedef struct {
  u_int64_t items_count;
  s_list_item *head_item;
  s_list_item *selected_item;
} s_list;

s_list *list_create();

void *list_read_index(s_list *l, u_int64_t index);
void *list_read_first(s_list *l);
void *list_read_last(s_list *l);
void *list_read_next(s_list *l);
void *list_read_previous(s_list *l);

void list_add(s_list *l, void *value);

void *list_pull(s_list *l);

void list_push(s_list *l, void *value);

void *list_pop(s_list *l);

/* ##### CORE ##### */


/* ##### Symbols ##### */
typedef enum {
  NOTDEFINED,
  VARIABLE,
  FUNCTION,
  CLASS
} e_symboltype;

typedef struct {
  int hash;
  char *name;
  int length;

  int token;

  bool isUppercase;
  bool isFullcase;
  bool startsUnderscore;

  e_symboltype type;
  void *body;
} s_symbol;

char *symbol_GetCleanName(s_symbol *symbol);

/* ##### Scope ##### */
typedef struct {
  s_list *symbols;
} s_scope;

s_scope *scope_Init(int sizeLimit);

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

s_parser *parse_Init(s_scope *scope, char *str);

bool parse_IsTokenBasicType(s_token token);

s_parser *parse_InitFromFile(s_scope *scope, char *filename, int sizeLimit);

int parse_Next(s_parser *parser);

int parse_Match(s_parser *parser, int token);

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


/* ##### Compiler ##### */
typedef struct {
  s_parser *parser;
  s_list *statements;
} s_compiler;

s_compiler *compiler_Init(s_parser *parser, int sizeLimit);
void compiler_Next(s_compiler *compiler);

/* ##### Expression ##### */
typedef struct {
  s_list *core_operations;
} s_expression;

s_expression *compiler_Expression(s_compiler *compiler);
s_expression *compiler_ExpressionStep(s_compiler *compiler, s_expression *exp, int level);

/* ##### VARIABLE ##### */
typedef struct {
  s_anytype *type;
  s_expression *init_expression;
} s_symbolbody_variable;

s_anytype *compiler_VariableType(s_compiler *compiler);
void compiler_VariableDefinition(s_compiler *compiler, s_symbol *name);

/* ##### FUNCTION ##### */
typedef struct {
  s_symbol *symbol;
} s_function_argument;

typedef struct {
  s_anytype ret_type;
  s_list *arguments;
} s_symbolbody_function;

void compiler_FunctionDefinition(s_compiler *compiler, s_symbol *name);

/* ##### CLASS ##### */
typedef struct {
  void *TODO;
} s_symbolbody_class;
void compiler_ClassDefinition(s_compiler *compiler);


/* ##### STATEMENT ##### */
typedef struct {

} s_statement;

void compiler_Statement(s_compiler *compiler);


#endif