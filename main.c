#include "9cc.h"

char *user_input;
Token *token;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

	user_input = argv[1];
	token = tokenize(argv[1]);
	program();

  printf(".global main\n");
	for(int i=0; code[i]; i++){
		gen(code[i]);
	}
  return 0;
}