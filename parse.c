#include "9cc.h"

Token *token;
Node *code[100];

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

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'|| *p == '>' || *p == '<' || *p == ';'|| *p == '='){
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if('a' <= *p && *p <= 'z'){
			cur = new_token(TK_IDENT, cur, p++, 1);
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

Token *consume_ident(){
	if(token->kind != TK_IDENT ){
		return NULL;
	}
	Token *ident_token = token;
	token = token->next;
	return ident_token;
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

void program(){
	int i=0;
	while(!at_eof()){
		code[i++] = stmt();
	}
	code[i++] = NULL;
}

Node *stmt(){
	Node *node = expr();
	expect(";");
	return node;
}

Node *expr(){
	return assign();
}

Node *assign(){
	Node *node = equality();
	if(consume("=")){
		node = new_node(ND_ASSIGN, node, assign());	
 	}
	return node;
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
	Token *tok = consume_ident();
	if(tok){
		Node *node = new_node(ND_LVAR, NULL, NULL);
		node->offset = (tok->str[0] - 'a' + 1)*4;
		return node;
	}

	return new_node_num(expect_number());
}