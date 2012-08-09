/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef PARSER_H
#define PARSER_H
#include "lex.h"

typedef struct stmt_call stmt_call;
typedef struct var_t var_t;
typedef struct fun_t fun_t;
typedef struct expr_node expr_node;
typedef struct stmt_t stmt_t;

/*the type of each kind of abstract syntax element*/
typedef enum ast_type_t{
	AST_STMT_ASSIGN = 1,
	AST_STMT_CALL,
	AST_STMT_RET,
	AST_STMT_IF,
	AST_STMT_NONE,
	AST_NUM,
	AST_VAR,
	AST_EXPR_NODE
} ast_type_t;

/*the type of operators*/
typedef enum ast_opr_t{
	AST_OPR_ADD,
	AST_OPR_SUB,
	AST_OPR_MUL,
	AST_OPR_DIV,
	AST_OPR_PAREN_L ,
	AST_OPR_PAREN_R,
	AST_OPR_EQ,// == 
	AST_OPR_NE,// !=
	AST_OPR_LS,// <
	AST_OPR_GT,// >
	AST_OPR_NONE
} ast_opr_t;

struct stmt_call{
	fun_t *fun;
	expr_node *args[10];
	int arglen;
};

/*the expression node, formed the expression tree*/
struct expr_node{
	struct expr_node *left;
	struct expr_node *right;
	ast_opr_t opr;

	ast_type_t type;
	int num;
	var_t *var;
	stmt_call call;

	/*the number of needed register generating
	 * assembly code for this expr node in eshov tree*/
	int reg_num;
};

/*type of variable, not used in fact.-_-
 * I simply restrict the variable type to integer
 * */
typedef enum var_type{
	TYPE_INT,
	TYPE_CHAR,
	TYPE_VOID,
	TYPE_ARRAY
} var_type;

struct var_t{
	var_type type;

	/*the offset displacement relative to ebp register,
	 * used to locate variable in function's stack, such as : pos(%ebp)
	 * I admitted, this maybe not good design pattern, have relation with
	 * another module. :-(*/
	int pos;
	char *name;
};


/*assign_stmt := var_id = expression */
typedef struct stmt_assign{
	var_t *var;
	expr_node *exp;
} stmt_assign;



typedef struct stmt_ret{
	expr_node *exp;
} stmt_ret;

typedef struct stmt_if{
	expr_node *left;
	expr_node *right;
	ast_opr_t cmp;

	/*the statements forming the body of if stmt
	 * It has to be pointer array, otherwise stmt_if and stmt_t depends
	 * on each other, nested data structure*/
	stmt_t *stmts[20];
	int len;
} stmt_if;


/*statement type*/
struct stmt_t{
	stmt_assign assign;
	stmt_call call;
	stmt_ret ret;
	stmt_if cond;
	ast_type_t type;
};


/*the function AST structure*/
struct fun_t{
	char *name;

	var_type ret_type; /*return value type*/
	var_t varlist[50]; /*the function's local variable, included parameters*/
	int varlen;
	int arglen;

	stmt_t stmts[100];
	int stmtlen;
};


typedef struct program_t{
	fun_t funs[10];
	int funlen;
} program_t;

extern token *stream;
extern program_t *parse_pro;

extern void parse_proto();
extern void parse_fun();
extern expr_node *get_expr_node();
extern expr_node *parse_expr();
extern void parse_var_decl();
extern var_t *get_fun_var(char *);
extern void parse_init(program_t *);
extern void parse_program();
extern stmt_t parse_call();
extern stmt_t parse_stmt();

#endif

