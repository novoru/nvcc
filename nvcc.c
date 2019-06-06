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
  variables = new_map();
  tokenize();
  program();

  // プロローグ
  // 変数26個分の領域を確保する
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  if(variables->keys > 0) {
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", (variables->keys->len)*8);
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
