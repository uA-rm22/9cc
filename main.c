#include "9cc.h"

char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
	user_input = argv[1];
	token = tokenize(argv[1]);
	program();

  printf(".global main\n");
  printf("main:\n");
	//プロローグ
	printf("push {fp}\n");
	printf("mov fp, sp\n");
	printf("sub sp, #104\n");

	for(int i=0; code[i]; i++){
		gen(code[i]);
		printf("pop {r0}\n");
	}
  printf("mov sp,fp\n");//ldr r0,[sp],#4 (post index)
	printf("pop {fp}\n");
  printf("bx lr\n");
  return 0;
}