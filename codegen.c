#include "9cc.h"

void gen_lval(Node *node){
	if(node->kind != ND_LVAR){
		error("代入の左辺値が変数ではありません");
	}
	printf("mov r0, fp\n");
	printf("sub r0, r0, #%d\n", node->offset);
	printf("push {r0}\n");
}

void gen(Node *node){
	if(node->kind == ND_RETURN){
		gen(node->lhs);
		printf("pop {r0}\n");
		printf("mov sp, fp\n");
		printf("pop {fp}\n");
		printf("bx lr\n");
		return;
	}

	switch(node->kind){
	case ND_NUM:
		printf("ldr r0,=%d\n", node->val);
		printf("push {r0}\n");
		return;
	case ND_LVAR:
		gen_lval(node);
		printf("pop {r0}\n");
		printf("ldr r0,[r0]\n");
		printf("push {r0}\n");
		return;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("pop {r1}\n");
		printf("pop {r0}\n");//left value(address)
		printf("str r1 ,[r0]\n");
		printf("push {r1}\n");
		return;
	}
	gen(node->lhs);
	gen(node->rhs);
	printf("pop {r1}\n");
	printf("pop {r0}\n");//left value

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
