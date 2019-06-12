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
  tokenize();
  program();

  
  // プロローグ
  printf(".intel_syntax noprefix\n");

  // 先頭の式から順にコード生成
  for(int i = 0; code[i]; i++) {
    gen(code[i]);
  }

  return 0;
}
