/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#include "util.h"
#include "parser.h"
#include "posix.h"

int get_opr_prior(ast_opr_t opr){
	if (opr == AST_OPR_ADD || opr == AST_OPR_SUB)
		return 1;
	else if (opr == AST_OPR_MUL || opr == AST_OPR_DIV)
		return 2;
	else if (opr == AST_OPR_PAREN_L)
		return 3;
	return -1;
}


posix_info *posix_init(){
	posix_info * info = (posix_info *) malloc(sizeof(posix_info));
	memset(info, 0, sizeof(info));
	info->expr_top = -1;
	info->opr_top = -1;
	info->status = 1;
	return info;
}


int posix_del(posix_info *info){
	free(info);
	return 1;
}

int posix_operand_in(posix_info *info, expr_node *node){
	info->expr_stack[++info->expr_top] = node;
}

/* combine the top two operands in stack together with top opeator in
 * opr_stack
 * */
int posix_combine(posix_info *info){
	if (info->expr_top < 1 || info->opr_top < 0){
		info->status = 0;
		return 0;
	}
	expr_node *parent = get_expr_node();
	parent->right = info->expr_stack[info->expr_top--];
	parent->left = info->expr_stack[info->expr_top--];

	/*when combine the top two operands in the stack, compute the eshov value*/
	int lrn = parent->left->reg_num, rrn = parent->right->reg_num;
	parent->reg_num = lrn == rrn ? lrn + 1 : max(lrn, rrn);

	parent->opr = info->opr_stack[info->opr_top--];
	info->expr_stack[++info->expr_top] = parent;
	return 1;
}

/*add a operator into the opeartor stack*/
int posix_operator_in(posix_info *info, ast_opr_t opr){
	/*if the stack is empty or at the top of the stack is '('
	 * put the new operator into stack directly*/
	if (info->opr_top == -1 || opr == AST_OPR_PAREN_L){
		info->opr_stack[++info->opr_top] = opr;
		return 1;
	}
	/*when operator is ')', unstack the element until a '(' is found*/
	if (opr == AST_OPR_PAREN_R){
		while (info->opr_top >= 0 && info->opr_stack[info->opr_top] != AST_OPR_PAREN_L)
			if (!posix_combine(info))
				return info->status = 0;

		if (info->opr_top < 0)
			return info->status = 0;
		info->opr_top--;
		return 1;
	}
	int prev, cur = get_opr_prior(opr);

	while (info->opr_top >= 0 && get_opr_prior(info->opr_stack[info->opr_top]) >= cur 
			&& info->opr_stack[info->opr_top] != AST_OPR_PAREN_L ){
		if (!posix_combine(info))
			return 0;
	}
	info->opr_stack[++info->opr_top] = opr;
	return 1;
}

int posix_expr_root(posix_info *info, expr_node **root_addr){
	/*combine the left operands, check the validity of expression*/
	while( posix_combine(info))
		;

	/*check: opr_stack should be empty ,expr_stack should just left
	 * only one operand, the root expr node*/
	if ( info->opr_top != -1 || info->expr_top != 0){
		info->status = 0;
		return 0;
	}
	*root_addr = info->expr_stack[info->expr_top--];
	return 1;
}

void posix_print(expr_node *root, int depth){
	if (!root->left && !root->right){
		if (root->type == AST_NUM)
			printf("%dth %d\n", depth, root->num);
		else if (root->type == AST_VAR)
			printf("%dth %s\n", depth, root->var->name);
	}
	else {
		posix_print(root->left, depth+1);
		printf("%dth opr %d\n", depth, root->opr);
		posix_print(root->right, depth+1);
	}
}

/*check the existence of left parentheses in operator stack */
int posix_exit_lparen(posix_info *info){
	int i = 0;
	for ( ; i <= info->opr_top; i++)
		if (info->opr_stack[i] == AST_OPR_PAREN_L)
			return 1;
	return 0;
}

