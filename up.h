#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef UP
#define UP

/* ##### Colors heaven ##### */
#define COLOR_RED "31"
#define COLOR_GREEN "32"
#define COLOR_YELLOW "33"
#define COLOR_BLUE "34"
#define COLOR_MAGENTA "35"
#define COLOR_CYAN "36"

#define NORMAL(C, T) "\033[0m\033[0;" C "m" T "\033[0m"
#define BOLD(C, T) "\033[0m\033[0;" C "m\033[1m" T "\033[0m"

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
  struct _s_list_item *prev;
  struct _s_list_item *next;
  void *payload;
} s_list_item;

typedef struct {
  u_int64_t items_count;
  s_list_item *head_item;
  s_list_item *selected_item;
} s_list;

s_list *list_create();
//void list_destroy(s_list *l);

//void *list_read_index(s_list *l, u_int64_t index);
void *list_read_first(s_list *l);
void *list_read_last(s_list *l);
void *list_read_next(s_list *l);
void *list_read_previous(s_list *l);
void *list_read_selected(s_list *l);

//void list_add(s_list *l, void *value);
//void *list_pull(s_list *l);

void list_push(s_list *l, void *value);
void *list_pop(s_list *l);

/* ##### PARENTABLE LINKED LIST ##### */
typedef struct {
  s_list *lists;
  s_list *selected_list;
} s_parlist;

s_parlist *parlist_create(s_parlist *parent);
//void parlist_destroy(s_parlist *pl);

void *parlist_read_first(s_parlist *pl);
void *parlist_read_last(s_parlist *pl);
void *parlist_read_next(s_parlist *pl);
void *parlist_read_previous(s_parlist *pl);

//void parlist_add(s_parlist *pl, void *value);

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
void stack_clear__##T(s_stack__##T *s) { \
  s->ptr = 0; \
} \
void stack_push__##T(s_stack__##T *s, T value) { \
  s->content[s->ptr++] = value; \
} \
T stack_pop__##T(s_stack__##T *s) { \
  return s->content[--s->ptr]; \
} \

#define define_stack(T, N) \
typedef struct { \
  T *content; \
  u_int64_t ptr; \
  u_int64_t size; \
} s_stack__##N; \
s_stack__##N stack_create__##N(int size) { \
  s_stack__##N ret; \
  ret.size = size; \
  ret.ptr = 0; \
  ret.content = (T *)malloc(size * sizeof(T)); \
  return ret; \
} \
void stack_free__##N(s_stack__##N s) { \
  free(s.content); \
} \
void stack_clear__##N(s_stack__##N *s) { \
  s->ptr = 0; \
} \
void stack_push__##N(s_stack__##N *s, T value) { \
  s->content[s->ptr++] = value; \
} \
T stack_pop__##N(s_stack__##N *s) { \
  return s->content[--s->ptr]; \
} \

typedef enum {
  TOKEN_Symbol = 128,
  
  TOKEN_Debug_Info, TOKEN_Debug_Breakpoint,

  TOKEN_NULL, TOKEN_Literal_Int, TOKEN_Literal_Real, TOKEN_Literal_String,

  TOKEN_CommentBlock_End, TOKEN_DirectChildren,

  TOKEN_This, TOKEN_Return, TOKEN_Super, TOKEN_Root,

  TOKEN_If, TOKEN_Else, TOKEN_While, TOKEN_For, TOKEN_Switch,
  TOKEN_Assign, TOKEN_Cond, TOKEN_Lor, TOKEN_Lan, TOKEN_Or, TOKEN_Xor, TOKEN_And, TOKEN_Eq, TOKEN_Ne, TOKEN_Lt, TOKEN_Gt, TOKEN_Le, TOKEN_Ge, TOKEN_Shl, TOKEN_Shr, TOKEN_Add, TOKEN_Sub, TOKEN_Mul, TOKEN_Div, TOKEN_Mod, TOKEN_Inc, TOKEN_Dec, TOKEN_Brak,
  
  TOKEN_Dot, TOKEN_Link
} e_token;

typedef struct {
  char *method_name;
  e_token sub_token;
} s_token_operator;

s_token_operator token_operators[] = {
  { .method_name = "Assign", .sub_token = TOKEN_Assign },
  { .method_name = "Cond", .sub_token = TOKEN_Cond },
  { .method_name = "Lor", .sub_token = TOKEN_Lan },
  { .method_name = "Lan", .sub_token = TOKEN_Or },
  { .method_name = "Or", .sub_token = TOKEN_Xor },
  { .method_name = "Xor", .sub_token = TOKEN_And },
  { .method_name = "And", .sub_token = TOKEN_Eq },
  { .method_name = "Eq", .sub_token = TOKEN_Ne },
  { .method_name = "Ne", .sub_token = TOKEN_Lt },
  { .method_name = "Less", .sub_token = TOKEN_Shl },
  { .method_name = "Gt", .sub_token = TOKEN_Shl },
  { .method_name = "Le", .sub_token = TOKEN_Shl },
  { .method_name = "Ge", .sub_token = TOKEN_Shl },
  { .method_name = "Shl", .sub_token = TOKEN_Add },
  { .method_name = "Shr", .sub_token = TOKEN_Add },
  { .method_name = "Add", .sub_token = TOKEN_Mul },
  { .method_name = "Sub", .sub_token = TOKEN_Mul },
  { .method_name = "Mul", .sub_token = TOKEN_Inc },
  { .method_name = "Div", .sub_token = TOKEN_Inc },
  { .method_name = "Mod", .sub_token = TOKEN_Inc },
  { .method_name = "Inc", .sub_token = TOKEN_Inc },
  { .method_name = "Dec", .sub_token = TOKEN_Dec }
};

/* ##### Types ##### */
typedef struct _s_statement s_statement;
typedef struct _s_symbol s_symbol;
typedef struct _s_class_instance s_class_instance;
typedef struct _s_expression_operation s_expression_operation;

typedef struct _s_exe_scope s_exe_scope;

typedef struct _s_symbol s_anytype;

typedef struct {
  s_anytype *type;
  u_int64_t data_index;
} s_anyvalue_field;

typedef struct {
  s_anytype *type;
  s_class_instance *data;
} s_anyvalue_argument;

typedef struct {
  s_anytype *type;
  s_list *instances;
} s_anyvalue_local;

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
  STATEMENT_ARGUMENT_DEF,
  STATEMENT_LOCAL_DEF,
  STATEMENT_METHOD_DEF,
  STATEMENT_CLASS_DEF,
  STATEMENT_CONSTRUCTOR_DEF,
  STATEMENT_EXPRESSION,
  STATEMENT_DEBUG_INFO,
  STATEMENT_DEBUG_BREAKPOINT
} e_statementtype;

typedef enum {
  STATEMENT_END_CONTINUE,
  STATEMENT_END_BREAK,
  STATEMENT_END_RETURN,
  STATEMENT_END_EXIT
} e_statementend;

typedef struct {
  s_list *statements; // <s_statement>
} s_statementbody_block;

typedef struct {
  s_statement *check;
  s_statement *_true;
  s_statement *_false;
} s_statementbody_if;

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
  s_statement *value;
} s_statementbody_return;

typedef struct {
  s_symbol *symbol;
} s_statementbody_field_def;

typedef struct {
  s_symbol *symbol;
  s_method_def *method;
} s_statementbody_method_def;

typedef struct {
  s_symbol *symbol;
} s_statementbody_argument_def;

typedef struct {
  s_symbol *symbol;
} s_statementbody_local_def;

typedef struct {
  s_symbol *symbol;
} s_statementbody_class_def;

typedef struct {
  s_method_def *method;
} s_statementbody_constructor_def;

typedef struct {
  s_list *operations; // <s_expression_operation>
} s_statementbody_expression;

typedef struct {
  s_symbol *symbol;
  char *comment;
} s_statementbody_debug_def;

typedef union {
  s_statementbody_block *block;
  s_statementbody_if *_if;
  s_statementbody_for *_for;
  s_statementbody_while *_while;
  s_statementbody_return *_return;
  s_statementbody_field_def *field_def;
  s_statementbody_argument_def *argument_def;
  s_statementbody_argument_def *local_def;
  s_statementbody_method_def *method_def;
  s_statementbody_class_def *class_def;
  s_statementbody_constructor_def *constructor_def;
  s_statementbody_expression *expression;
  s_statementbody_debug_def *debug;
} u_statementbody;

struct _s_statement {
  struct _s_statement *parent;
  s_scope *scope;
  u_statementbody body;
  e_statementtype type;
  e_statementend (*exe_cb)(s_exe_scope);
  s_list *temporaries; // <s_symbol>
};

/* ##### Symbols ##### */
typedef enum {
  SYMBOL_NOTDEFINED,
  SYMBOL_KEYWORD,
  SYMBOL_FIELD,
  SYMBOL_METHOD,
  SYMBOL_ARGUMENT,
  SYMBOL_LOCAL,
  SYMBOL_CLASS
} e_symboltype;

const char *debug_symboltype[] = {
  "NOTDEFINED",
  "KEYWORD",
  "FIELD",
  "METHOD",
  "ARGUMENT",
  "LOCAL",
  "CLASS"
};

typedef struct {
  e_token token;
} s_symbolbody_keyword;

typedef struct {
  s_anyvalue_field value;
  s_statement *init_expression;
} s_symbolbody_field;

typedef struct {
  s_anyvalue_argument value;
  s_statement *init_expression;
} s_symbolbody_argument;

typedef struct {
  s_anyvalue_local value;
  s_statement *init_expression;
} s_symbolbody_local;

typedef struct {
  s_list *overloads; // <s_method_def>
} s_symbolbody_method;

typedef struct {
  s_scope *scope;
  s_list *parents; // <s_symbol<class>>
  s_list *constructors; // <s_method_def>
  s_list *fields; // <s_symbol<field>>
  s_list *methods; // <s_symbol<method>>
} s_symbolbody_class;

typedef union {
  s_symbolbody_keyword *keyword;
  s_symbolbody_field *field;
  s_symbolbody_method *method;
  s_symbolbody_argument *argument;
  s_symbolbody_local *local;
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

bool symbol_Equal(s_symbol *a, s_symbol *b);

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
s_statement *statement_CreateChildren(s_statement *parent, e_statementtype type, s_scope *forcedScope);
s_statement *statement_CreateBlock(s_statement *parent);

s_statement *compile_DefinitionStatement(s_compiler *compiler, s_statement *parent);
s_statement *compile_Statement(s_compiler *compiler, s_statement *parent);

/* ##### For STATEMENT ##### */


/* ##### While STATEMENT ##### */


/* ##### Expression ##### */
typedef enum {
  OP_NULL = 0x00,
  OP_LoadSymbol,
  OP_ConstructorCall,
  OP_MethodCall,
  OP_UseTemporaryInstance,
  OP_LoadThis,
  OP_LoadReturn,
  OP_Link,
  OP_AccessSymbol
} e_expression_operation_type;

typedef union {
  s_symbol *symbol;
  s_method_def *method;
  s_class_instance *temporary;
} u_expression_operation_payload;

struct _s_expression_operation {
  e_expression_operation_type type;
  u_expression_operation_payload payload;
};

s_expression_operation *expression_Emit(s_list *core_operations, e_expression_operation_type type);

s_expression_operation *expression_Step(s_compiler *compiler, s_statement *statement, s_scope *scope, s_list *operations, int level);
s_statement *compile_Expression(s_compiler *compiler, s_statement *parent);



/* ##### FIELD ##### */
s_anytype *compile_FieldType(s_compiler *compiler, s_statement *statement);
s_statement *compile_ArgumentDefinition(s_compiler *compiler, s_statement *parent);
s_statement *compile_FieldDefinition(s_compiler *compiler, s_statement *parent);
s_statement *compile_LocalFieldDefinition(s_compiler *compiler, s_statement *parent);

/* ##### METHOD ##### */
typedef enum {
  METHODBODY_STATEMENT,
  METHODBODY_CALLBACK
} e_methodbody_type;

typedef union {
  s_statement *statement;
  void (*callback)(s_class_instance *ret, s_class_instance *self, s_class_instance **args);
} u_methodbody_content;

typedef struct {
  e_methodbody_type type;
  u_methodbody_content content;
} s_methodbody;

struct _s_method_def {
  int hash;
  s_anytype *ret_type;
  s_list *arguments; // <s_symbol>
  s_methodbody body;
};

int method_ComputeHash(s_method_def *method);

s_statement *compile_MethodDefinition(s_compiler *compiler, s_statement *parent);
s_statement *compile_ConstructorMethodDefinition(s_compiler *compiler, s_statement *parent);

s_method_def *method_FindOverload(s_symbol *method, s_list *args);

/* ##### CLASS ##### */
struct _s_class_instance {
  s_symbol *class;
  s_class_instance **data;
};

struct _s_exe_scope {
  s_class_instance *ret;
  s_class_instance *self;
  s_statement *statement;
};

#define EXE_SCOPE(RET,SELF,STATEMENT) (s_exe_scope) { .ret = (RET), .self = (SELF), .statement = (STATEMENT) }
#define SUB_EXE_SCOPE(EXE,STATEMENT) (s_exe_scope) { .ret = (EXE.ret), .self = (EXE.self), .statement = (STATEMENT) }

s_symbol *class_Create(char *name, s_scope *scope);

s_method_def *class_CreateConstructor(s_symbol *class, void (*cb)(s_class_instance *ret, s_class_instance *self, s_class_instance **args), int nArguments, ...);

s_method_def *class_CreateMethod(s_symbol *class, char *name, void (*cb)(s_class_instance *ret, s_class_instance *self, s_class_instance **args), char *returnType, int nArguments, ...);

s_method_def *class_FindMethodByName(s_symbol *class, char *name, s_list *args);

s_class_instance *class_CreateInstance(s_symbol *class);

void *class_DeriveFrom(s_statement *dest, s_symbol *src);

void compile_ClassBody(s_compiler *compiler, s_statement *class);
s_statement *compile_ClassDefinition(s_compiler *compiler, s_statement *parent);

s_statement *compile_Debug(s_compiler *compiler, s_statement *parent);
s_statement *compile_Breakpoint(s_compiler *compiler, s_statement *parent);

/* ##### CORE libs ##### */
define_stack(s_class_instance *, s_class_instance_ptr);
s_stack__s_class_instance_ptr stack;

void __exe_initializeFieldInInstance(s_class_instance *instance, s_symbol *field_symbol);
void __exe_method(s_class_instance *self, s_method_def *method, s_class_instance *return_instance, s_class_instance **args);

s_class_instance *__core_exe_expression(s_exe_scope exe);

e_statementend __core_argument_def(s_exe_scope exe);
e_statementend __core_field_def(s_exe_scope exe);
e_statementend __core_local_def(s_exe_scope exe);

e_statementend __core_expression(s_exe_scope exe);

e_statementend __core_exe_statement(s_exe_scope exe);

e_statementend __core_if(s_exe_scope exe);
e_statementend __core_for(s_exe_scope exe);
e_statementend __core_while(s_exe_scope exe);

e_statementend __core_debug(s_exe_scope exe);
e_statementend __core_breakpoint(s_exe_scope exe);

/* ##### Generic object LIB ##### */
void object_Assign(s_class_instance *ret, s_class_instance *self, s_class_instance **args);
void object_Print(s_class_instance *ret, s_class_instance *self, s_class_instance **args);
void object_ToString(s_class_instance *ret, s_class_instance *self, s_class_instance **args);

// SDK
s_class_instance *sdk_class_ExecuteMethod(s_class_instance *target, s_method_def *method);

typedef struct {
  bool isDecimal;
  union {
    u_int64_t integer;
    double decimal;
  } content;
} s_number;

typedef struct {
  u_int32_t len;
  char *content;
} s_string;

void string_resize(s_string *str, u_int64_t size);

// Required lib classes
s_symbol *LIB_NumberClass = NULL;
s_symbol *LIB_StringClass = NULL;

#endif