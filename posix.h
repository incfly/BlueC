/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef POSIX_H
#define POSIX_H
#include "parser.h"

typedef struct posix_info{
	/*先用静态数组,以后用动态*/
	ast_opr_t opr_stack[100];
	expr_node *expr_stack[100];

	/*top指示当前栈顶元素,添加元素的时候
	 * stack[++top] = elem*/
	int opr_top;
	int expr_top;
	int status; /*0--wrong, 1 correct*/
} posix_info;


extern int posix_expr_root(posix_info *info, expr_node **root_addr);
extern int posix_operator_in(posix_info *info, ast_opr_t opr);
extern int posix_operand_in(posix_info *info, expr_node *node);
extern int posix_del(posix_info *info);
extern posix_info *posix_init();
extern void posix_print(expr_node *root,int);/*打印出expr tree,用于调试*/
extern int posix_exit_lparen(posix_info *);//判断是否存在)

#endif
