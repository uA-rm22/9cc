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
	int len;
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

bool consume(char *op){
	if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len) ){
		return false;
	}
	token = token->next;
	return true;
}	

void expect(char *op){
	if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len) ){
		error_at(token->str, "'%s'ではありません",op);
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

Token *new_token(TokenKind kind, Token *cur, char *str, int len){
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str=str;
	tok->len = len;
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

		if(*p == '<' || *p == '>' || *p == '=' || *p == '!' ){
			if(*(p+1) == '='){
				cur = new_token(TK_RESERVED, cur, p, 2);
				p = p + 2;
				continue;
			}
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'|| *p == '>' || *p == '<'){
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if(isdigit(*p)){
			cur=new_token(TK_NUM, cur, p, 1);
			cur->val=strtol(p, &p, 10);
			continue;
		}

		error_at(token->str, "トークナイズできません");
	}
	new_token(TK_EOF, cur, p, 1);
	return head.next;
}

typedef enum{
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
	ND_EQ,
	ND_NEQ,
	ND_LESS,
	ND_LESSEQ
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
Node *unary();
Node *equality();
Node *relational();
Node *add();
Node *mul();

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

Node *expr(){
	return equality();
}

Node *equality(){
	Node *node = relational();
	for(;;){
		if(consume("==")){
			node = new_node(ND_EQ, node, relational());
		}else if(consume("!=")){
			node = new_node(ND_NEQ, node, relational());
		}else{
			return node;
		}
	}
}

Node *relational(){
	Node *node = add();
	for(;;){
		if(consume("<")){
			node = new_node(ND_LESS, node, add());
		}else if(consume("<=")){
			node = new_node(ND_LESSEQ, node, add());
		}else if(consume(">")){
			node = new_node(ND_LESS, add(), node);
		}else if(consume(">=")){
			node = new_node(ND_LESSEQ, add(), node);
		}else{
			return node;
		}
	}
}

Node *add(){
	Node *node = mul();
	for(;;){
		if(consume("+")){
			node = new_node(ND_ADD, node, mul());
		}else if(consume("-")){
			node = new_node(ND_SUB, node, mul());
		}else{
			return node;
		}
	}
}

Node *mul(){
	Node *node = unary();
	for(;;){
		if(consume("*")){
			node = new_node(ND_MUL, node, unary());
		}else if(consume("/")){
			node = new_node(ND_DIV, node, unary());
		}else{
			return node;
		}
	}
}

Node *unary(){
	if(consume("+")){
		return primary();
	}
	if(consume("-")){
		return new_node(ND_SUB, new_node_num(0), primary());
	}
	return primary();
}

Node *primary(){
	if(consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}
	return new_node_num(expect_number());
}

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