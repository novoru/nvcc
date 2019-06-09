#include "nvcc.h"

static Node *new_node(int ty, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *new_node_ident(char name, int offset);
static Node *stmt(Env *env);
static Node *expr(Env *env);
static Node *assign(Env *env);
static Node *equality(Env *env);
static Node *relational(Env *env);
static Node *add(Env *env);
static Node *mul(Env *env);
static Node *unary(Env *env);
static Node *term(Env *env);
static int consume(int ty);


static Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  
  return node;
}

static Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;

  return node;
}

static Node *new_node_ident(char name, int offset) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  node->offset = offset;

  return node;
}

static Type *new_type_int() {
  Type *type = malloc(sizeof(Type));
  type->ty = TY_INT;

  return type;
}

// 現在のトークンが期待した型かどうかをチェックする関数
static int cur_token_is(int ty) {
  return (((Token *) tokens->data[pos])->ty == ty);
}

// 現在のトークンが期待した型であれば、posをインクリメントする関数
static int consume(int ty) {
  if(!cur_token_is(ty))
    return 0;
  pos++;
  return 1;
}

// 次のトークンが期待した型かどうかをチェックする関数
static int peek_token_is(int ty) {
  return (((Token *)tokens->data[pos+1])->ty == ty);
}

// 次のトークンが期待した型であれば、posをインクリメントする関数
static int consume_peek(int ty) {
  if(!peek_token_is(ty))
    return 0;
  pos++;
  return 1;
}

static Node *return_stmt(Env *env) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_RETURN;
  node->lhs = expr(env);

  return node;
}

static Node *if_stmt(Env *env) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IF;
  
  if(consume('(')) {
    node->cond = expr(env);
    if(!consume(')'))
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    
    node->conseq = stmt(env);
    
    if(consume(TK_ELSE))
      node->_else = stmt(env);

    return node;
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  return node;
}

static Node *while_stmt(Env *env) {
  Node *node = malloc(sizeof(Node));
  node = malloc(sizeof(Node));
  node->ty = ND_WHILE;

  if(consume('(')) {
    node->cond = expr(env);
    if(!consume(')'))
      error_at(((Token *)tokens->data[pos])->input,
	       "開きカッコに対応する閉じカッコがありません");
    node->conseq = stmt(env);

    return node;
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  return node;
}

static Node *for_stmt(Env *env) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FOR;

  if(consume('(')) {
      
    if(!consume(';')) {
      node->init = expr(env);
      if(!consume(';'))
	error_at(((Token *)tokens->data[pos])->input,
		 "';'ではないトークンです");
    }
      
    if(!consume(';')) {
      node->cond = expr(env);
      if(!consume(';'))
	error_at(((Token *)tokens->data[pos])->input,
		 "';'ではないトークンです");
    }
      
    if(!consume(')')) {
      node->update = expr(env);
      if(!consume(')'))
	error_at(((Token *)tokens->data[pos])->input,
		 "開きカッコに対応する閉じカッコがありません");
    }
  }
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "'('ではないトークンです");

  node->conseq = stmt(env);

  return node;
}

static Node *block_stmt(Env *env) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_BLOCK;
  node->stmts = new_vector();
  while(!consume('}'))
    vec_push(node->stmts, (void *)stmt(env));

  return node;
}

static Node *declare_int_stmt(Env *env) {
  Token *token = ((Token *)tokens->data[pos]);
  
  if(!cur_token_is(TK_IDENT))
    error_at(token->input, "識別子ではないトークンです");
    
  char *ident = token->ident;
  int offset = (int)get_env(global_scope, ident);
  if(offset == NULL) {
    offset = (global_scope->store->keys->len + 1) * 8;
    set_env(global_scope, ident, (void *)offset);
  }

  set_env(env, ident, new_type_int());
  
  pos++;
  
  return new_node_ident(ident, offset);
}

static Node *call_func(Env *env) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = ((Token *)tokens->data[pos++])->ident;
  node->args = new_vector();

  if(consume_peek(')')) {
    pos++;
    return node;
  }

  pos++;
      
  vec_push(node->args, (void *)expr(env));

  while(consume(',')) {
    vec_push(node->args, (void *)expr(env));
  }

  if(!consume(')'))
    error_at(((Token *)tokens->data[pos])->input,
	     "開きカッコに対応する閉じカッコがありません");

  return node;
}

static Node *function() {
  Token *token = (Token *)tokens->data[pos];

  if(token->ty != TK_IDENT)
    error_at(token->input, "関数名ではないトークンです");

  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  node->name = token->ident;
  node->args = new_vector();
  node->env = new_env();

  pos++;
  
  if(!consume('('))
    error_at(token->input, "'('ではないトークンです");

  while(consume(',')) {
    vec_push(node->args, (void *)expr(node->env));
  }
  
  if(!consume(')'))
    error_at(token->input, "開きカッコに対応する閉じカッコがありません");

  if(!consume('{'))
    error_at(token->input, "'{'ではないトークンです");

  node->block = block_stmt(node->env);
  
  return node;
  
}

void program() {
  int i = 0;
  while(!cur_token_is(TK_EOF))
    code[i++] = function();
  code[i] = NULL;

}

static Node *stmt(Env *env) {
  Node *node;

  if(consume(TK_RETURN)) {
    node = return_stmt(env);
  }
  else if(consume(TK_IF)) {
    return if_stmt(env);
  }
  else if(consume(TK_WHILE)) {
    return while_stmt(env);
  }
  else if(consume(TK_FOR)) {
    return for_stmt(env);
  }
  else if(consume('{')) {
    return block_stmt(env);
  }
  else if(consume(TK_INT)) {
    node = declare_int_stmt(env);
  }
  else {
    node = expr(env);
  }

  if(!consume(';'))
    error_at(((Token *)tokens->data[pos])->input,
	     "';'ではないトークンです");

  return node;
} 

static Node *expr(Env *env) {
  Node *node = assign(env);

  return node;
}

static Node *assign(Env *env) {
  Node *node = equality(env);

  if(consume('='))
    node = new_node('=', node, assign(env));
  
  return node;
}

static Node *equality(Env *env) {
  Node *node = relational(env);

  if(consume(TK_EQ))
    node = new_node(ND_EQ, node, relational(env));
  if(consume(TK_NE))
    node = new_node(ND_NE, node, relational(env));
  
  return node;
}

static Node *relational(Env *env) {
  Node *node = add(env);

  for(;;) {
    if(consume('<'))
      node = new_node('<', node , add(env));
    else if(consume('>'))
      node = new_node('>', add(env), node);
    else if(consume(TK_LE))
      node = new_node(ND_LE, node, add(env));
    else if(consume(TK_GE))
      node = new_node(ND_GE, add(env), node);
    return node;
  }
}

static Node *add(Env *env) {
  Node *node = mul(env);

  for(;;) {
    if(consume('+'))
      node = new_node('+', node, mul(env));
    else if(consume('-'))
      node = new_node('-', node, mul(env));
    else
      return node;
  }
}

static Node *mul(Env *env) {
  Node *node = unary(env);

  for(;;) {
    if(consume('*'))
      node = new_node('*', node, unary(env));
    else if(consume('/'))
      node = new_node('/', node, unary(env));
    else
      return node;
  }
}

static Node *unary(Env *env) {
  if(consume('+'))
    return term(env);
  if(consume('-'))
    return new_node('-', new_node_num(0), term(env));
  return term(env);
  
}

static Node *term(Env *env) {
  Token *token = (Token *)tokens->data[pos];
  
  // 次のトークンが'('なら"(" expr ")"のはず
  if(consume('(')) {
    Node *node = expr(env);
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
      return call_func(env);
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

char *node_to_str(Node *node) {
  char *nd = "";

  switch(node->ty) {
  case ND_NUM:
    nd = "ND_NUM";
    break;
  case ND_IDENT:
    nd = "ND_IDENT";
    break;
  case ND_RETURN:
    nd = "ND_RETURN";
    break;
  case ND_WHILE:
    nd = "ND_WHILE";
    break;
  case ND_FOR:
    nd = "ND_FOR";
    break;
  case ND_IF:
    nd = "ND_IF";
    break;
  case ND_BLOCK:
    nd = "ND_BLOCK";
    break;
  case ND_CALL:
    nd = "ND_CALL";
    break;
  case ND_FUNC:
    nd = "ND_FUNC";
    break;
  case ND_INT:
    nd = "ND_INT";
    break;
  case ND_EQ:
    nd = "ND_EQ";
    break;
  case ND_NE:
    nd = "ND_NE";
    break;
  case ND_LE:
    nd = "ND_LE";
    break;
  case ND_GE:
    nd = "ND_GE";
    break;
  default:
    nd = "unknown";
    break;
  }

  return nd;
}

void inspect_node(Node *node) {
  fprintf(stderr, "node: %s\n", node_to_str(node));
}

char *type_to_str(Type *type) {
  char *ty = "";

  switch(type->ty) {
  case TY_INT:
    ty = "TY_INT";
    break;
  case TY_PTR:
    ty = "TY_INT";
    break;
  default:
    ty = "unknown";
    break;
  }

  return ty;
}

void inspect_type(Type *type) {
  fprintf(stderr, "type: %s\n", type_to_str(type));
}
