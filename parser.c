/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
/*
 * This file defines the parsing process, build the AST, from token stream
 * I use the top-bottom way to analyze the program, because it's simple and
 * strong enough to handle my language's grammar 
 * */

#include "util.h"
#include "lex.h"
#include "parser.h"
#include "posix.h"

/*the type of next token*/
#define next_type (stream[tok_index].type)

#define tok_is_operator(t) (t == TOKEN_OPR_ADD || t == TOKEN_OPR_SUB \
		|| t == TOKEN_OPR_MUL || t == TOKEN_OPR_DIV || t == TOKEN_PAREN_L\
		|| t == TOKEN_PAREN_R)

#define tok_is_typeid(t) (t == TOKEN_KEY_VOID || t == TOKEN_KEY_INT)

#define cur_fun (parse_pro->funs[parse_pro->funlen])

/*judge whether the token type 't' may appear after the expression
 * this macro is used to juge the ending condtion of expression */
#define is_after_expr(t) ( t == TOKEN_KEY_COLON || t == TOKEN_KEY_COMMA\
		|| t == TOKEN_CMP_EQ ||	t == TOKEN_CMP_NE || t == TOKEN_CMP_LS ||\
		t == TOKEN_CMP_GT)

token *stream;
token cur_tok;
int tok_index = 0, token_num;
program_t *parse_pro;
char error_message[100];


char *get_errMsg(char *info){
	sprintf(error_message, "Line %d: %s\n", cur_tok.line, info);
	return error_message;
}

int has_token(){
	return tok_index < token_num;
}

token get_token(){
	exit_on_error( !has_token(), "no more token");
	return	cur_tok = stream[tok_index++];
}

double parse_num(){
	return atof(cur_tok.str);
}




expr_node *get_expr_node(){
	expr_node *p = (expr_node *) malloc(sizeof(expr_node));
	memset(p, 0, sizeof(expr_node));
	p->type = AST_EXPR_NODE;
	p->reg_num = 1;
	return p;
}

ast_opr_t opr_trans(int v){
	switch(v){
		case TOKEN_OPR_ADD:
			return AST_OPR_ADD;
		case TOKEN_OPR_SUB:
			return AST_OPR_SUB;
		case TOKEN_OPR_MUL:
			return AST_OPR_MUL;
		case TOKEN_OPR_DIV:
			return AST_OPR_DIV;
		case TOKEN_PAREN_L:
			return AST_OPR_PAREN_L;
		case TOKEN_PAREN_R:
			return AST_OPR_PAREN_R;
		default:
			return AST_OPR_NONE;
	}
}


/*
 * Parse the expression syntax, this may need some explanation.
 * To emit the code of computing expression, I use the eshov tree, descripted
 * Dragon Book. If I treat the expression formed by factors(a*b*c, (expr) ), and
 * expr(factor + factor - factor), the data strcutre would be a linked list.
 * But using posix expression analysis with stack, it formed the AST tree in parsing 
 * process. And the number of register each expression node need could be 
 * computed in this process.
 * 
 * To simplify the code, I divide it into two parts: parsing module and posix 
 * expr module . I put the posix/stack in another module, posix.c.
 * The posix_info contains the stack, operator stack and status information.
 * */
expr_node *parse_expr(){
	expr_node *root = NULL, *tnode;
	posix_info *info = posix_init();

	while(!is_after_expr(next_type) && 
			!(next_type == TOKEN_PAREN_R && !posix_exit_lparen(info))){
		int ntype = next_type;
		if (tok_is_operator(ntype)){
			get_token(); 
			exit_on_error( !posix_operator_in(info, opr_trans(cur_tok.type)),
					get_errMsg("error in parse expr") );
			continue;
		}
		tnode = get_expr_node();
		switch (ntype){
			/*if next type is TOKEN_ID, it may be a variable or function call*/
			case TOKEN_ID:
				if (stream[tok_index+1].type == TOKEN_PAREN_L){
					tnode->type = AST_STMT_CALL;
					stmt_t call1 = parse_call();
					tnode->call = call1.call;
				}
				else{
					get_token();
					var_t *var = get_fun_var(cur_tok.str);
					exit_on_error(!var, get_errMsg("variable must be declared first"));
					tnode->var = var;
					tnode->type = AST_VAR;
				}
				break;

			case TOKEN_NUMBER:
				get_token();
				tnode->num = parse_num();
				tnode->type = AST_NUM;
				break;

			default:
				exit_on_error(1, get_errMsg("expected operator or operand\n"));
		}
		exit_on_error(!posix_operand_in(info, tnode), get_errMsg("err in parse_expr"));
	}
	exit_on_error(!posix_expr_root(info, &root), get_errMsg("err1 in parse_expr"));
	posix_del(info);
	return root;
}


void add_fun_var(){
	int len = cur_fun.varlen, i;
	/*check the existence of duplicated named variable*/
	for ( i = 0; i < len; i++)
		exit_on_error(strcmp(cur_tok.str, cur_fun.varlist[i].name) == 0,
			get_errMsg("duplicated names of variable"));
	cur_fun.varlist[len].type = TYPE_INT;
	cur_fun.varlist[len].name = cur_tok.str;
	cur_fun.varlen++;
}

var_t *get_fun_var(char *name){
	int i =0;
	for ( ; i < cur_fun.varlen; i++)
		if (strcmp(name, cur_fun.varlist[i].name) == 0)
			return &cur_fun.varlist[i];
	return NULL;
}

/*parsing variable's declaration stmt*/
void parse_var_decl(){
	get_token();
	exit_on_error(cur_tok.type != TOKEN_KEY_INT, get_errMsg("expect type identifier"));
	while(has_token() && next_type != TOKEN_KEY_COLON){
		get_token();
		exit_on_error(cur_tok.type != TOKEN_ID, 
				get_errMsg("expect identifier invar declaration"));
		add_fun_var();
		exit_on_error(!has_token() || (next_type != TOKEN_KEY_COMMA &&
					next_type != TOKEN_KEY_COLON), get_errMsg("expect , or ; after identifer"));
		if (next_type == TOKEN_KEY_COMMA)
			get_token();
	}
	int error = !has_token();
	if (has_token()){
		get_token();
		error |= cur_tok.type != TOKEN_KEY_COLON;
	}	
	exit_on_error(error, get_errMsg("expected colon at the end of line"));
}


var_type get_vartype(enum TOKEN_TYPE t){
	switch(t){
		case TOKEN_KEY_INT:
			return TYPE_INT;
		case TOKEN_KEY_CHAR:
			return TYPE_CHAR;
		case TOKEN_KEY_VOID:
			return TYPE_VOID;
		default:
			return 0;
	}
}

void parse_proto(){
	exit_on_error(!tok_is_typeid(next_type),
		get_errMsg("expected type identifier at the start of function\n"));
	get_token();
	cur_fun.ret_type = get_vartype(cur_tok.type);
	exit_on_error(next_type != TOKEN_ID, get_errMsg("expect fun_name"));
	get_token();
	cur_fun.name = cur_tok.str;
	exit_on_error(next_type != TOKEN_PAREN_L, get_errMsg("expect (, prototype"));
	get_token();

	int len = 0;
	while(tok_is_typeid(next_type)){
		get_token();
		cur_fun.varlist[len].type = get_vartype(cur_tok.type);
		exit_on_error(next_type != TOKEN_ID, get_errMsg("expect id, arglist"));
		get_token();
		cur_fun.varlist[len].name = cur_tok.str;
		if (next_type == TOKEN_KEY_COMMA)
			get_token();
		len++;
	}
	cur_fun.arglen = cur_fun.varlen = len;
	exit_on_error(next_type != TOKEN_PAREN_R, get_errMsg("expect ), prototype"));
	get_token();
}

stmt_t parse_assign(){
	stmt_t stmt;
	get_token();
	exit_on_error(!(stmt.assign.var = get_fun_var(cur_tok.str)),
			get_errMsg("variable must be declared first"));
	stmt.type = AST_STMT_ASSIGN;
	get_token(); //consume '='
	exit_on_error(!(stmt.assign.exp = parse_expr()), get_errMsg("error in parse assign\n"));
	exit_on_error(next_type != TOKEN_KEY_COLON, get_errMsg("expect colon to end stmt\n"));
	get_token(); //consume ';'
	return stmt;
}

stmt_t parse_ret(){
	stmt_t stmt;
	get_token();
	stmt.type = AST_STMT_RET;
	stmt.ret.exp = parse_expr();
	get_token(); //consume ';'
	return stmt;
}

/*check the function call matching with its prototype
 * because the data type is totally integer, we just check
 * parameter number*/
int call_check(stmt_call call){
	if (call.arglen != call.fun->arglen)
		return 0;
	return 1;
}

stmt_t parse_call(){
	stmt_t stmt;
	stmt.type = AST_STMT_CALL;
	stmt.call.arglen = 0;
	get_token(); // function name;
	int i = 0;
	for ( ; i < parse_pro->funlen && 
			strcmp(cur_tok.str, parse_pro->funs[i].name); i++);
	exit_on_error( i == parse_pro->funlen, 
			get_errMsg("unfound definition of the function called\n"));
	stmt.call.fun = &parse_pro->funs[i];
	exit_on_error( next_type != TOKEN_PAREN_L, get_errMsg("expected ( for function call\n"));
	get_token();//consume (
	while (has_token() && next_type != TOKEN_PAREN_R){
		expr_node *e = parse_expr();
		stmt.call.args[stmt.call.arglen++] = e;
		if (next_type == TOKEN_KEY_COMMA)
			get_token();
	}
	exit_on_error(next_type != TOKEN_PAREN_R, get_errMsg("expected ) for function\n"));
	get_token();// consume )
	exit_on_error(!call_check(stmt.call), get_errMsg("function calling unmatches prototype\n"));
	return stmt;
}

stmt_t parse_if(){
	stmt_t stmt;
	stmt.type = AST_STMT_IF;
	stmt.cond.len = 0;
	get_token();//if
	exit_on_error(next_type != TOKEN_PAREN_L, get_errMsg("expect ( after if stmt\n"));
	get_token();
	stmt.cond.left = parse_expr();
	get_token();//< > == !=
	switch (cur_tok.type){
		case TOKEN_CMP_EQ:
			stmt.cond.cmp = AST_OPR_EQ;
			break;
		case TOKEN_CMP_NE:
			stmt.cond.cmp = AST_OPR_NE;
			break;
		case TOKEN_CMP_LS:
			stmt.cond.cmp = AST_OPR_LS;
			break;
		case TOKEN_CMP_GT:
			stmt.cond.cmp = AST_OPR_GT;
			break;
		default:
			exit_on_error(1, get_errMsg("unrecognized comparing operator\n"));
	}
	stmt.cond.right = parse_expr();
	exit_on_error(next_type != TOKEN_PAREN_R, get_errMsg("expect ) in if stmt to end condition\n"));
	get_token();
	exit_on_error(next_type != TOKEN_BRACE_L, get_errMsg("expect { to enclose if stmt's body\n"));
	get_token();
	while (has_token() && next_type != TOKEN_BRACE_R){
		stmt_t *sptr = (stmt_t *) malloc(sizeof(stmt_t));
		*sptr = parse_stmt();
		stmt.cond.stmts[stmt.cond.len++] = sptr;
	}
	exit_on_error(next_type != TOKEN_BRACE_R, get_errMsg("expect } to end if stmt's body\n"));
	get_token();
	return stmt;
}

stmt_t parse_stmt(){
	stmt_t stmt;
	stmt.type == AST_STMT_NONE;

	/*variable declaration*/
	if (tok_is_typeid(next_type))
		parse_var_decl();
	else if (next_type == TOKEN_KEY_RETURN)
		stmt = parse_ret();

	/*assignment stmt*/
	else if(next_type == TOKEN_ID && stream[tok_index+1].type == TOKEN_OPR_ASSIGN)
		stmt = parse_assign();

	/*function call*/
	else if(next_type == TOKEN_ID && stream[tok_index+1].type == TOKEN_PAREN_L){
		stmt = parse_call();
		get_token();
		exit_on_error(cur_tok.type != TOKEN_KEY_COLON, get_errMsg("expected colon to end call stmt\n"));
	}
	else if (next_type == TOKEN_KEY_IF)
		stmt = parse_if();
	else{
		printf("%s\n", cur_tok.str);
		exit_on_error(1, get_errMsg("unrecognized stmt\n"));
	}
	return stmt;
}

void parse_fun(){
	cur_fun.arglen = cur_fun.stmtlen = 0;
	parse_proto();
	exit_on_error(next_type != TOKEN_BRACE_L, get_errMsg("expected\
				{ to start function"));
	get_token();
	while(has_token() && next_type != TOKEN_BRACE_R){
		stmt_t stmt = parse_stmt();
		if (stmt.type != AST_STMT_NONE)
			cur_fun.stmts[cur_fun.stmtlen++] = stmt;
	}
	exit_on_error(next_type != TOKEN_BRACE_R, get_errMsg("expected } to end function"));
	get_token();
}

void parse_init(program_t *pro){
	pro->funlen = 0;
	parse_pro = pro;
}

/*
 * Put the build-in io function(read, write) declaration into program
 * */
void put_io(){
	parse_pro->funs[0].arglen = 0;
	parse_pro->funs[0].name = "read";
	parse_pro->funs[1].arglen = 1;
	parse_pro->funs[1].name = "write";
	parse_pro->funlen = 2;
}



void parse_program(){
	int i;
	put_io();
	while(has_token()){
		parse_fun();
		for ( i = 0; i < parse_pro->funlen; i++)
			exit_on_error( strcmp(cur_fun.name, parse_pro->funs[i].name) == 0, 
					"two function could not share the same name\n");
		parse_pro->funlen++;
	}
	for ( i = 0; i < parse_pro->funlen; i++)
		if ( strcmp(parse_pro->funs[i].name, "main") == 0)
			break;
	exit_on_error(i == parse_pro->funlen, "main function not found\n");
	exit_on_error(parse_pro->funs[i].arglen != 0, 
			"main function should not contain any parameters\n");
}
