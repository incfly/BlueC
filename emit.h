/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef EMIT_H
#define EMIT_H
#include "parser.h"
#include "ins.h"

/*stands for some label and directive in assembly.*/
typedef struct emit_dirct{
	char *name;

	/*identify the postion of the directive, located before the pos-th instruction */
	int pos;
	/*The identify of Label, jmp Label, if lid = -1, it's not label, just directive.
	 * such as "type fun@function", ".globl _start" */
	int lid;
} emit_dirct;

/* The emit result data structure,
 * containing the generated instruction from AST
 * */
typedef struct emit_context{
	ins_t ins[200];
	int inslen;

	emit_dirct dirct[30];
	int label_id;/*the length of current label, not dirlen*/
	int dirlen;

	/*register status*/
	int reg_used[10];
	int reg_free;/*the number of free register*/
} emit_context;

extern emit_context * emit_init();
extern reg_t alloc_reg(emit_context *);
extern reg_t emit_expr(emit_context *, expr_node *);
extern void emit_stmt(emit_context *, stmt_t );
extern void emit_program(emit_context *, program_t *);

#endif
