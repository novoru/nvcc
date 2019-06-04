#include "nvcc.h"

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

Node *new_node_ident(char name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;

  return node;
}

// 次のトークンが期待した型かどうかをチェックする関数
int consume(int ty) {
  if(((Token *) tokens->data[pos])->ty != ty)
    return 0;
  pos++;
  return 1;
}

void program() {
  int i = 0;
  while(((Token *)tokens->data[pos])->ty != TK_EOF)
    code[i++] = stmt();
  code[i] = NULL;

}

Node *stmt() {
  Node *node;

  if(consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  }
  else {
    node = expr();
  }
  
  if(!consume(';'))
    error_at(((Token *)tokens->data[pos])->input,
	     "';'ではないトークンです");
  return node;
} 

Node *expr() {
  Node *node = assign();

  return node;
}

Node *assign() {
  Node *node = equality();

  if(consume('='))
    node = new_node('=', node, assign());
  
  return node;
}

Node *equality() {
  Node *node = relational();

  if(consume(TK_EQ))
    node = new_node(ND_EQ, node, relational());
  if(consume(TK_NE))
    node = new_node(ND_NE, node, relational());
  
  return node;
}

Node *relational() {
  Node *node = add();

  for(;;) {
    if(consume('<'))
      node = new_node('<', node , add());
    else if(consume('>'))
      node = new_node('>', add(), node);
    else if(consume(TK_LE))
      node = new_node(ND_LE, node, add());
    else if(consume(TK_GE))
      node = new_node(ND_GE, add(), node);
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
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    return node;
  }

  // そうでなければ数値のはず
  if(((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);

  // そうでなければ識別子のはず
  if(((Token *)tokens->data[pos])->ty == TK_IDENT)
    return new_node_ident(((Token *)tokens->data[pos++])->ident);
  
  error_at(((Token *)tokens->data[pos])->input,
	   "数値でも識別子でも開きカッコでもないトークンです");
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

