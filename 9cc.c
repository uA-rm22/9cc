#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
}TokenKind;

typedef struct Token Token;
struct Token{
	TokenKind kind;
	Token *next;
	int val;
	char *str;
};

Token *token;
char *user_input;

void error_at(char *loc ,char *fmt,...){
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr,"%s\n", user_input);
	fprintf(stderr,"%*s", pos, "");
	fprintf(stderr,"^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool consume(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op ){
		return false;
	}
	token = token->next;
	return true;
}	

void expect(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op){
		error_at(token->str, "'%c'ではありません",op);
	}
	token = token->next;
}

int expect_number(){
	if(token->kind != TK_NUM){
		error_at(token->str, "数ではありません");
	}
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof(){
	return token->kind==TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str=str;
	cur->next = tok;
	return tok;
}

Token *tokenize(char *p){
	Token head;
	head.next=NULL;
	Token *cur = &head;

	while(*p){
		if(isspace(*p)){
			p++;
			continue;
		}

		if(*p == '+' || *p == '-'){
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if(isdigit(*p)){
			cur=new_token(TK_NUM, cur, p);
			cur->val=strtol(p, &p, 10);
			continue;
		}

		error_at(token->str, "トークナイズできません");
	}
	new_token(TK_EOF, cur, p);
	return head.next;
}

typedef enum{
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
}NodeKind;

typedef struct Node Node;

struct Node{
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node *mul();
Node *primary();
Node *expr();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *mul(){
	Node *node = primary();
	for(;;){
		if(consume('*')){
			node = new_node(ND_MUL, node, primary());
		}else if(consume('/')){
			node = new_node(ND_DIV, node, primary());
		}else{
			return node;
		}
	}
}

Node *expr(){
	Node *node = mul();
	for(;;){
		if(consume('+')){
			node = new_node(ND_ADD, node, mul());
		}else if(consume('-')){
			node = new_node(ND_SUB, node, mul());
		}else{
			return node;
		}
	}
}

Node *primary(){
	if(consume('(')){
		Node *node = expr();
		expect(')');
		return node;
	}

	return new_node_num(expect_number());
}

void gen(Node *node){
	if(node->kind == ND_NUM){
		char *tmp;

		printf("ldr r0,=%d\n", node->val);
		printf("str r0, [sp, #-4]!\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("ldr r1, [sp], #4\n");
	printf("ldr r0, [sp], #4\n");

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
			printf("eor r4, r0, r1\n");
			printf("ratio DivMod R2, R0, R1, R3\n");//R2<= ans R0/R1 R3<=temp R0<=res

			printf("cmp r4, #0\n");
			printf("rsblt r2, #0\n");
			printf("mov r0, r2\n");
			break;
	}
	printf("str r0, [sp,#-4]!\n");
}


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

  printf("ldr r0,[sp],#4\n");
  printf("bx lr\n");
  return 0;
}