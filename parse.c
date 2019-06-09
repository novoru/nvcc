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

Node *new_node_ident(char name, int offset) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  node->offset = offset;

  return node;
}

// 現在のトークンが期待した型かどうかをチェックする関数
int cur_token_is(int ty) {
  return (((Token *) tokens->data[pos])->ty == ty);
}

// 現在のトークンが期待した型であれば、posをインクリメントする関数
int consume(int ty) {
  if(!cur_token_is(ty))
    return 0;
  pos++;
  return 1;
}

// 次のトークンが期待した型かどうかをチェックする関数
int peek_token_is(int ty) {
  return (((Token *)tokens->data[pos+1])->ty == ty);
}

// 次のトークンが期待した型であれば、posをインクリメントする関数
int consume_peek(int ty) {
  if(!peek_token_is(ty))
    return 0;
  pos++;
  return 1;
}

Node *return_stmt() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_RETURN;
  node->lhs = expr();

  return node;
}

Node *if_stmt() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IF;
  
  if(consume('(')) {
    node->cond = expr();
    if(!consume(')'))
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    
    node->conseq = stmt();
    
    if(consume(TK_ELSE))
      node->_else = stmt();

    return node;
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  return node;
}

Node *while_stmt() {
  Node *node = malloc(sizeof(Node));
  node = malloc(sizeof(Node));
  node->ty = ND_WHILE;

  if(consume('(')) {
    node->cond = expr();
    if(!consume(')'))
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    node->conseq = stmt();

    return node;
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  return node;
}

Node *for_stmt() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FOR;

  if(consume('(')) {
      
    if(!consume(';')) {
      node->init = expr();
      if(!consume(';'))
	error_at(((Token *)tokens->data[pos])->input,
		 "';'ではないトークンです");
    }
      
    if(!consume(';')) {
      node->cond = expr();
      if(!consume(';'))
	error_at(((Token *)tokens->data[pos])->input,
		 "';'ではないトークンです");
    }
      
    if(!consume(')')) {
      node->update = expr();
      if(!consume(')'))
	error_at(((Token *)tokens->data[pos])->input,
		 "開きカッコに対応する閉じカッコがありません");
    }
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  node->conseq = stmt();

  return node;
}

Node *block_stmt() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_BLOCK;
  node->stmts = new_vector();
  while(!consume('}'))
    vec_push(node->stmts, (void *)stmt());

  return node;
}

Node *declare_int_stmt() {
  Token *token = ((Token *)tokens->data[pos]);
  
  if(!cur_token_is(TK_IDENT))
    error_at(token->input, "識別子ではないトークンです");
    
  char *ident = token->ident;
  int offset = (int)get_env(global_scope, ident);
  if(offset == NULL) {
    offset = (global_scope->store->keys->len + 1) * 8;
    set_env(global_scope, ident, (void *)offset);
  }

  pos++;
  
  return new_node_ident(ident, offset);
}

Node *call_func() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = ((Token *)tokens->data[pos++])->ident;
  node->args = new_vector();

  if(consume_peek(')')) {
    pos++;
    return node;
  }

  pos++;
      
  vec_push(node->args, (void *)expr());

  while(consume(',')) {
    vec_push(node->args, (void *)expr());
  }

  if(!consume(')'))
    error_at(((Token *)tokens->data[pos])->input,
	     "開きカッコに対応する閉じカッコがありません");

  return node;
}

Node *function() {
  Token *token = (Token *)tokens->data[pos];

  if(token->ty != TK_IDENT)
    error_at(token->input, "関数名ではないトークンです");

  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  node->name = token->ident;
  node->args = new_vector();

  pos++;
  
  if(!consume('('))
    error_at(token->input, "'('ではないトークンです");

  while(consume(',')) {
    vec_push(node->args, (void *)expr());
  }
  
  if(!consume(')'))
    error_at(token->input, "開きカッコに対応する閉じカッコがありません");

  if(!consume('{'))
    error_at(token->input, "'{'ではないトークンです");

  node->block = block_stmt();
  
  return node;
  
}

void program() {
  int i = 0;
  while(!cur_token_is(TK_EOF))
    code[i++] = function();
  code[i] = NULL;

}

Node *stmt() {
  Node *node;

  if(consume(TK_RETURN)) {
    node = return_stmt();
  }
  else if(consume(TK_IF)) {
    return if_stmt();
  }
  else if(consume(TK_WHILE)) {
    return while_stmt();
  }
  else if(consume(TK_FOR)) {
    return for_stmt();
  }
  else if(consume('{')) {
    return block_stmt();
  }
  else if(consume(TK_INT)) {
    node = declare_int_stmt();
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
  Token *token = (Token *)tokens->data[pos];
  
  // 次のトークンが'('なら"(" expr ")"のはず
  if(consume('(')) {
    Node *node = expr();
    if(!consume(')'))
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    return node;
  }

  if(cur_token_is(TK_NUM)) {
    pos++;
    return new_node_num(token->val);
  }

  if(cur_token_is(TK_IDENT)) {
    
    // 識別子の次のトークンが'('の場合は関数名
    if(peek_token_is('(')) {
      return call_func();
    }
    //そうでなければ変数名
    else {
      pos++;
      char *ident = token->ident;
      int offset = (int)get_env(global_scope, ident);

      // 定義されていない変数名が現れたらエラー
      if(offset == NULL)
	error("定義されていない変数名です: %s", ident);
      
      return new_node_ident(ident, offset);
    }
  }

  error_at(token->input, "数値でも識別子でも開きカッコでもないトークンです");
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

