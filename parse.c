#include "9cc.h"
Node *code[100];
LVar *locals;
int lvar_max = 0;

LVar *find_lvar(Token *tok){
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
			return var;
		}
	}
	return NULL;
}

void error(char *s){
	fprintf(stderr, "%s", s);
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

Token *consume_kind(TokenKind kind){
	if(token->kind != kind ){
		return NULL;
	}
	Token *ident_token = token;
	token = token->next;
	return ident_token;
}	

void expect(char *op){
	if( strlen(op) != token->len || memcmp(token->str, op, token->len) ){
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

Type *new_type(int ty, Type *ptr_to){
	Type *type = calloc(1, sizeof(Type));
	type->ty = ty;
	type->ptr_to = ptr_to;
	return type;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	if(lhs){
		node->type = lhs->type;
	}
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	node->type = new_type(INT, NULL);
	return node;
}

Node *new_node_LVar(Token *tok){
	Node *node = new_node(ND_LVAR, NULL, NULL);
	LVar *lvar = find_lvar(tok);
	if(lvar){
		node->offset = lvar->offset;
		node->type = lvar->type;
	}else{	
		lvar = calloc(1, sizeof(LVar));
		lvar->next = locals;
		lvar->name = tok->str;
		lvar->len = tok->len;
		lvar->type = tok->type;
		if(locals == NULL){
			lvar->offset = 0;
			lvar_max = 1;
		}else{
			lvar->offset = locals->offset + 1;
			lvar_max += 1;
		}
		node->offset = lvar->offset;
		node->type = lvar->type;
		locals = lvar;
	}
	return node;
}

void program(){
	int i=0;
	int arg_num;
	Token *tk;
	while(!at_eof()){
		locals = NULL;
		arg_num = 0;
		expect("int");
		Token *function_name = consume_kind(TK_IDENT);
		Node *node = new_node(ND_FUNCDEF, NULL, NULL);
		node->name = function_name->str;
		node->len = function_name->len;
		node->type = new_type(INT, NULL);
		expect("(");
		if( !consume(")") ){
			expect("int");
			tk = consume_kind( TK_IDENT );
			tk->type = new_type(INT, NULL);
			node->statements_pointer[ arg_num++ ] = new_node_LVar( tk );
			while( consume(",") ){
				expect("int");
				tk = consume_kind( TK_IDENT );
				tk->type = new_type(INT, NULL);
				node->statements_pointer[ arg_num++ ] = new_node_LVar( tk );
			}
			consume(")");
		}
		node->val = arg_num;
		code[i++] = node;

		expect("{");
		while( !consume("}") ){
			code[i++] = stmt();
		}
		code[i++] = new_node(ND_FUNCEND, NULL, NULL);
	}
	code[i++] = NULL;
}

Node *stmt(){
	Node *node;
	if(consume_kind(TK_RETURN)){
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}else if(consume_kind(TK_IF)){
		node = calloc(1, sizeof(Node));
		node->kind = ND_IF;
		node->rhs = calloc(1, sizeof(Node));//if文のブロック内のステートメントを指すためのnode
	
		expect("(");
		node->lhs = expr();//条件文
		expect(")");
		node->rhs->lhs = stmt();//turuの時に実行されるステートメント
		if(consume_kind(TK_ELSE)){
			node->rhs->rhs = stmt();//falseの時に実行されるステートメント
			node->kind = ND_IF_ELSE;
		}
		return node;
	}else if(consume_kind(TK_WHILE)){
		node = calloc(1, sizeof(Node));
		node->kind = ND_WHILE;
		expect("(");
		node->lhs = expr();//条件式
		expect(")");
		node->rhs = stmt();//whileのブロック内の式
		return node;
	}else if(consume_kind(TK_FOR)){
		node = calloc(1, sizeof(Node));
		node->kind = ND_FOR;
		node->lhs = calloc(1, sizeof(Node));//for文の初期化式とループごとに実行される式を指す。
		node->rhs = calloc(1, sizeof(Node));//for文のブロック内のステートメントとループ条件式の2つの式を指す。

		expect("(");
		if(!consume(";")){
			node->lhs->lhs = expr();//初期化の式
		}
		expect(";");
		if(!consume(";")){
			node->rhs->lhs = expr();//ループを継続するかの判定式
		}
		expect(";");
		if(!consume(")")){
			node->lhs->rhs = expr();//ループ終了時に実行される式
		}
		expect(")");
		node->rhs->rhs = stmt();//ループ内のステートメント
		return node;
	}else if(consume("{")){
		int element_num = 0;
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_BLOCK;
		node->statements_pointer[0] = stmt();
		while(!consume("}")){
			element_num += 1;
			node->statements_pointer[element_num] = stmt();
		}
		node->statements_pointer[element_num+1] = NULL;
		return node;
	}else if(consume_kind(TK_INT)){
		Type *type = new_type( INT, NULL );
		while( consume("*") ){
			type = new_type( PTR, type);
		}
		Token *tk = consume_kind(TK_IDENT);
		tk->type = type;
		node = new_node_LVar(tk);
	}else{
		node = expr();
	}
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
	Node *right_value;
	for(;;){
		if(consume("+")){
			right_value = mul();
			if( node->type->ty == PTR && right_value->type->ty == INT){
				right_value->val = right_value->val*4;
			}
			if( node->type->ty == INT && right_value->type->ty == PTR){
				node->val = node->val*4;
			}
			node = new_node(ND_ADD, node, right_value);
		}else if(consume("-")){
			right_value = mul();
			if( node->type->ty == PTR && right_value->type->ty == INT){
				right_value->val = right_value->val*4;
			}
			if( node->type->ty == INT && right_value->type->ty == PTR){
				node->val = node->val*4;
			}
			node = new_node(ND_SUB, node, right_value);
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
	if(consume("*")){
		return new_node(ND_DEREF, unary(), NULL);
	}
	if(consume("&")){
		return new_node(ND_ADDR, unary(), NULL);
	}
	if(consume_kind( TK_SIZEOF )){
		Node *node = unary();
		switch(node->type->ty){
			case INT: return new_node_num(4);
			case PTR: return new_node_num(4);
		}
	}
	return primary();
}

Node *primary(){
	if(consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}
	Token *tok = consume_kind(TK_IDENT);
	if(tok){
		if(consume("(")){
			int i=0;
			Node *node = new_node(ND_FUNCCALL, NULL, NULL);
			node->name = tok->str;
			node->len = tok->len;
			node->type = new_type(INT, NULL);
			if(!consume(")")){
				node->statements_pointer[0] = expr();
				while(consume(",")){
					i += 1;
					node->statements_pointer[i] = expr();
				}
				node->val = i+1;
				expect(")");
				return node;
			}else{
				node->val = 0;
				return node;
			}	
		}
		if(!find_lvar(tok)){
			error_at( tok->str, "定義されていない変数です\n" );
		}
		Node *node = new_node_LVar(tok);
		return node;
	}

	return new_node_num(expect_number());
}
//