#include "nvcc.h"

Env *new_env() {
  Env *env = malloc(sizeof(Env));
  env->store = new_map();

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
