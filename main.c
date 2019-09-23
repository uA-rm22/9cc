#include "9cc.h"

char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
	user_input = argv[1];
	token = tokenize(argv[1]);
	Node *node = expr();

  printf(".global main\n");
  printf("main:\n");

	gen(node);

  printf("pop {r0}\n");//ldr r0,[sp],#4 (post index)
  printf("bx lr\n");
  return 0;
}