#include "nvcc.h"

Token *new_token(int ty) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->input = user_input;

  return token;
}

Token *new_token_num(int val) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_NUM;
  token->val = val;
  token->input = user_input;

  return token;
}

Token *new_token_ident(char *ident) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_IDENT;
  token->ident = ident;
  token->input = user_input;

  return token;
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

    if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      vec_push(tokens, (void *) new_token(TK_RETURN));
      i++;
      p += 6;
      continue;
    }
    
    if('a' <= *p && *p <= 'z') {
      char  *rpos = p;
      int n = 0;
      
      while(is_alnum(*p) && (*p != ' '  || *p != EOF || *p != '\t' ||
			     *p != '\n' || *p != '\r')) {
	i++;
	n++;
	p++;
      }
      
      vec_push(tokens, (void *) new_token_ident(strndup(rpos, n/sizeof(char))));

      continue;
    }

    if(strncmp(p, "==", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_EQ));
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "!=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_NE));
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "<=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_LE));
      i++;
      p+=2;

      continue;
    }
    
    if(strncmp(p, ">=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_GE));
      i++;
      p+=2;

      continue;
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
       *p == '(' || *p == ')' || *p == '<' || *p == '>' ||
       *p == '=' || *p == ';') {
      vec_push(tokens, (void *) new_token((int)*p));
      i++;
      p++;
      
      continue;
      
    }

    if(isdigit(*p)) {
      vec_push(tokens, (void *) new_token_num(strtol(p, &p, 10)));
      i++;
      continue;
      
    }

    error_at(p, "トークナイズできません");
    
  }

  vec_push(tokens, (void *) new_token(TK_EOF));

}

int is_alnum(char c) {
  return ('a' <= c && c<= 'z') ||
         ('A' <= c && c<= 'Z') ||
         ('0' <= c && c<= '9') ||
         (c == '_');
}
