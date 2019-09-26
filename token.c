#include "9cc.h"

int is_alnum(char c){
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
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
		if(isspace(*p) || *p == '\n'){
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

		if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
			cur = new_token(TK_RETURN, cur ,p, 6);
			p += 6;
			continue;
		}

		if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
			cur = new_token(TK_IF, cur ,p, 2);
			p += 2;
			continue;
		}

		if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
			cur = new_token(TK_ELSE, cur ,p, 4);
			p += 4;
			continue;
		}

		if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
			cur = new_token(TK_WHILE, cur ,p, 5);
			p += 5;
			continue;
		}

		if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
			cur = new_token(TK_FOR, cur ,p, 3);
			p += 3;
			continue;
		}

		if(strncmp(p, "int", 3) == 0 && !is_alnum(p[3])){
			cur = new_token(TK_INT, cur ,p, 3);
			p += 3;
			continue;
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'|| *p == '>' || *p == '<' || *p == ';'|| *p == '=' ||*p == '{'|| *p == '}' || *p == ',' || *p == '&'){
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if('a' <= *p && *p <= 'z' ){
			int len = 0;
			while(is_alnum(*p)){
				len++;
				p++;
			}
			cur = new_token(TK_IDENT, cur, p-len , len);
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