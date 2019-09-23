#include "9cc.h"

void gen(Node *node){
	if(node->kind == ND_NUM){

		printf("ldr r0,=%d\n", node->val);
		printf("push {r0}\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("pop {r1}\n");
	printf("pop {r0}\n");

	switch(node->kind){
		case ND_ADD:
			printf("add r0, r0, r1\n");
			break;
		case ND_SUB:
			printf("sub r0, r0, r1\n");
			break;
		case ND_MUL:
			printf("mul r0, r0, r1\n");
			break;
		case ND_DIV:
			printf("push {lr}\n");
			printf("bl __aeabi_idiv\n");//call subroutine

			printf("pop {pc}\n");
			break;
		case ND_EQ:
			printf("mov r2, #0\n");
			printf("cmp r0, r1\n");
			printf("moveq r2, #1\n");
			printf("mov r0, r2\n");
			break;
		case ND_NEQ:
			printf("mov r2, #0\n");
			printf("cmp r0, r1\n");
			printf("movne r2, #1\n");
			printf("mov r0, r2\n");;
			break;
		case ND_LESS:
			printf("mov r2, #0\n");
			printf("cmp r0, r1\n");
			printf("movlt r2, #1\n");
			printf("mov r0, r2\n");
			break;
		case ND_LESSEQ:
			printf("mov r2, #0\n");
			printf("cmp r0, r1\n");
			printf("movle r2, #1\n");
			printf("mov r0, r2\n");
			break;
	}
	printf("push {r0}\n");//str r0,[sp, #-4]! (pre index)
}
