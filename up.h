#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef UP
#define UP

#define PERROR(A, B, ...) printf("[" A "] Error! - " B "\n", ##__VA_ARGS__);

/* ##### Helpers shit ##### */
FILE *openFile(char *filename);

void closeFile(FILE *fd);

int hashOfString(char *str);

bool isUppercase_Char(char ch);
bool isUppercase_String(char *str);

bool isFullcase_String(char *str);

bool startsUnderscore_String(char *str);

/* ##### LINKED LIST ##### */
typedef struct _s_list_item {
  void *value;
  struct _s_list_item *next;
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

/* ##### PARENTABLE LINKED LIST ##### */
typedef struct {
  s_list *lists;
  s_list *selected_list;
} s_parlist;

s_parlist *parlist_create(s_parlist *parent);

void *parlist_read_first(s_parlist *pl);
void *parlist_read_last(s_parlist *pl);
void *parlist_read_next(s_parlist *pl);
void *parlist_read_previous(s_parlist *pl);

void parlist_add(s_parlist *pl, void *value);

void parlist_push(s_parlist *pl, void *value);

/* ##### STACK ##### */
#define define_stack(T) \
typedef struct { \
  T *content; \
  u_int64_t ptr; \
  u_int64_t size; \
} s_stack__##T; \
s_stack__##T stack_create__##T(int size) { \
  s_stack__##T ret; \
  ret.size = size; \
  ret.ptr = 0; \
  ret.content = (T *)malloc(size * sizeof(T)); \
  return ret; \
} \
void stack_free__##T(s_stack__##T s) { \
  free(s.content); \
} \
void stack_push__##T(s_stack__##T *s, T value) { \
  s->content[s->ptr] = value; \
  s->ptr++; \
} \
T stack_pop__##T(s_stack__##T *s) { \
  s->ptr--; \
  return s->content[s->ptr]; \
} \

/* ##### Symbols ##### */
typedef enum {
  NOTDEFINED,
  KEYWORD,
  PRIMARYTYPE,
  VARIABLE,
  FUNCTION,
  CLASS
} e_symboltype;

typedef struct {
  int hash;
  char *name;
  int length;

  bool isUppercase;
  bool isFullcase;
  bool startsUnderscore;

  e_symboltype type;
  void *body;
} s_symbol;

char *symbol_GetCleanName(s_symbol *symbol);

/* ##### Scope ##### */
typedef struct _s_scope {
  struct _s_scope *parent;
  s_parlist *symbols;
} s_scope;

s_scope *scope_Create(s_scope *parent);
s_scope *scope_CreateAsRoot();

/* ##### Types ##### */
typedef enum {
  TYPE_Int8 = 0x10,
  TYPE_Int16,
  TYPE_Int32,
  TYPE_Int64,

  TYPE_UInt8 = 0x20,
  TYPE_UInt16,
  TYPE_UInt32,
  TYPE_UInt64,

  TYPE_Float32 = 0x40,
  TYPE_Float64
} e_primary_types;

typedef struct {
  e_primary_types type;
} s_symbolbody_primarytype;

typedef struct {
  bool isPrimary;
  bool isDictionary;
  bool isList;
  bool isClass;
  // In case of:
  //  Primary: type = e_primary_types
  //  Dictionary: *type = s_anytype
  //  List: *type = s_anytype
  //  Class: *type = s_symbol
  void *type;
} s_anytype;

typedef struct {
  s_anytype type;
  void *content;
} s_anyvalue;

typedef struct {
  int key;
  s_anytype *value;
} s_dictionarytype;

typedef struct {
  s_anytype *items;
} s_listtype;

s_anyvalue s_anyvalue_createPrimary(int type, void *value);

s_symbol *symbol_CreateFromPrimaryType(char *keyword, e_primary_types type);

/* ##### Parser ##### */
typedef union {
  int64_t integer;
  double decimal;
  char *string;
  s_symbol *symbol;
  void *any;
} u_token_value;

typedef struct {
  int type;
  u_token_value value;
} s_token;

typedef struct {
  char *source;
  char *ptr;
  int line;
  s_token token;
} s_parser;

typedef enum {
  TOKEN_Symbol = 128, TOKEN_Var,

  TOKEN_Literal_Int, TOKEN_Literal_Real, TOKEN_Literal_String,

  TOKEN_Char, TOKEN_Short, TOKEN_Int, TOKEN_Long, TOKEN_Float, TOKEN_Double, TOKEN_String, TOKEN_Bool, TOKEN_Unsigned,

  TOKEN_CommentBlock_End,

  TOKEN_Else, TOKEN_Enum, TOKEN_If, TOKEN_Return, TOKEN_Sizeof, TOKEN_While, TOKEN_For, TOKEN_Switch,
  TOKEN_Assign, TOKEN_Cond, TOKEN_Lor, TOKEN_Lan, TOKEN_Or, TOKEN_Xor, TOKEN_And, TOKEN_Eq, TOKEN_Ne, TOKEN_Lt, TOKEN_Gt, TOKEN_Le, TOKEN_Ge, TOKEN_Shl, TOKEN_Shr, TOKEN_Add, TOKEN_Sub, TOKEN_Mul, TOKEN_Div, TOKEN_Mod, TOKEN_Inc, TOKEN_Dec, TOKEN_Brak
} e_token;

typedef struct {
  e_token token;
} s_symbolbody_keyword;

s_parser *parse_Init(char *str);

bool parse_IsTokenBasicType(s_token token);

int parse_Next(s_scope *scope, s_parser *parser);

s_token parse_Preview(s_scope *scope, s_parser *parser);

int parse_Match(s_scope *scope, s_parser *parser, int token);

s_symbol *symbol_CreateFromKeyword(char *keyword, e_token token);


/* ##### STATEMENT ##### */
typedef enum {
  IF,
  FOR,
  WHILE,
  STATEMENT,
  RETURN,
  VARIABLE_DEF,
  FUNCTION_DEF,
  CLASS_DEF,
  EXPRESSION
} e_statementtype;

typedef struct {
  s_list *statements;
} s_statementbody_statement;

typedef struct _s_statement {
  struct _s_statement *parent;
  s_scope *scope;
  void *body;
  e_statementtype type;
  void (*exe_cb)(struct _s_statement *);
} s_statement;


/* ##### Compiler ##### */
typedef struct {
  s_scope *rootScope;
  s_parser *parser;
  s_statement *rootStatement;
} s_compiler;

s_compiler *compiler_Init(char *content);
void compiler_Next(s_compiler *compiler);


/* ##### STATEMENT ##### */
s_statement *statement_Create(s_compiler *compiler, s_statement *parent, e_statementtype type);
s_statement *statement_CreateAsRoot(s_compiler *compiler);

void compile_Statement(s_compiler *compiler, s_statement *statement);

/* ##### Expression ##### */
typedef struct {
  s_token *token;
} s_expression_operation;

typedef struct {
  s_list *core_operations;
} s_statementbody_expression;

s_expression_operation *expression_Emit(s_list *core_operations, s_token token);
void expression_Step(s_compiler *compiler, s_statement *statement, int level);
s_statement *compile_Expression(s_compiler *compiler, s_statement *parent);

/* ##### VARIABLE ##### */
typedef struct {
  s_anyvalue value;
  s_statement *init_expression;
} s_symbolbody_variable;

typedef struct {
  s_symbol *symbol;
} s_statementbody_variable;

s_anytype *compile_VariableType(s_compiler *compiler, s_statement *statement);
s_statement *compile_VariableDefinition(s_compiler *compiler, s_statement *parent);

/* ##### FUNCTION ##### */
typedef struct {
  s_symbol *symbol;
} s_function_argument;

typedef struct {
  s_anytype ret_type;
  s_list *arguments;
} s_symbolbody_function;

void compile_FunctionDefinition(s_compiler *compiler, s_symbol *name);

/* ##### CLASS ##### */
typedef struct {
  void *TODO;
} s_symbolbody_class;
void compile_ClassDefinition(s_compiler *compiler);


/* ##### CORE structs ##### */
typedef struct {
  s_anyvalue value;
  s_expression_operation *op;
} core_expression_item;

core_expression_item core_expression_item_Create(s_expression_operation *op, s_anyvalue value);

define_stack(core_expression_item)

typedef struct {
  s_anyvalue content[32];
} __core_function_arguments;

/* ##### CORE libs ##### */
s_anyvalue __core_exe_expression(s_statement *statement);
void __core_exe_assign(s_symbol *symbol, s_anyvalue item);

void __core_print(s_stack__core_expression_item *stack);

void __core_assign(s_stack__core_expression_item *stack);

void __core_add(s_stack__core_expression_item *stack);
void __core_sub(s_stack__core_expression_item *stack);
void __core_mul(s_stack__core_expression_item *stack);
void __core_div(s_stack__core_expression_item *stack);

void __core_expression(s_statement *statement);
void __core_variable_def(s_statement *statement);
void __core_call_function(s_statement *statement);

void __core_exe_statement(s_statement *statement);

void __core_new_class_instance();

#endif