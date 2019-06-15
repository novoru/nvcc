#include "nvcc.h"

static void gen_lval(Node *node) {
  if(node->ty == ND_VARREF) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
  }
  else if(node->ty == ND_DEREF || ND_ADDR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->expr->var->offset);
    printf("  push rax\n");
  }
  else
    error(" 代入の左辺値が変数ではありません");
}

static void gen_num(Node *node) {
  printf("  push %d\n", node->val);
}

static void push_var() {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

static void gen_assign(Node *node) {
  gen_lval(node->lhs);
  gen(node->rhs);
  
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

static void gen_return(Node *node) {
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

static void gen_if(Node *node) {
    gen(node->cond);
    
    printf("  mov rax, [rsp]\n");
    printf("  cmp rax, 0\n");
    
    if(node->_else == NULL) {
      printf("  je  .Lend%d\n", nlabels);
      printf("  sub rsp, 16\n");
      gen(node->conseq);
      printf(".Lend%d:\n", nlabels++);
    }
    else {
      printf("  je  .Lelse%d\n", nlabels);
      printf("  sub rsp, 16\n");
      gen(node->conseq);
      printf("  jmp .Lend%d\n", nlabels);
      printf(".Lelse%d:\n", nlabels);
      gen(node->_else);
      printf(".Lend%d:\n", nlabels++);
    }
}

static void gen_while(Node *node) {
  printf(".Lbegin%d:\n", nlabels);
  gen(node->cond);
  printf("  mov rax, [rsp]\n");
  printf("  cmp rax, 0\n");
  printf("  je  .Lend%d\n", nlabels);
  printf("  sub rsp, 16\n");
  gen(node->conseq);
  printf("  jmp .Lbegin%d\n", nlabels);
  printf(".Lend%d:\n", nlabels++);
}

static void gen_for(Node *node) {
  if(node->init != NULL) gen(node->init);
  printf(".Lbegin%d:\n", nlabels);
  if(node->cond != NULL) gen(node->cond);
  printf("  mov rax, [rsp]\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", nlabels);
  printf("  sub rsp, 16\n");
  gen(node->conseq);
  if(node->update != NULL) gen(node->update);
  printf("  jmp .Lbegin%d\n", nlabels);
  printf(".Lend%d:\n", nlabels++);
}

static void gen_block(Node *node) {
  for(int i = 0; i < node->stmts->len; i++) {
    gen((Node *)node->stmts->data[i]);
    if(i < node->stmts->len-1)
      printf("  pop rax\n");
  }
    
  // 空のブロック実行後はスタックが空になってしまうため、
  // スタックにダミーデータを積んでおく
  if(node->stmts->len == 0)
    printf("  push 0\n");
}

static void gen_call(Node *node) {
  if(node->args != NULL) {
    for(int i = 0; i < node->args->len; i++) {
      gen((Node *)node->args->data[i]);
      if(i == 0)
	printf("  pop rdi\n");
      if(i == 1)
	printf("  pop rsi\n");
      if(i == 2)
	printf("  pop rdx\n");
      if(i == 3)
	printf("  pop rcx\n");
      if(i == 4)
	printf("  pop r8\n");
      if(i == 5)
	printf("  pop r9\n");
    }
  }
  printf("  call %s\n", node->token->ident);
  printf("  push rax\n");
}

static void gen_func(Node *node) {
  printf(".global %s\n", node->token->ident);
  printf("%s:\n", node->token->ident);
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  
  Env *env = node->env;
  printf("  sub rsp, %d\n", 16 * env->store->keys->len);

  // 引数の値をローカル変数に書き出す

  int nargs = 0;
  for(int i = 0; i < env->store->keys->len; i++) {
    Var *var = (Var *)env->store->vals->data[i];
    if(var->isarg) {
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", var->offset);
      printf("  push rax\n");
      if(nargs == 0) printf("  mov rdi, rdi\n");
      if(nargs == 1) printf("  mov rdi, rsi\n");
      if(nargs == 2) printf("  mov rdi, rdx\n");
      if(nargs == 3) printf("  mov rdi, rcx\n");
      if(nargs == 4) printf("  mov rdi, r8\n");
      if(nargs == 5) printf("  mov rdi, r9\n");
    
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");

      nargs++;
    }
  }

  gen(node->block);
  
  printf("  ret\n");

  return;
}

void gen(Node *node) {
  if(node->ty == ND_NUM) {
    gen_num(node);
    return;
  }

  if(node->ty == ND_VARDEF)
    return;
  
  if(node->ty == ND_VARREF) {
    gen_lval(node);
    push_var();
    return;
  }

  if(node->ty == '=') {
    gen_assign(node);
    return;
  }

  if(node->ty == ND_RETURN) {
    gen_return(node);
    return;
  }

  if(node->ty == ND_IF) {
    gen_if(node);
    return;
  }

  if(node->ty == ND_WHILE) {
    gen_while(node);
    return;
  }

  if(node->ty == ND_FOR) {
    gen_for(node);
    return;
  }

  if(node->ty == ND_BLOCK) {
    gen_block(node);
    return;
  }

  if(node->ty == ND_CALL) {
    gen_call(node);
    return;
  }

  if(node->ty == ND_FUNC) {
    gen_func(node);
    return;
  }

  if(node->ty == ND_DEREF) {
    gen_lval(node->expr);
    push_var();
    return;
  }

  if(node->ty == ND_ADDR) {
    gen_lval(node->expr);
    push_var();
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
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
  case ND_GE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}
