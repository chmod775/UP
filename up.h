#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef UP
#define UP

/* ##### Helpers shit ##### */
FILE *openFile(char *filename);

void closeFile(FILE *fd);

int hashOfSymbol(char *str);

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
void list_destroy(s_list *l);

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
void parlist_destroy(s_parlist *pl);

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

typedef enum {
  TOKEN_Symbol = 128, TOKEN_Debug,

  TOKEN_NULL, TOKEN_Literal_Int, TOKEN_Literal_Real, TOKEN_Literal_String,

  TOKEN_CommentBlock_End,

  TOKEN_If, TOKEN_Else, TOKEN_Return, TOKEN_While, TOKEN_For, TOKEN_Switch,
  TOKEN_Assign, TOKEN_Cond, TOKEN_Lor, TOKEN_Lan, TOKEN_Or, TOKEN_Xor, TOKEN_And, TOKEN_Eq, TOKEN_Ne, TOKEN_Lt, TOKEN_Gt, TOKEN_Le, TOKEN_Ge, TOKEN_Shl, TOKEN_Shr, TOKEN_Add, TOKEN_Sub, TOKEN_Mul, TOKEN_Div, TOKEN_Mod, TOKEN_Inc, TOKEN_Dec, TOKEN_Brak
} e_token;

/* ##### Types ##### */
typedef struct _s_anytype s_anytype;
typedef struct _s_statement s_statement;
typedef struct _s_symbol s_symbol;
typedef struct _s_class_instance s_class_instance;

typedef struct {
  s_anytype *key;
  s_anytype *value;
} s_dictionarytype;

typedef struct {
  s_anytype *items;
} s_listtype;

typedef union {
  s_symbol *class;
  s_dictionarytype *dictionary;
  s_listtype *list;
} u_anytype_content;

typedef enum {
  TYPE_ANY,
  TYPE_CLASS,
  TYPE_LIST,
  TYPE_DICTIONARY
} e_anytype_category;

struct _s_anytype {
  e_anytype_category category;
  u_anytype_content content;
};

typedef struct {
  s_anytype type;
  u_int64_t data_index;
} s_anyvalue;

s_anyvalue s_anyvalue_createPrimary(int type, void *value);


/* ##### Scope ##### */
typedef struct _s_scope {
  struct _s_scope *parent;
  s_parlist *symbols;
} s_scope;

/* ##### Method ##### */
typedef struct _s_method_def s_method_def;

/* ##### STATEMENT ##### */
typedef enum {
  STATEMENT_BLOCK,
  STATEMENT_IF,
  STATEMENT_FOR,
  STATEMENT_WHILE,
  STATEMENT_RETURN,
  STATEMENT_FIELD_DEF,
  STATEMENT_METHOD_DEF,
  STATEMENT_CLASS_DEF,
  STATEMENT_CONSTRUCTOR_DEF,
  STATEMENT_EXPRESSION
} e_statementtype;

typedef struct {
  s_list *statements; // <s_statement>
} s_statementbody_block;

typedef struct {
  s_statement *init;
  s_statement *check;
  s_statement *step;
} s_statementbody_for;

typedef struct {
  s_statement *check;
  s_statement *loop;
} s_statementbody_while;

typedef struct {
  s_symbol *symbol;
} s_statementbody_field_def;

typedef struct {
  s_symbol *symbol;
} s_statementbody_method_def;

typedef struct {
  s_symbol *symbol;
} s_statementbody_class_def;

typedef struct {
  s_method_def *method;
} s_statementbody_constructor_def;

typedef struct {
  s_list *core_operations; // <s_expression_operation>
} s_statementbody_expression;

typedef union {
  s_statementbody_block *block;
  s_statementbody_for *_for;
  s_statementbody_while *_while;
  s_statementbody_field_def *field_def;
  s_statementbody_method_def *method_def;
  s_statementbody_class_def *class_def;
  s_statementbody_constructor_def *constructor_def;
  s_statementbody_expression *expression;
} u_statementbody;

struct _s_statement {
  struct _s_statement *parent;
  s_scope *scope;
  u_statementbody body;
  e_statementtype type;
  void (*exe_cb)(struct _s_statement *);
};

/* ##### Symbols ##### */
typedef enum {
  SYMBOL_NOTDEFINED,
  SYMBOL_KEYWORD,
  SYMBOL_FIELD,
  SYMBOL_METHOD,
  SYMBOL_CLASS
} e_symboltype;

typedef struct {
  e_token token;
} s_symbolbody_keyword;

typedef struct {
  s_anyvalue value;
  s_statement *init_expression;
} s_symbolbody_field;

typedef struct {
  s_list *overloads; // <s_method_def>
} s_symbolbody_method;

typedef struct {
  s_scope *scope;
  s_symbol *parent;
  s_list *constructors; // <s_method_def>
  s_list *fields; // <s_symbol<field>>
  s_list *methods; // <s_symbol<method>>
} s_symbolbody_class;

typedef union {
  s_symbolbody_keyword *keyword;
  s_symbolbody_field *field;
  s_symbolbody_method *method;
  s_symbolbody_class *class;
} u_symbolbody;

typedef struct _s_symbol {
  int hash;
  char *name;
  int length;

  bool isUppercase;
  bool isFullcase;
  bool startsUnderscore;

  e_symboltype type;
  u_symbolbody body;
} s_symbol;

char *symbol_GetCleanName(s_symbol *symbol);

s_symbol *symbol_Create(char *name, e_symboltype type, int length);
s_symbol *symbol_CreateEmpty(e_symboltype type);
s_symbol *symbol_CreateFromKeyword(char *keyword, e_token token);

s_symbol *symbol_Find(char *name, s_parlist *symbols);

/* ##### Scope ##### */
s_scope *scope_Create(s_scope *parent);
s_scope *scope_CreateAsRoot();

void scope_AddSymbol(s_scope *scope, s_symbol *symbol);

void scope_Destroy(s_scope *scope);

char *scope_Print(s_scope *scope);

/* ##### Parser ##### */
typedef union {
  int64_t integer;
  double decimal;
  char *string;
  s_symbol *symbol;
  void *any;
} u_token_content;

typedef struct {
  int type;
  u_token_content content;
} s_token;

typedef struct {
  char *source;
  char *ptr;
  int line;
  s_token token;
} s_parser;

s_parser *parse_Init(char *str);

int parse_Next(s_scope *scope, s_parser *parser);

s_token parse_Preview(s_scope *scope, s_parser *parser);

int parse_Match(s_scope *scope, s_parser *parser, int token);


/* ##### Compiler ##### */
typedef struct {
  s_scope *rootScope;
  s_parser *parser;
  s_statement *rootStatement;
} s_compiler;

s_compiler *compiler_Init(char *content);
void compiler_Next(s_compiler *compiler);


/* ##### STATEMENT ##### */
s_statement *statement_Create(s_statement *parent, s_scope *scope, e_statementtype type);

s_statement *statement_CreateInside(s_statement *parent, e_statementtype type);
s_statement *statement_CreateChildren(s_statement *parent, e_statementtype type);
s_statement *statement_CreateBlock(s_statement *parent);

s_statement *compile_DefinitionStatement(s_compiler *compiler, s_statement *parent);
s_statement *compile_Statement(s_compiler *compiler, s_statement *parent);

/* ##### For STATEMENT ##### */


/* ##### While STATEMENT ##### */


/* ##### Expression ##### */
typedef enum {
  OP_NULL = 0x00,

  OP_Literal_Int = 0x10,
  OP_Literal_Real,
  OP_Literal_String,

  OP_AccessField = 0x20,

  OP_MethodCall = 0x30,

  OP_ConstructorCall = 0x40
} e_expression_operation_type;

typedef struct {
  s_symbol *target;
  s_method_def *method;
  s_list *arguments; // <s_symbol>
} s_expressionpayload_call;

typedef union {
  int64_t integer;
  double decimal;
  char *string;
  s_symbol *field;
  s_expressionpayload_call *call;
} u_expression_operation_payload;

typedef struct {
  e_expression_operation_type type;
  u_expression_operation_payload payload;
} s_expression_operation;

s_expression_operation *expression_Emit(s_list *core_operations, e_expression_operation_type type);

void expression_Step(s_compiler *compiler, s_statement *statement, int level);
s_statement *compile_Expression(s_compiler *compiler, s_statement *parent);

/* ##### FIELD ##### */
s_anytype *compile_FieldType(s_compiler *compiler, s_statement *statement);
s_statement *compile_ArgumentDefinition(s_compiler *compiler, s_statement *parent);
s_statement *compile_FieldDefinition(s_compiler *compiler, s_statement *parent);

/* ##### METHOD ##### */
typedef enum {
  METHODBODY_STATEMENT,
  METHODBODY_CALLBACK
} e_methodbody_type;

typedef union {
  s_statement *statement;
  void (*callback)(s_class_instance *self, s_list *args);
} u_methodbody_content;

typedef struct {
  e_methodbody_type type;
  u_methodbody_content content;
} s_methodbody;

struct _s_method_def {
  int hash;
  s_anytype ret_type;
  s_list *arguments; // <s_symbol>
  s_methodbody body;
};

int method_ComputeHash(s_method_def *method);

s_statement *compile_MethodDefinition(s_compiler *compiler, s_statement *parent);
s_statement *compile_ConstructorMethodDefinition(s_compiler *compiler, s_statement *parent);

/* ##### CLASS ##### */
struct _s_class_instance {
  s_symbol *class;
  void *data;
};

s_symbol *class_Create(char *name, s_scope *scope);

s_method_def *class_CreateConstructor(s_symbol *class, void (*cb)(s_class_instance *self, s_list *args), int nArguments, ...);

s_method_def *class_CreateMethod(s_symbol *class, char *name, void (*cb)(s_class_instance *self, s_list *args), char *returnType, int nArguments, ...);

s_method_def *class_FindMethod(s_symbol *class, char *name, s_list *args);

s_class_instance *class_CreateInstance(s_symbol *class);

void compile_ClassBody(s_compiler *compiler, s_statement *class);
s_statement *compile_ClassDefinition(s_compiler *compiler, s_statement *parent);

/* ##### CORE structs ##### */
typedef struct {
  s_anyvalue value;
  s_expression_operation *op;
} core_expression_item;

core_expression_item core_expression_item_Create(s_expression_operation *op, s_anyvalue value);

define_stack(core_expression_item)

typedef struct {
  s_anyvalue content[32];
} __core_method_arguments;

/* ##### CORE libs ##### */
s_anyvalue __core_exe_expression(s_statement *statement);

void __core_class_createInstance(s_statement *statement);
void __core_class_executeConstructor(s_statement *statement);
void __core_method_execute(s_statement *statement);

void __core_field_def(s_statement *statement);

void __core_expression(s_statement *statement);

void __core_exe_statement(s_statement *statement);

void __core_if(s_statement *statement);
void __core_for(s_statement *statement);
void __core_while(s_statement *statement);

// Symbols actions
void __core_symbol_assign(s_symbol *symbol, s_anyvalue value);
s_anyvalue __core_symbol_get_value(s_symbol *symbol);

void __core_symbol_call(s_symbol *symbol, s_list *arguments);



// Class actions
s_symbol *__core_class_get_method(s_symbol *class, char *methodName);
s_symbol *__core_class_get_field(s_symbol *class, char *fieldName);

#endif