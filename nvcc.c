#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 可変長ベクタ
typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;

  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if(vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }

  vec->data[vec->len++] = elem;
}

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

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;

  return node;
}

// 入力プログラム
char *user_input;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンは来ないものとする
Token tokens[100];

// 現在着目しているトークンのインデックス
int pos;

// 次のトークンが期待した型かどうかをチェックする関数
int consume(int ty) {
  if(tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告するための関数
void error_at(char *loc, char *msg) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

// user_inputが指している文字列を
// トークンに分割してtokensに保存する
void tokenize() {
  char *p = user_input;

  int i = 0;
  while(*p) {
    // 空白文字をスキップ
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(strncmp(p, "==", 2) == 0) {
      tokens[i].ty = TK_EQ;
      tokens[i].input = p;
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "!=", 2) == 0) {
      tokens[i].ty = TK_NE;
      tokens[i].input = p;
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "<=", 2) == 0) {
      tokens[i].ty = TK_LE;
      tokens[i].input = p;
      i++;
      p+=2;

      continue;
    }
    
    if(strncmp(p, ">=", 2) == 0) {
      tokens[i].ty = TK_GE;
      tokens[i].input = p;
      i++;
      p+=2;

      continue;
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
       *p == '(' || *p == ')' || *p == '<' || *p == '>') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      
      continue;
      
    }

    if(isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
      
    }

    error_at(p, "トークナイズできません");
    
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
  
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

Node *expr() {
  Node *node = equality();

  return node;
}

Node *equality() {
  Node *node = relational();

  // TODO
  
  return node;
}

Node *relational() {
  Node *node = add();

  for(;;) {
    if(consume('<'))
      node = new_node('<', node , add());
    else if(consume('>'))
      node = new_node('>', add(), node);
    else if(consume(TK_EQ))
      node = new_node(TK_EQ, node, add());
    else if(consume(TK_NE))
      node = new_node(TK_NE, node, add());
    else if(consume(TK_LE))
      node = new_node(TK_LE, node, add());
    else if(consume(TK_GE))
      node = new_node(TK_GE, add(), node);
    return node;
  }
}

Node *add() {
  Node *node = mul();

  for(;;) {
    if(consume('+'))
      node = new_node('+', node, mul());
    else if(consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for(;;) {
    if(consume('*'))
      node = new_node('*', node, unary());
    else if(consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *unary() {
  if(consume('+'))
    return term();
  if(consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
  
}

Node *term() {
  // 次のトークンが'('なら"(" expr ")"のはず
  if(consume('(')) {
    Node *node = expr();
    if(!consume(')'))
      error_at(tokens[pos].input,
	       "開きカッコに対応する閉じカッコがありません");

    return node;
  }

  // そうでなければ数値のはず
  if(tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  error_at(tokens[pos].input,
	   "数値でも開きカッコでもないトークンです");
  
}

void gen(Node *node) {
  if(node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  imul rdi\n");
    break;
  case '/':
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case '<':
  case '>':
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_LE:
  case TK_GE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

int expect(int line, int expected, int actual) {
  if(expected == actual)
    return;
  fprintf(stderr, "%d: %d expected, but got %d\n",
	  line, expected, actual);
  exit(1);
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for(int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);

  printf("OK\n");
}

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if(strcmp(argv[1], "-test") == 0) {
    runtest();
    exit(0);
  }
  
  user_input = argv[1];
  tokenize();
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
  
}
