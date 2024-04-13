#include <stdlib.h>
#include "ast.h"

#define BOX(V) ({ \
    typeof(V) *box = malloc(sizeof(V)); \
    *box = V; \
    box; \
})

#define DESERIALIZE(T, read) T ## _deserialize(read)

#define DESERIALIZE_SIG(T) static T T ## _deserialize(FILE *read)

#define DES(T) DESERIALIZE(T, read)
#define DES_SIG DESERIALIZE_SIG

#define DECL_INT_DESERIALIZE(INT_T) \
static INT_T INT_T ## _deserialize(FILE *read) { \
    INT_T value; \
    fread(&value, sizeof(INT_T), 1, read); \
    return value; \
}

#define LOC (ftell(read))

#define TRAP() (*(volatile char *) NULL)

DES_SIG(ExprList);
DES_SIG(DeclList);
DES_SIG(String);
DES_SIG(Literal);
DES_SIG(Builtin);
DES_SIG(Expr);
DES_SIG(FnArgs);
DES_SIG(Decl);

DECL_INT_DESERIALIZE(i64)
DECL_INT_DESERIALIZE(u8)
DECL_INT_DESERIALIZE(u16)
// DECL_INT_DESERIALIZE(u64)
// DECL_INT_DESERIALIZE(usize)
DECL_INT_DESERIALIZE(Ident)

DES_SIG(String) {
    u16 len = DES(u16);
    String str = {
        .len = len,
        .data = malloc(len + 1),
    };
    fread(str.data, 1, len, read);
    str.data[len] = 0;
    return str;
}

#define DECL_LIST_DES(T) \
static T## List T ##List_deserialize(FILE *read) { \
    u16 len = DES(u16); \
    T##List l = { \
        .len = len, \
        .data = malloc(sizeof(T) * len), \
    }; \
    for (u16 i = 0; i < len; i++) {\
        l.data[i] = DES(T); \
    } \
    return l; \
} \

DECL_LIST_DES(Expr)
DECL_LIST_DES(Decl)

DES_SIG(Literal) {
    Literal lit = {
        .kind = DES(u8),
    };
    switch (lit.kind) {
        case L_NONE:
            break;
        case L_INT:
            lit.num.value = DES(i64);
            break;
        case L_STRING:
            lit.str.value = DES(String);
            break;
        default:
            TRAP();
    }
    return lit;
}

DES_SIG(Builtin) {
    u8 tag = DES(u8);
    return tag;
}

DES_SIG(Expr) {
    Expr expr = {
        .kind = DES(u8),
    };
    switch (expr.kind) {
        case E_FOR: {
            ForExpr e = {
                .kind = E_FOR,
                .id = DES(Ident),
                .iter = BOX(DES(Expr)),
                .body = BOX(DES(Expr)),
            };
            expr.loop = e;
            break;
        }
        case E_BIND: {
            BindExpr e = {
                .kind = E_BIND,
                .id = DES(Ident),
                .value = BOX(DES(Expr)),
                .body = BOX(DES(Expr)),
            };
            expr.bind = e;
            break;
        }
        case E_ASSIGN: {
            AssignExpr e = {
                .kind = E_ASSIGN,
                .id = DES(Ident),
                .value = BOX(DES(Expr)),
            };
            expr.assign = e;
            break;
        }
        case E_IF: {
            IfExpr e = {
                .kind = E_IF,
                .cond = BOX(DES(Expr)),
                .then = BOX(DES(Expr)),
                .els = BOX(DES(Expr)),
            };
            expr.cond = e;
            break;
        }
        case E_CALL: {
            CallExpr e = {
                .kind = E_CALL,
                .fn = BOX(DES(Expr)),
                .args = DES(ExprList),
            };
            expr.call = e;
            break;
        }
        case E_AND: {
            AndExpr e = {
                .kind = E_AND,
                .exprs = DES(ExprList),
            };
            expr.and = e;
            break;
        }
        case E_OR: {
            OrExpr e = {
                .kind = E_OR,
                .exprs = DES(ExprList),
            };
            expr.or = e;
            break;
        }
        case E_SEQUENCE: {
            SequenceExpr e = {
                .kind = E_SEQUENCE,
                .exprs = DES(ExprList),
            };
            expr.sequence = e;
            break;
        }
        case E_LIST: {
            expr.list = (ListExpr) {
                .kind = E_LIST,
                .exprs = DES(ExprList),
            };
            break;
        }
        case E_LITERAL: {
            expr.literal = (LiteralExpr) {
                .kind = E_LITERAL,
                .literal = DES(Literal),
            };
            break;
        }
        case E_BUILTIN: {
            expr.builtin = (BuiltinExpr) {
                .kind = E_BUILTIN,
                .builtin = DES(Builtin),
            };
            break;
        }
        case E_IDENT: {
            expr.ident = (IdentExpr) {
                .kind = E_IDENT,
                .id = DES(Ident),
            };
            break;
        }
        default:
            TRAP();
    }
    return expr;
}

DES_SIG(FnArgs) {
    u8 tag = DES(u8);
    FnArgs args = {
        .kind = tag == 0 ? FARGS_ONE : FARGS_LIST,
    };
    switch (args.kind) {
        case FARGS_ONE:
            args.one.id = DES(Ident);
            break;
        case FARGS_LIST: {
            u8 len = tag - 1;
            args.list.ids = (IdentList){
                .len = len,
                .data = malloc(sizeof(Ident) * len),
            };
            for (u8 i = 0; i < len; i++) {
                args.list.ids.data[i] = DES(Ident);
            }
            break;
        }
        default:
            TRAP();
    }
    return args;
}

DES_SIG(Decl) {
    Decl decl = {
        .kind = DES(u8),
    };
    switch (decl.kind) {
        case D_FN:
            decl.fn.id = DES(Ident);
            decl.fn.args = DES(FnArgs);
            decl.fn.body = DES(Expr);
            break;
        case D_BIND:
            decl.bind.id = DES(Ident);
            decl.bind.value = DES(Expr);
            break;
        default:
            TRAP();
    }
    return decl;
}

Ast ast_deserialize(FILE *read) {
    return DESERIALIZE(DeclList, read);
}

#define PRINT(T, v, depth) T ## _print(v, depth)

#define PRINT_SIG(T) static void T ## _print(T v, int depth)

#define P(T, v) PRINT(T, v, depth + 1)

#define i64FMT "%lld"
#define u8FMT "%u"
#define u16FMT "%u"
#define u64FMT "%llu"
#define usizeFMT "%lu"
#define IdentFMT u16FMT

#define DECL_INT_PRINT(INT_T) \
PRINT_SIG(INT_T) { \
    printf("%*s"INT_T ## FMT, depth * 4, "", v); \
}

PRINT_SIG(ExprList);
// PRINT_SIG(DeclList);
PRINT_SIG(Literal);
PRINT_SIG(Builtin);
PRINT_SIG(Expr);
PRINT_SIG(FnArgs);
PRINT_SIG(Decl);

DECL_INT_PRINT(Ident)

#define DECL_LIST_PRINT(T) \
PRINT_SIG(T##List) { \
    printf("%*s[\n", depth * 4, ""); \
    for (u16 i = 0; i < v.len; i++) {\
        P(T, v.data[i]); \
        printf(",\n"); \
    } \
    printf("%*s]", depth * 4, ""); \
} \

DECL_LIST_PRINT(Expr)
// DECL_LIST_PRINT(Decl)
DECL_LIST_PRINT(Ident)

PRINT_SIG(Literal) {
    switch (v.kind) {
        case L_NONE:
            printf("%*sNone", depth * 4, "");
            break;
        case L_INT:
            printf("%*sLit("i64FMT")", depth * 4, "", v.num.value);
            break;
        case L_STRING:
            printf("%*sLit(\"%s\")", depth * 4, "", v.str.value.data);
            break;
        default:
            TRAP();
    }
}

#define BUILTIN_CASE(V) case V: printf("%*s", depth * 4, &#V[2]); break;

PRINT_SIG(Builtin) {
    switch (v) {
        BUILTIN_CASE(B_TYPE_OF)
        BUILTIN_CASE(B_REPR)
        BUILTIN_CASE(B_NOT)
        BUILTIN_CASE(B_EQ)
        BUILTIN_CASE(B_NEQ)
        BUILTIN_CASE(B_LT)
        BUILTIN_CASE(B_GT)
        BUILTIN_CASE(B_LE)
        BUILTIN_CASE(B_GE)
        BUILTIN_CASE(B_ADD)
        BUILTIN_CASE(B_SUB)
        BUILTIN_CASE(B_MUL)
        BUILTIN_CASE(B_DIV)
        BUILTIN_CASE(B_LEN)
        BUILTIN_CASE(B_PUSH)
        BUILTIN_CASE(B_POP)
        BUILTIN_CASE(B_GET)
        BUILTIN_CASE(B_SET)
        BUILTIN_CASE(B_INT)
        BUILTIN_CASE(B_SPLIT)
        BUILTIN_CASE(B_TRIM)
        BUILTIN_CASE(B_JOIN)
        BUILTIN_CASE(B_OPEN)
        BUILTIN_CASE(B_READ)
        BUILTIN_CASE(B_WRITE)
        BUILTIN_CASE(B_CLOSE)
        BUILTIN_CASE(B_PRINT)
        default:
            TRAP();
    }
}
#define EXPR_CASE(V) \
printf("%*s)", depth * 4, "");\
break; \
case V: \
printf("%*s%s (\n", depth * 4, "", &#V[2]); \

PRINT_SIG(Expr) {
    switch (v.kind) {
        EXPR_CASE(E_FOR)
            P(Ident, v.loop.id);
            printf(",\n");
            P(Expr, *v.loop.iter);
            printf(",\n");
            P(Expr, *v.loop.body);
            printf("\n");
        EXPR_CASE(E_BIND)
            P(Ident, v.bind.id);
            printf(",\n");
            P(Expr, *v.bind.value);
            printf(",\n");
            P(Expr, *v.bind.body);
            printf("\n");
        EXPR_CASE(E_ASSIGN)
            P(Ident, v.assign.id);
            printf(",\n");
            P(Expr, *v.assign.value);
            printf("\n");
        EXPR_CASE(E_IF)
            P(Expr, *v.cond.cond);
            printf(",\n");
            P(Expr, *v.cond.then);
            printf(",\n");
            P(Expr, *v.cond.els);
            printf("\n");
        EXPR_CASE(E_CALL)
            P(Expr, *v.call.fn);
            printf(",\n");
            P(ExprList, v.call.args);
            printf("\n");
        EXPR_CASE(E_AND)
            P(ExprList, v.and.exprs);
            printf("\n");
        EXPR_CASE(E_OR)
            P(ExprList, v.or.exprs);
            printf("\n");
        EXPR_CASE(E_SEQUENCE)
            P(ExprList, v.sequence.exprs);
            printf("\n");
        EXPR_CASE(E_LIST)
            P(ExprList, v.list.exprs);
            printf("\n");
        EXPR_CASE(E_LITERAL)
            P(Literal, v.literal.literal);
            printf("\n");
        EXPR_CASE(E_BUILTIN)
            P(Builtin, v.builtin.builtin);
            printf("\n");
        EXPR_CASE(E_IDENT)
            P(Ident, v.ident.id);
            printf("\n%*s)", depth * 4, "");
            break;
        default:
            TRAP();
    }
}

PRINT_SIG(FnArgs) {
    switch (v.kind) {
        case FARGS_ONE:
            printf("%*sArg (\n", depth * 4, "");
            P(Ident, v.one.id);
            printf("\n%*s)", depth * 4, "");
            break;
        case FARGS_LIST:
            printf("%*sList (\n", depth * 4, "");
            P(IdentList, v.list.ids);
            printf("\n%*s)", depth * 4, "");
            break;
        default:
            TRAP();
    }
}

PRINT_SIG(Decl) {
    switch (v.kind) {
        case D_FN:
            printf("%*sFn (\n", depth * 4, "");
            P(Ident, v.fn.id);
            printf(",\n");
            P(FnArgs, v.fn.args);
            printf(",\n");
            P(Expr, v.fn.body);
            printf("\n%*s)", depth * 4, "");
            break;
        case D_BIND:
            printf("%*sBind (\n", depth * 4, "");
            P(Ident, v.bind.id);
            printf(",\n");
            P(Expr, v.bind.value);
            printf("\n%*s)", depth * 4, "");
            break;
        default:
            TRAP();
    }
}

void ast_print(Ast ast) {
    int depth = -1;
    for (u16 i = 0; i < ast.len; i++) {
        P(Decl, ast.data[i]);
        printf(",\n");
    }
}