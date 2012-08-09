/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
/*
 * This file define how to convert emitted code data structure to 
 * assembly code, in ascii string, for the assembly compiler
 */

#include "dump.h"
#include "ins.h"

const char *ins_name[] = { 
	[INS_MOV] = "mov", 
	[INS_ADD] = "add",
	[INS_SUB] = "sub",
	[INS_MUL] = "imul",
	[INS_DIV] = "div",
	[INS_CMP] = "cmp",
	[INS_RET] = "ret",
	[INS_JMP] = "jmp",
	[INS_JE]  = "je",
	[INS_JNE] = "jne",
	[INS_JG]  = "jg",
	[INS_JL]  = "jl",
	[INS_CALL] = "call",
	[INS_INT] = "int",
	[INS_PUSH] = "push",
	[INS_POP] = "pop",
	[INS_LEAVE] = "leave"
};

const char *reg_name[] = {
	[REG_EAX] = "eax",
	[REG_EBX] = "ebx",
	[REG_ECX] = "ecx",
	[REG_EDX] = "edx",
	[REG_ESI] = "esi",
	[REG_EDI] = "edi",
	[REG_ESP] = "esp",
	[REG_EBP] = "ebp"
};


char *dump_pool_init(int len){
	char *pool = (char *) malloc( sizeof(char) * len);
	memset(pool, 0, sizeof(char) * len);
}

void dump_opd(char *pool, opd_t opd){
	switch(opd.type){
		case OPD_IMM:
			dump_opd_imm(pool, opd);
			break;
		case OPD_VAR:
			dump_opd_var(pool, opd);
			break;
		case OPD_REG:
			dump_opd_reg(pool, opd);
			break;
		case OPD_IDX:
			dump_opd_idx(pool, opd);
			break;
		case OPD_IND:
			dump_opd_ind(pool, opd);
			break;
		default:
			;
	}
}

void dump_opd_imm(char *pool, opd_t opd){
	sprintf(pool, "%s$%d", pool, opd.imm);
}

void dump_opd_reg(char *pool, opd_t opd){
	sprintf(pool, "%s%%%s", pool, reg_name[opd.reg]);
}

void dump_opd_var(char *pool, opd_t opd){
	sprintf(pool, "%s%d(%%ebp)", pool, opd.var.pos);
}

void dump_opd_idx(char *pool, opd_t opd){
}

void dump_opd_ind(char *pool, opd_t opd){
}


void dump_ins_opcode(char *pool, ins_t inst){
	sprintf(pool, "%s%s", pool, ins_name[inst.opcode]);
}

/*dump assembly text for instruction
 * I haven't add the operand length suffix 'l' for instruction
 * */
void dump_ins(char *pool, ins_t inst){
	dump_ins_opcode(pool, inst);
	if (inst.opcode == INS_CALL)
		sprintf(pool, "%s %s", pool, inst.call);
	else if (inst.opcode == INS_JMP || inst.opcode == INS_JE || inst.opcode == INS_JNE ||
			inst.opcode == INS_JG || inst.opcode == INS_JL){
		sprintf(pool, "%s L%d", pool, inst.lopd.imm);
	}
	else if (inst.opd_num == 1){
		sprintf(pool, "%s ", pool);
		dump_opd(pool, inst.lopd); 
	}
	else if (inst.opd_num == 2){
		sprintf(pool, "%s ", pool);
		dump_opd(pool, inst.lopd); 
		sprintf(pool, "%s, ", pool);
		dump_opd(pool, inst.ropd);
	}
	sprintf(pool, "%s\n", pool);
}

void dump_program(char *pool, emit_context *context){
	int i = 0, j = 0;
	for ( ; i < context->inslen; i++){
		for ( ; j < context->dirlen && context->dirct[j].pos <= i; j++)
			if (context->dirct[j].lid != -1)/*lid != -1 means it is a label*/
				sprintf(pool, "%sL%d:\n", pool, context->dirct[j].lid);
			else /*lid == -1: it's a directive, just sprintf it into string*/
				sprintf(pool, "%s%s", pool, context->dirct[j].name);
		dump_ins(pool, context->ins[i]);
	}
}

