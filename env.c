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

Var *get_env(Env *env, char *name) {
  Var *var = (Var *)map_get(env->store, name);
  if(var == NULL && env->outer != NULL)
    var = (Var *)map_get(env->outer, name);

  return var;
}

void set_env(Env *env, char *name, Var *elm) {
  map_put(env->store, name, (void *)elm);
}

char *env_to_str(Env *env) {
  char *s = "";

  while(env->outer != NULL)
    s = format("%s\n%s", s, env_to_str(env->outer));

  for(int i = 0 ; i < env->store->keys->len; i++) {
    s = format("%s\t  key: %s, elm: %s\n",
	   s, (char *)env->store->keys->data[i],
	   var_to_str((Var *)env->store->vals->data[i]));
  }

  return s;
  
}

void inspect_env(Env *env) {
  fprintf(stderr, "env -> {\n%s\t}\n", env_to_str(env));
}
