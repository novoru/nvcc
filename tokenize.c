#include "nvcc.h"

static Token *new_token(int ty, char *p);
static Token *new_token_num(int val, char *p);
static Token *new_token_ident(char *ident, char *p);

static Token *new_token(int ty, char *p) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->input = p;

  return token;
}

static Token *new_token_num(int val, char *p) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_NUM;
  token->val = val;
  token->input = p;

  return token;
}

static Token *new_token_ident(char *ident, char *p) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_IDENT;
  token->ident = ident;
  token->input = p;

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
      vec_push(tokens, (void *) new_token(TK_RETURN, p));
      i++;
      p += 6;
      continue;
    }
    
    if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      vec_push(tokens, (void *) new_token(TK_IF, p));
      i++;
      p += 2;
      continue;
    }
    
    if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      vec_push(tokens, (void *) new_token(TK_ELSE, p));
      i++;
      p += 4;
      continue;
    }
    
    if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      vec_push(tokens, (void *) new_token(TK_WHILE, p));
      i++;
      p += 5;
      continue;
    }
    
    if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      vec_push(tokens, (void *) new_token(TK_FOR, p));
      i++;
      p += 3;
      continue;
    }
    
    if(strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
      vec_push(tokens, (void *) new_token(TK_INT, p));
      i++;
      p += 3;
      continue;
    }
    
    if(strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
      vec_push(tokens, (void *) new_token(TK_SIZEOF, p));
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
      
      vec_push(tokens, (void *) new_token_ident(strndup(rpos, n/sizeof(char)), p));

      continue;
    }

    if(strncmp(p, "==", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_EQ, p));
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "!=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_NE, p));
      i++;
      p+=2;

      continue;
    }

    if(strncmp(p, "<=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_LE, p));
      i++;
      p+=2;

      continue;
    }
    
    if(strncmp(p, ">=", 2) == 0) {
      vec_push(tokens, (void *) new_token(TK_GE, p));
      i++;
      p+=2;

      continue;
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
       *p == '(' || *p == ')' || *p == '<' || *p == '>' ||
       *p == '=' || *p == ';' || *p == '{' || *p == '}' ||
       *p == ',' || *p == '&' ) {
      vec_push(tokens, (void *) new_token((int)*p, p));
      i++;
      p++;
      
      continue;
      
    }

    if(isdigit(*p)) {
      vec_push(tokens, (void *) new_token_num(strtol(p, &p, 10), p));
      i++;
      continue;
      
    }

    error_at(p, "トークナイズできません");
    
  }

  vec_push(tokens, (void *) new_token(TK_EOF, p));

}

int is_alnum(char c) {
  return ('a' <= c && c<= 'z') ||
         ('A' <= c && c<= 'Z') ||
         ('0' <= c && c<= '9') ||
         (c == '_');
}
