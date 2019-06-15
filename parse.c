#include "nvcc.h"

static Node *new_node(int ty, Token *token);
static Node *new_node_num(int val);
static Node *new_node_binop(int ty, Node *lhs, Node *rhs);
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


static Node *new_node_binop(int ty, Node *lhs, Node *rhs) {
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

static Node *new_node(int ty, Token *token) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->token = token;

  return node;
}

static Type *new_type_int() {
  Type *type = malloc(sizeof(Type));
  type->ty = TY_INT;

  return type;
}

static Type *new_type_ptr() {
  Type *type = malloc(sizeof(Type));
  type->ty = TY_PTR;

  return type;
}

static int align(Type *type) {
  switch(type->ty) {
  case TY_INT:
    return 8;
  case TY_PTR:
    return 16;
  default:
    return 16;
  }
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

  while(!consume('}')) {
    vec_push(node->stmts, (void *)stmt(env));
  }

  return node;
}

static Node *call_func(Env *env, Token *token) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->token = token;
  node->args = new_vector();

  if(consume(')')) {
    return node;
  }

  vec_push(node->args, (void *)expr(env));

  while(consume(','))
    vec_push(node->args, (void *)expr(env));

  if(!consume(')'))
    error_at(((Token *)tokens->data[pos])->input,
	     "開きカッコに対応する閉じカッコがありません");

  return node;
}

static Node *parse_arg(Env *env) {
  if(!consume(TK_INT))
    error_at(((Token *)tokens->data[pos])->input,
	     "型名ではないトークンです");

  Type *type = new_type_int();
  Token *token = (Token*)tokens->data[pos];
  
  if(!consume(TK_IDENT))
    error_at(((Token *)tokens->data[pos])->input,
	     "識別子ではないトークンです");

  
  Node *node = new_node(ND_VARDEF, token);
  Var *var = malloc(sizeof(Var));
  var->type = type;
  var->isarg = 1;

  int offset = 0;
  for(int i = 0; i < env->store->keys->len; i++) {
    offset += align(((Var *)env->store->vals->data[i])->type);
  }

  var->offset = offset + align(type);

  node->var = var;
  set_env(env, token->ident, var);
  
  return node;
  
}

Type *type_specifier() {
  if(consume(TK_INT)) {
    return new_type_int();
  }
  else if(cur_token_is(TK_IDENT))
    return NULL;
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "型名でも識別子でもないトークンです");
}

Node *declaration(Env *env) {

  Type *type;
  Type *tmptype = type_specifier();

  if(consume('*')) {
    type = new_type_ptr();
    type->ptr_to = tmptype;
  }
  else if(cur_token_is(TK_IDENT))
    type = tmptype;
  else
    error_at(((Token *)tokens->data[pos])->input,
	     "ポインタでも識別子でもないトークンです");

  Token *token = (Token *)tokens->data[pos];
  
  if(!cur_token_is(TK_IDENT))
    error_at(token->input,
	     "識別子ではないトークンです");

  Node *node = new_node(ND_VARDEF, token);
  Var *var = malloc(sizeof(Var));
  var->type = type;
  node->var = var;

  int offset = 0;
  for(int i = 0; i < env->store->keys->len; i++) {
    offset += align(((Var *)env->store->vals->data[i])->type);
  }

  var->offset = offset + align(type);
  set_env(env, token->ident, var);
  
  pos++;

  return node;
}

static Node *primary(Env *env, Token *token) {
  if(consume('('))
    return call_func(env, token);

  Var *var = get_env(env, token->ident);
  
  if(var == NULL)
    error("定義されていない変数名です: %s", token->ident);

  Node *node = new_node(ND_VARREF, token);
  node->var = var;

  return node;
  
}

static Node *function() {
  Type *rettype = type_specifier();
  
  Token *token = (Token *)tokens->data[pos];
  
  if(token->ty != TK_IDENT)
    error_at(token->input, "関数名ではないトークンです");

  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  node->token = token;
  node->args = new_vector();
  node->env = new_env();
  node->rettype = rettype;

  pos++;

  if(!consume('('))
    error_at(token->input, "'('ではないトークンです");

  if(!consume(')')) {
    vec_push(node->args, (void *)parse_arg(node->env));
    while(consume(',')) {
      vec_push(node->args, (void *)parse_arg(node->env));
    }

    if(!consume(')'))
      error_at(token->input, "開きカッコに対応する閉じカッコがありません");
  }

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
  else if(cur_token_is(TK_INT)) {
    node = declaration(env);
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
    node = new_node_binop('=', node, assign(env));
  
  return node;
}

static Node *equality(Env *env) {
  Node *node = relational(env);

  if(consume(TK_EQ))
    node = new_node_binop(ND_EQ, node, relational(env));
  if(consume(TK_NE))
    node = new_node_binop(ND_NE, node, relational(env));
  
  return node;
}

static Node *relational(Env *env) {
  Node *node = add(env);

  for(;;) {
    if(consume('<'))
      node = new_node_binop('<', node , add(env));
    else if(consume('>'))
      node = new_node_binop('>', add(env), node);
    else if(consume(TK_LE))
      node = new_node_binop(ND_LE, node, add(env));
    else if(consume(TK_GE))
      node = new_node_binop(ND_GE, add(env), node);
    return node;
  }
}

static Node *add(Env *env) {
  Node *node = mul(env);

  for(;;) {
    if(consume('+'))
      node = new_node_binop('+', node, mul(env));
    else if(consume('-'))
      node = new_node_binop('-', node, mul(env));
    else
      return node;
  }
}

static Node *mul(Env *env) {
  Node *node = unary(env);

  for(;;) {
    if(consume('*'))
      node = new_node_binop('*', node, unary(env));
    else if(consume('/'))
      node = new_node_binop('/', node, unary(env));
    else
      return node;
  }
}

static Node *unary(Env *env) {
  if(consume('+'))
    return term(env);
  if(consume('-'))
    return new_node_binop('-', new_node_num(0), term(env));
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
    pos++;
    return primary(env, token);
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
    nd = format("{ ty:ND_NUM, val: %d }", node->val);
    break;
  case ND_IDENT:
    nd = format("{ ty: ND_IDENT, ident: %s }", node->token->ident);
    break;
  case ND_VARDEF:
    nd = format("{ ty: ND_VARDEF, ident: %s, var: %s }",
		node->token->ident, var_to_str(node->var));
    break;
  case ND_VARREF:
    nd = format("{ ty: ND_VARREF, ident: %s, var: %s }",
		node->token->ident, var_to_str(node->var));
    break;
  case ND_RETURN:
    nd = format("{ ty: ND_RETURN, lhs: %s }", node_to_str(node->lhs));
    break;
  case ND_WHILE:
    nd = format("{ ty: ND_WHILE, cond: %s, conseq: %s }",
		node_to_str(node->cond), node_to_str(node->conseq));
    break;
  case ND_FOR:
    nd = format("{ ty: ND_FOR, init: %s, cond: %s, update: %s ,conseq: %s }",
		node_to_str(node->init), node_to_str(node->cond),
		node_to_str(node->update), node_to_str(node->conseq));
    break;
  case ND_IF:
    nd = format("{ ty: ND_IF, cond: %s, conseq: %s, else: %s }",
		node_to_str(node->cond), node_to_str(node->conseq),
		node_to_str(node->_else));
    break;
  case ND_BLOCK:
    for(int i = 0; i < node->stmts->len; i++) {
      char *str = node_to_str((Node *)node->stmts->data[i]);
      if(i == 0)
	nd = format("{ ty: ND_BLOCK, { %s, ", str);
      else
	nd = format("%s, %s", nd, str);
    }
    nd = format("%s } }", nd);
    break;
  case ND_CALL:
    nd = format("{ ty: ND_CALL, ident: %s, args: ", node->token->ident);
    for(int i = 0; i < node->args->len; i++) {
      char *str = node_to_str((Node *)node->args->data[i]);
      if(i == 0)
	nd = format("%s { %s, ", nd, str);
      else
	nd = format("%s,%s", nd, str);
    }
    nd = format("%s } }", nd);
    break;
  case ND_FUNC:
    nd = format("{ ty: ND_FUNC, ident: %s, args: ", node->token->ident);
    for(int i = 0; i < node->args->len; i++) {
      char *str = node_to_str((Node *)node->args->data[i]);
      if(i == 0)
	nd = format("%s { %s, ", nd, str);
      else
	nd = format("%s,%s", nd, str);
    }
    nd = format("%s }, block: %s, rettype: %s}",
		nd, node_to_str(node->block), type_to_str(node->rettype));
    break;
  case ND_EQ:
    nd = format("{ ty: ND_EQ, lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case ND_NE:
    nd = format("{ ty: ND_NE, lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case ND_LE:
    nd = format("{ ty: ND_LE, lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case ND_GE:
    nd = format("{ ty: ND_GE, lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '+':
    nd = format("{ ty: '+', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '-':
    nd = format("{ ty: '-', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '*':
    nd = format("{ ty: '*', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '/':
    nd = format("{ ty: '/', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '<':
    nd = format("{ ty: '<', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '>':
    nd = format("{ ty: '>', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
    break;
  case '=':
    nd = format("{ ty: '=', lhs: %s, rhs: %s }",
		node_to_str(node->lhs), node_to_str(node->rhs));
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
  if(type == NULL) return "null";
  
  char *ty = "";

  switch(type->ty) {
  case TY_INT:
    ty = "TY_INT";
    break;
  case TY_PTR:
    ty = format("%s -> %s", "TY_PTR", type_to_str(type->ptr_to));
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

char *var_to_str(Var *var) {
  char *s = "";

  s = format("{ type: %s, offset: %d, isarg: %d }",
	     type_to_str(var->type), var->offset, var->isarg);

  return s;
}

void inspect_var(Var *var) {
  fprintf(stderr, "var-> { %s }\n", var_to_str(var));
}
