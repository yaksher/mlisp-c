#ifndef __AST_H__
#define __AST_H__
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef size_t usize;
typedef ssize_t isize;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef u16 Ident;

typedef union Expr Expr;
typedef union Decl Decl;

typedef struct ExprList {
    usize len;
    Expr *data;
} ExprList;

typedef struct DeclList {
    usize len;
    Decl *data;
} DeclList;

typedef DeclList Ast;

typedef struct IdentList {
    usize len;
    Ident *data;
} IdentList;

typedef struct String {
    usize len;
    char *data;
} String;

typedef enum {
    B_TYPE_OF,
    B_REPR,
    B_NOT,
    B_EQ,
    B_NEQ,
    B_LT,
    B_GT,
    B_LE,
    B_GE,
    B_ADD,
    B_SUB,
    B_MUL,
    B_DIV,
    B_LEN,
    B_PUSH,
    B_POP,
    B_GET,
    B_SET,
    B_INT,
    B_SPLIT,
    B_TRIM,
    B_JOIN,
    B_OPEN,
    B_READ,
    B_WRITE,
    B_CLOSE
} Builtin;

typedef enum {
    L_NONE,
    L_INT,
    L_STRING,
} LiteralKind;

typedef union {
    LiteralKind kind;
    struct { LiteralKind kind; } none;
    struct { LiteralKind kind; i64 value; } num;
    struct { LiteralKind kind; String value; } str;
} Literal;

typedef enum {
    D_FN,
    D_BIND,
} DeclKind;

typedef enum {
    FARGS_ONE,
    FARGS_MANY,
} FnArgsKind;

typedef union {
    FnArgsKind kind;
    struct { FnArgsKind kind; Ident id; } one;
    struct { FnArgsKind kind; IdentList ids; } many;
} FnArgs;

typedef enum {
    E_FOR,
    E_BIND,
    E_ASSIGN,
    E_IF,
    E_CALL,
    E_AND,
    E_OR,
    E_SEQUENCE,
    E_LIST,
    E_LITERAL,
    E_BUILTIN,
    E_IDENT,
} ExprKind;

typedef struct {
    ExprKind kind;
    Ident id;
    Expr *iter;
    Expr *body;
} ForExpr;

typedef struct {
    ExprKind kind;
    Ident id;
    Expr *value;
    Expr *body;
} BindExpr;

typedef struct {
    ExprKind kind;
    Ident id;
    Expr *value;
} AssignExpr;

typedef struct {
    ExprKind kind;
    Expr *cond;
    Expr *then;
    Expr *els;
} IfExpr;

typedef struct {
    ExprKind kind;
    Expr *fn;
    ExprList args;
} CallExpr;

typedef struct {
    ExprKind kind;
    ExprList exprs;
} AndExpr;

typedef struct {
    ExprKind kind;
    ExprList exprs;
} OrExpr;

typedef struct {
    ExprKind kind;
    ExprList exprs;
} SequenceExpr;

typedef struct {
    ExprKind kind;
    ExprList exprs;
} ListExpr;

typedef struct {
    ExprKind kind;
    Literal literal;
} LiteralExpr;

typedef struct {
    ExprKind kind;
    Builtin builtin;
} BuiltinExpr;

typedef struct {
    ExprKind kind;
    Ident id;
} IdentExpr;

union Expr {
    ExprKind kind;
    ForExpr loop;
    BindExpr bind;
    AssignExpr assign;
    IfExpr cond;
    CallExpr call;
    AndExpr and;
    OrExpr or;
    SequenceExpr sequence;
    ListExpr list;
    LiteralExpr literal;
    BuiltinExpr builtin;
    IdentExpr ident;
};

union Decl {
    DeclKind kind;
    struct { DeclKind kind; Ident id; FnArgs args; Expr body; } fn;
    struct { DeclKind kind; Ident id; Expr value; } bind;
};

Ast ast_deserialize(FILE *stream);

void ast_print(Ast ast);

#endif // __AST_H__