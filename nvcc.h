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

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
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
  TK_EOF,        // 入力の終わりを表すトークン
};

typedef struct {
  int ty;       // トークンの型
  int val;      // tyがTK_NUMだった場合、その数値
  char *input;  // トークン文字列(エラーメッセージ用)
} Token;

Token *new_token(int ty);
Token *new_token_num(int val);
void tokenize();

/* parser */

// 抽象構文木のノードの型を表す値
enum {
  ND_NUM = 256  // 整数のノードの型
};

typedef struct Node {
  int ty;             // 演算しかND_NUM
  struct Node *lhs;   // 左辺
  struct Node *rhs;   // 右辺
  int val;            // tyがND_NUMの場合のみ使う
} Node;

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
int consume(int ty);
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

// code generator
void gen(Node *node);

// gloabal variables
Vector *tokens;    // トークンを格納するためのベクタ
int pos;           // 現在着目しているトークンのインデックス
char *user_input;  // 入力プログラム

#endif
