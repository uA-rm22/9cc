#include "9cc.h"

int label_number = 0;

void gen_lval(Node *node){
	if(node->kind == ND_DEREF){
		gen(node->lhs);
	}else{
		if(node->kind != ND_LVAR){
			error("代入の左辺値が変数ではありません");
		}
		printf("mov r0, fp\n");
		printf("sub r0, r0, #%d\n", (lvar_max/2+lvar_max%2)*8 + 4 - node->offset*4);
	}
	printf("push {r0}\n");
}

void gen(Node *node){
	int ele_num = 0;
	switch(node->kind){
	case ND_RETURN:
		gen(node->lhs);
		printf("pop {r0}\n");
		printf("mov sp, fp\n");
		printf("pop {fp}\n");
		printf("bx lr\n");
		return;
	case ND_IF:
		gen(node->lhs); //条件式
		printf("pop {r0}\n");
		printf("cmp r0,#0\n");
		printf("beq .Lend%d\n",label_number);
		gen(node->rhs->lhs);//条件がtrueの時に実行される式
		printf(".Lend%d:\n",label_number);
		label_number += 1;
		return;
	case ND_IF_ELSE:
		gen(node->lhs); //条件式
		printf("pop {r0}\n");
		printf("cmp r0,#0\n");
		printf("beq .Lelse%d\n",label_number);
		gen(node->rhs->lhs);//条件がtrueの時に実行される式
		printf("b .Lend%d\n",label_number);
		printf(".Lelse%d:\n",label_number);
		gen(node->rhs->rhs);//条件がfalseの時に実行される式
		printf(".Lend%d:\n",label_number);
		label_number += 1;
		return;
	case ND_WHILE:
		printf(".Lbegin%d:\n",label_number);
		gen(node->lhs); //条件式
		printf("pop {r0}\n");
		printf("cmp r0,#0\n");
		printf("beq .Lend%d\n",label_number);
		gen(node->rhs);//条件がtrueの時に実行される式
		printf("b .Lbegin%d\n",label_number);
		printf(".Lend%d:\n",label_number);
		label_number += 1;
		return;
	case ND_FOR:
		gen(node->lhs->lhs);//初期化式
		printf(".Lbegin%d:\n",label_number);
		gen(node->rhs->lhs); //条件式
		printf("pop {r0}\n");
		printf("cmp r0,#0\n");
		printf("beq .Lend%d\n",label_number);
		gen(node->rhs->rhs);//条件がtrueの時に実行される式
		gen(node->lhs->rhs);//ループ終了時に実行される式		
		printf("b .Lbegin%d\n",label_number);
		printf(".Lend%d:\n",label_number);
		label_number += 1;
		return;
	case ND_BLOCK:
		while(node->statements_pointer[ele_num] != NULL){
			gen(node->statements_pointer[ele_num]);
			printf("pop {r0}\n");
			ele_num += 1;
		}
		return;
	case ND_FUNCDEF:
		for(ele_num = 0; ele_num < node->len; ele_num++){
			printf("%c",*(node->name));
			node->name++;
		}
		printf(":\n");
		printf("str fp,[sp, #-4]!\n");//push fp
		printf("add fp, sp, #0\n");
		printf("sub sp, sp, #%d\n", (lvar_max/2+lvar_max%2)*8 + 4);
 		for( ele_num = 0; ele_num < node->val; ele_num++ ){
			printf("str r%d, [fp, #%d]\n", ele_num, -((lvar_max/2+lvar_max%2)*8 + 4 - ele_num*4) );
		}
		return;
	case ND_FUNCEND:
		printf("pop {r0}\n");//pop r0
		printf("add sp, fp, #0\n");
		printf("ldr fp, [sp], #4\n");
		printf("bx lr\n");
		return;
	case ND_FUNCCALL:
		printf("push {lr}\n");
		for(ele_num = 0; ele_num < node->val; ele_num++){
			gen(node->statements_pointer[ele_num]);
		}
		for(ele_num = node->val;0 < ele_num  ; ele_num--){
			printf("pop {r%d}\n", ele_num-1);
		}
		printf("bl ");
		for(ele_num = 0; ele_num < node->len; ele_num++){
			printf("%c",*(node->name));
			node->name++;
		}
		printf("\n");
		printf("pop {lr}\n");
		printf("push {r0}\n");
		return;
		
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
	case ND_ADDR:
		gen_lval(node->lhs);
		return;
	case ND_DEREF:
		gen(node->lhs);
		printf("pop {r0}\n");
		printf("ldr r0, [r0]\n");
		printf("push {r0}\n");
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
