#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf(".global main\n");
	printf("main:\n");
	printf("str fp, [sp, #-4]!\n");
	printf("add fp, sp, #0\n");
  printf("  mov r3, #%d\n", atoi(argv[1]));
	printf("mov r0, r3\n");
	printf("add sp, fp, #0\n");
	printf("ldr fp,[sp], #4\n");
  printf("  bx lr\n");
  return 0;
}