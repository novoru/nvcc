#include "nvcc.h"

Env *new_env() {
  Env *env = malloc(sizeof(Env));
  env->store = new_map();
  env->outer = NULL;

  return env;
}

Env *new_enclosed_env(Env *outer) {
  Env *env = malloc(sizeof(Env));
  env->outer = outer;
  env->store = new_map();

  return env;
}

Node *get_env(Env *env, char *name) {
  Node *node = (Node *)map_get(env->store, name);
  if(node == NULL && env->outer != NULL)
    node = (Node *)map_get(env->outer, name);

  return node;
}

void set_env(Env *env, char *name, Node *elm) {
  map_put(env->store, name, (void *)elm);
}

char *env_to_str(Env *env) {
  char *s = "";

  while(env->outer != NULL)
    s = format("%s\n%s", s, env_to_str(env->outer));

  for(int i = 0 ; i < env->store->keys->len; i++) {
    s = format("%s\nkey: %s, elm: %s\n",
	   s, (char *)env->store->keys->data[i],
	   type_to_str((Type *)env->store->vals->data[i]));
  }

  return s;
  
}

void inspect_env(Env *env) {
  fprintf(stderr, "%s\n", env_to_str(env));
}
