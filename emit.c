/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#include "emit.h"
#include "util.h"
#include "ins.h"
#include "parser.h"
#define regnum_greater_one(root) (root->left->reg_num > root->right->reg_num ?\
		root->left : root->right)
#define regnum_smaller_one(root) (root->left->reg_num <= root->right->reg_num ?\
		root->left : root->right)

emit_context * emit_init(){
	emit_context * p = (emit_context *) malloc(sizeof (emit_context) );
	memset(p, 0, sizeof(emit_context));
	p->inslen = 0;
	p->reg_free = 4;/*eax~edx*/
	p->label_id = 1;
}

void free_all_reg(emit_context *context){
	int i = 0;
	for ( ; i < 4; i++)
		context->reg_used[i] = 0;
	context->reg_free = 4;
}

int append_ins(emit_context *context, ins_t ins){
	context->ins[context->inslen++] = ins;
	return 1;
}

/*allocate a directive, isLabel identify whether it is a label*/
emit_dirct *alloc_dirct(emit_context *context, int isLabel){
	emit_dirct *ptr = &context->dirct[context->dirlen++];
	ptr->pos = context->inslen;
	ptr->lid = -1;
	if (isLabel)
		ptr->lid = context->label_id++;
	return ptr;
}

void append_dirct(emit_context *context, char *str){
	emit_dirct *pdir = alloc_dirct(context, 0);
	pdir->name = (char *)malloc(sizeof(char)
			* (1+strlen(str)));
	strcpy(pdir->name, str);
}

/*find a free register and mark it used*/
reg_t alloc_reg(emit_context *info){
	reg_t choice = REG_NONE;
	if (!info->reg_used[REG_EAX])
		choice = REG_EAX;
	else if (!info->reg_used[REG_EBX])
		choice = REG_EBX;
	else if (!info->reg_used[REG_ECX])
		choice = REG_ECX;
	else if (!info->reg_used[REG_EDX])
		choice = REG_EDX;
	if (choice != REG_NONE){
		info->reg_free--;
		info->reg_used[choice] = 1;
	}
	return choice;
}

int free_reg(emit_context *context, reg_t r){
	context->reg_used[r] = 0;
	context->reg_free++;
}

ins_t get_ins_by_astopr(opd_t lopd, opd_t ropd, ast_opr_t opr){
	ins_t inst;
	switch(opr){
		case AST_OPR_ADD:
			inst = getins_add(lopd, ropd, 4);
			break;
		case AST_OPR_SUB:
			inst = getins_sub(lopd, ropd, 4);
			break;
		case AST_OPR_MUL:
			inst = getins_mul(lopd, ropd, 4);
			break;
		case AST_OPR_DIV:
			inst = getins_div(lopd, ropd, 4);
			break;
		default:
			exit_on_error(1, "wrong in emit_ins_by_astopr()");
	}
	return inst;
}

void emit_call(emit_context *context, stmt_call call){
	int i = 0;
	ins_t ins;
	reg_t reg;
	for ( ; i < call.arglen; i++){
		reg = emit_expr(context, call.args[i]);
		ins = getins_push( getopd_reg(reg), 4);
		append_ins(context, ins);
		free_reg(context, reg);
	}
	ins = getins_call(call.fun->name);
	append_ins(context, ins);
}

/*Generate assembly code for the expression root node. And return the register
 * containing the result, using the algorithm eshov tree in Dragon Book*/
reg_t emit_expr(emit_context *context, expr_node *root){
	opd_t lopd, ropd;
	ins_t ins;
	if (root->left == NULL && root->right == NULL){
		reg_t r = alloc_reg(context);
		exit_on_error(r == REG_NONE, "no avaliable reg\n");
		if (root->type == AST_NUM)
			lopd = getopd_imm(root->num);
		else if (root->type == AST_VAR)
			lopd = getopd_var(*root->var);
		else if (root->type == AST_STMT_CALL){
			emit_call(context, root->call);
			lopd = getopd_reg(REG_EAX);
		}
		ropd = getopd_reg(r);
		ins = getins_mov(lopd, ropd, 4);
		append_ins(context, ins);
		return r;
	}
	expr_node *first = regnum_greater_one(root), *second = regnum_smaller_one(root);
	reg_t fir_r, sec_r;
	fir_r = emit_expr(context, first);

	/*If the needed register number is greater than 4, save the one child node's
	 * result by pushing to stack*/
	if (root->reg_num > 4){
		lopd = getopd_reg(fir_r);
		ins = getins_push(lopd, 4);
		append_ins(context, ins);
		free_reg(context, fir_r);
	}
	sec_r = emit_expr(context, second);

	/*restore the saved child node result from stack*/
	if (root->reg_num > 4){
		fir_r = alloc_reg(context);
		ins = getins_pop( getopd_reg(fir_r), 4);
		append_ins(context, ins);
	}

	/* 7-8, 7 is left child, but I put 7 to right operand in generate code.
	 * This matches the AT&T assembly syntax. Well it may be a bad design
	 * and the code is difficult to read. Sorry >_<
	 * */
	lopd = getopd_reg(first == root->right? fir_r: sec_r);
	ropd = getopd_reg(first == root->left? fir_r : sec_r);
	ins = get_ins_by_astopr(lopd, ropd, root->opr);
	append_ins(context, ins);
	free_reg(context, lopd.reg);
	return ropd.reg;
}

void emit_assign(emit_context *context, stmt_t stmt){
	opd_t lopd, ropd;
	reg_t reg = emit_expr(context, stmt.assign.exp);
	lopd = getopd_reg(reg);
	ropd = getopd_var(*stmt.assign.var);
	ins_t ins = getins_mov(lopd, ropd, 4);
	append_ins(context, ins);
	free_reg(context, reg);/*remember to free corresponding register*/
}

/*code for ret instruction, insert a leave instruction before ret*/
void emit_ret(emit_context *context, stmt_t stmt){
	opd_t lopd, ropd;
	ins_t ins;
	reg_t reg = emit_expr(context, stmt.ret.exp);
	if (reg != REG_EAX){
		lopd = getopd_reg(reg);
		ropd = getopd_reg(REG_EAX);
		ins = getins_mov(lopd, ropd, 4);
		append_ins(context, ins);
	}
	ins = getins_leave();
	append_ins(context, ins);

	ins = getins_ret();
	append_ins(context, ins);
}


/* if statement code emission. 
 * Keep in mind in AT&T syntax, cmp b,a means a - b
 * */
void emit_if(emit_context *context, stmt_if s){
	//cmp a,b
	reg_t lreg = emit_expr(context, s.left);
	reg_t rreg = emit_expr(context, s.right);
	free_reg(context, lreg);
	free_reg(context, rreg);
	ins_t ins = getins_cmp( getopd_reg(rreg), getopd_reg(lreg), 4);
	append_ins(context, ins);
	
	//jxx bodyStart 
	emit_dirct *dirStart= alloc_dirct(context, 1);
	opd_t bodyPos = getopd_imm(dirStart->lid);
	switch(s.cmp){
		case AST_OPR_EQ:
			ins = getins_je(bodyPos, 4);
			break;
		case AST_OPR_NE:
			ins = getins_jne(bodyPos, 4);
			break;
		case AST_OPR_GT:
			ins = getins_jg(bodyPos, 4);
			break;
		case AST_OPR_LS:
			ins = getins_jl(bodyPos, 4);
			break;
		default:
			;
	}
	append_ins(context, ins);
	//jmp ends;
	emit_dirct *dirEnd = alloc_dirct(context, 1);
	opd_t bodyEnd = getopd_imm(dirEnd->lid);
	ins = getins_jmp(bodyEnd, 4);
	append_ins(context, ins);

	dirStart->pos = context->inslen;
	dirStart->name = "label:\n";
	//body Stmts
	int i = 0;
	for ( ; i < s.len; i++)
		emit_stmt(context, *s.stmts[i]);
	dirEnd->pos = context->inslen;
	dirEnd->name = "label:\n";
}


void emit_stmt(emit_context *context, stmt_t stmt){
	switch(stmt.type){
		case AST_STMT_ASSIGN:
				emit_assign(context, stmt);
				break;
			case AST_STMT_RET:
				emit_ret(context, stmt);
				break;
			case AST_STMT_CALL:
				emit_call(context, stmt.call);
				break;
			case AST_STMT_IF:
				emit_if(context, stmt.cond);
				break;
			default:
				;
	}
}

void emit_fun(emit_context *context, fun_t *fun){
	char str[200];
	sprintf(str, "\n.type %s, @function\n", fun->name);
	append_dirct(context, str);
	sprintf(str, "%s:\n", fun->name);
	append_dirct(context, str);
	int i = 0;

	ins_t ins = getins_push(getopd_reg(REG_EBP), 4);
	append_ins(context, ins);
	ins = getins_mov(getopd_reg(REG_ESP), getopd_reg(REG_EBP), 4);
	append_ins(context, ins);
	for ( ; i < fun->stmtlen; i++)
		emit_stmt(context, fun->stmts[i]);
}

/*calculate the local variable's offset relative to ebp register.*/
void emit_locate_var(fun_t *fun){
	int i = 0, para_offset = 8, var_offset = -4;
	for ( ; i < fun->arglen; i++, para_offset += 4)
		fun->varlist[i].pos = para_offset;
	for ( ; i < fun->varlen; i++, var_offset -= 4)
		fun->varlist[i].pos = var_offset;
}

/*emit code for _start label. generate code calling main() function*/
void emit_startFun(emit_context *context){
	char str[200];
	sprintf(str, "\n.globl _start\n_start:\n");
	append_dirct(context, str);
	ins_t ins = getins_call("main");
	append_ins(context, ins);
	ins =getins_mov(getopd_reg(REG_EAX), getopd_reg(REG_EBX), 4);
	append_ins(context, ins);
	ins = getins_mov(getopd_imm(1), getopd_reg(REG_EAX), 4);
	append_ins(context, ins);
	ins = getins_int(getopd_imm(128));//int $0x80. $0x80 = 128
	append_ins(context, ins);
}

void emit_program(emit_context *context, program_t *pro){
	/*start to generate the code from the third function, because read()/write()
	 * build-in io does not need to code generation*/
	int i = 2;

	emit_startFun(context);
	for ( ; i < pro->funlen; i++){
		emit_locate_var(&pro->funs[i]);
		emit_fun(context, &pro->funs[i]);
		free_all_reg(context);
	}
}
