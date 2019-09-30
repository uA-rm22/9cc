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

Type *new_type(int ty, Type *ptr_to, size_t size){
	Type *type = calloc(1, sizeof(Type));
	type->ty = ty;
	type->ptr_to = ptr_to;
	type->size = size;
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
	node->type = new_type(INT, NULL, 4);
	return node;
}

Node *new_node_LVar(Token *tok, Type *type){
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
		lvar->type = type;
		if(locals == NULL){
			lvar_max = type->size;
			lvar->offset = lvar_max;
		}else{
			lvar->offset = locals->offset + lvar->type->size;
			lvar_max += type->size;
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
		lvar_max = 0;
		locals = NULL;
		arg_num = 0;
		expect("int");
		Token *function_name = consume_kind(TK_IDENT);
		Node *node = new_node(ND_FUNCDEF, NULL, NULL);
		node->name = function_name->str;
		node->len = function_name->len;
		node->type = new_type(INT, NULL, 4);
		expect("(");
		if( !consume(")") ){
			expect("int");
			tk = consume_kind( TK_IDENT );
			node->statements_pointer[ arg_num++ ] = new_node_LVar( tk, new_type(INT, NULL, 4) );
			while( consume(",") ){
				expect("int");
				tk = consume_kind( TK_IDENT );
				node->statements_pointer[ arg_num++ ] = new_node_LVar( tk, new_type(INT, NULL, 4) );
			}
			expect(")");
		}
		node->val = arg_num;
		code[i++] = node;

		expect("{");
		while( !consume("}") ){
			code[i++] = stmt();
		}
		code[i++] = new_node(ND_FUNCEND, NULL, NULL);
		if(lvar_max % 8 != 0){
			lvar_max = (lvar_max / 8 + 1 ) * 8;
		}
		node->offset = lvar_max;
	}
	code[i++] = NULL;
}

Node *stmt(){
	Node *node;
	if(consume_kind(TK_RETURN)){
		node = new_node( ND_RETURN, expr(), NULL );
	}else if(consume_kind(TK_IF)){	
		expect("(");
		node = new_node(ND_IF, expr(), NULL);//条件文
		expect(")");
		node->rhs = new_node(ND_IF, stmt(), NULL);;//turuの時に実行されるステートメント
		if(consume_kind(TK_ELSE)){
			node->rhs->rhs = stmt();//falseの時に実行されるステートメント
			node->kind = ND_IF_ELSE;
		}
		return node;
	}else if(consume_kind(TK_WHILE)){
		expect("(");
		node = new_node(ND_WHILE, expr(), NULL);//条件式
		expect(")");
		node->rhs = stmt();//whileのブロック内の式
		return node;
	}else if(consume_kind(TK_FOR)){
		node = new_node(ND_FOR, new_node(ND_FOR,NULL,NULL), new_node(ND_FOR,NULL,NULL));
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
		int element_num = 1;
		Node *node = new_node(ND_BLOCK, NULL, NULL);
		node->statements_pointer[0] = stmt();
		while(!consume("}")){
			node->statements_pointer[element_num++] = stmt();
		}
		node->statements_pointer[element_num++] = NULL;
		return node;
	}else if(consume_kind(TK_INT)){
		int ele_num=0;
		Type *type = new_type( INT, NULL, 4 );
		while( consume("*") ){
			type = new_type( PTR, type, 4);
		}
		Token *tk = consume_kind(TK_IDENT);
		if( consume("[") ){
			ele_num = expect_number();
			expect("]");
			type = new_type(ARRAY, type, ele_num*4);
		}
		node = new_node_LVar(tk, type);
		node->kind = ND_LVARDEF;
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
			if( (node->type->ty == PTR || node->type->ty == ARRAY) && right_value->type->ty == INT){
				right_value = new_node( ND_MUL, right_value, new_node_num(4) );
			}
			if( node->type->ty == INT && ( right_value->type->ty == PTR || right_value->type->ty == ARRAY)){
				node = new_node( ND_MUL, node, new_node_num(4) );
			}
			node = new_node(ND_ADD, node, right_value);
		}else if(consume("-")){
			right_value = mul();
			if( (node->type->ty == PTR || node->type->ty == ARRAY) && right_value->type->ty == INT){
				right_value = new_node( ND_MUL, right_value, new_node_num(4) );
			}
			if( node->type->ty == INT && ( right_value->type->ty == PTR || right_value->type->ty == ARRAY) ){
				node = new_node( ND_MUL, node, new_node_num(4) );
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
	Node *node;
	if(consume("+")){
		return primary();
	}
	if(consume("-")){
		return new_node(ND_SUB, new_node_num(0), primary());
	}
	if(consume("*")){
		node = new_node(ND_DEREF, unary(), NULL);
		node->type = new_type( INT, NULL, 4);
		return node;
	}
	if(consume("&")){
		node = new_node(ND_ADDR, unary(), NULL);
		node->type = new_type( PTR, node->type, 4);
		return node;
	}
	if(consume_kind( TK_SIZEOF )){
		node = unary();
		switch(node->type->ty){
			case INT: return new_node_num(4);
			case PTR: return new_node_num(4);
			case ARRAY: return new_node_num(4*node->type->size);
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
			int i=1;
			Node *node = new_node(ND_FUNCCALL, NULL, NULL);
			node->name = tok->str;
			node->len = tok->len;
			node->type = new_type(INT, NULL, 4);
			if(!consume(")")){
				node->statements_pointer[0] = expr();
				while(consume(",")){
					node->statements_pointer[i++] = expr();
				}
				node->val = i;
				expect(")");
			}else{
				node->val = 0;
			}	
			return node;
		}
		if(!find_lvar(tok)){
			error_at( tok->str, "定義されていない変数です\n" );
		}
		if(consume("[")){
			Node *left_value = new_node_LVar(tok, NULL);
			Node *right_value = add();
			expect("]");
			return new_node( ND_DEREF, new_node( ND_ADD, left_value, right_value ), NULL );
		}
		Node *node = new_node_LVar(tok, NULL);
		return node;
	}

	int number = expect_number();
	if(consume("[")){
		Node *left_value = new_node_num(number);
		Node *right_value = add();
		expect("]");
		return new_node( ND_DEREF, new_node( ND_ADD, left_value, right_value ), NULL );
	}
	return new_node_num(number);
}
//