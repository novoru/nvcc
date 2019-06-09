#include "nvcc.h"

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
  tokens = new_vector();
  global_scope = new_env();
  tokenize();
  program();

  
  // プロローグ
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  if(global_scope->store->keys > 0) {
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", ((global_scope->store->keys->len+1) / 2)*16);
  }
  
  // 先頭の式から順にコード生成
  for(int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 0;
}
