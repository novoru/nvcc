#ifndef NVCC_H
#define NVCC_H

#include "nvcc.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* container */

// 可変長ベクタ
typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
int expect(int line, int expected, int actual);
void runtest();

/* tokenizer */

// トークンの型を表す値
enum {
  TK_NUM = 256,  // 整数トークン
  TK_EQ,         // ==
  TK_NE,         // !=
  TK_LE,         // <=
  TK_GE,         // >=
  TK_IDENT,      // 識別子
  TK_RETURN,     // return
  TK_IF,         // if
  TK_ELSE,       // else
  TK_WHILE,      // while
  TK_FOR,        // for
  TK_INT,        // int
  TK_EOF,        // 入力の終わりを表すトークン
};

typedef struct {
  int ty;       // トークンの型
  int val;      // tyがTK_NUMだった場合、その数値
  char *ident;   // tyがTK_IDENTだった場合、その文字列
  char *input;  // トークン文字列(エラーメッセージ用)
} Token;

Token *new_token(int ty);
Token *new_token_num(int val);
Token *new_token_ident(char *ident);
void tokenize();
int is_alnum(char c);

/* parser */

// 抽象構文木のノードの型を表す値
enum {
  ND_NUM = 256,  // 整数のノードの型
  ND_IDENT,      // 識別子のノードの型
  ND_RETURN,
  ND_WHILE,
  ND_FOR,
  ND_IF,
  ND_BLOCK,
  ND_FUNC,
  ND_INT,
  ND_EQ,
  ND_NE,
  ND_LE,
  ND_GE,
};

typedef struct Node {
  int ty;

  union {
  
    struct {
      struct Node *lhs;     // 左辺
      struct Node *rhs;     // 右辺
    };
    
    int val;              // tyがND_NUMの場合のみ使う

    struct {
      char *name;

      union {
	// ND_IDENT
	int offset;

	// ND_FUNC
	Vector *args;
      };
    };
    
    // control flow
    struct {
      struct Node *cond;
      struct Node *conseq;
      
      union {
	// ND_IF, ND_WHILE
	struct {
	  struct Node *_else;
	};

	// ND_FOR
	struct {
	  struct Node *init;
	  struct Node *update;
	};
      };

    };
    
    // ND_BLOCK
    Vector *stmts;

  };
} Node;

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char name, int offset);
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
int consume(int ty);
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

/* Environment */
typedef struct {
  Map *store;
  struct Env *outer;
} Env;

Env *new_env();
Env *new_enclosed_env(Env *outer);
Node *get_env(Env *env, char *name);
void set_env(Env *env, char *name, Node *elm);

// code generator
void gen(Node *node);

// gloabal variables
Vector *tokens;    // トークンを格納するためのベクタ
int pos;           // 現在着目しているトークンのインデックス
char *user_input;  // 入力プログラム
Node *code[100];
Env *global_scope;
int nlabels;       // if文で使用するラベルの通し番号

#endif
