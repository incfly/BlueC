/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#include "ins.h"

/*
 * The macro of getins_xxx() function.
 * Here is a little trick: when I want to use the macro to define
 * getins_int(int, INS_INT), I found the macro parameter 'int'
 * is the keyword. :-(
 * */
#define INS_GETINS_DEFINE(name, code) \
	ins_t getins_##name(opd_t left, opd_t right,int size){\
		ins_t inst;\
		inst.opcode = code;\
		inst.lopd = left;\
		inst.ropd = right;\
		inst.size = size;\
		inst.opd_num = 2;\
		return inst;\
	}

#define INS_GETINS_ONEARG(name, code)\
	ins_t getins_##name(opd_t left, int size){\
		ins_t inst;\
		inst.opcode = code;\
		inst.lopd = left;\
		inst.size = size;\
		inst.opd_num = 1;\
		return inst;\
	}


INS_GETINS_DEFINE(mov, INS_MOV)
INS_GETINS_DEFINE(add, INS_ADD)
INS_GETINS_DEFINE(sub, INS_SUB)
INS_GETINS_DEFINE(mul, INS_MUL)
INS_GETINS_DEFINE(div, INS_DIV)
INS_GETINS_DEFINE(cmp, INS_CMP)

INS_GETINS_ONEARG(push, INS_PUSH)
INS_GETINS_ONEARG(pop, INS_POP)
INS_GETINS_ONEARG(jmp, INS_JMP)
INS_GETINS_ONEARG(je, INS_JE)
INS_GETINS_ONEARG(jne, INS_JNE)
INS_GETINS_ONEARG(jg, INS_JG)
INS_GETINS_ONEARG(jl, INS_JL)

ins_t getins_int(opd_t opr){
	ins_t inst;
	inst.lopd = opr;
	inst.opcode = INS_INT;
	inst.opd_num = 1;
	return inst;
}

ins_t getins_ret(){
	ins_t inst;
	inst.opd_num = 0;
	inst.opcode = INS_RET;
	return inst;
}

ins_t getins_leave(){
	ins_t inst;
	inst.opd_num = 0;
	inst.opcode = INS_LEAVE;
	return inst;
}

ins_t getins_call(char *name){
	ins_t inst;
	inst.call = name;
	inst.opd_num = 0;
	inst.opcode = INS_CALL;
	return inst;
}

opd_t getopd_reg(reg_t r){
	opd_t opd;
	opd.type = OPD_REG;
	opd.reg = r;
	return opd;
}

opd_t getopd_imm(int v){
	opd_t opd;
	opd.type = OPD_IMM;
	opd.imm = v;
	return opd;
}

opd_t getopd_var(var_t var){
	opd_t opd;
	opd.type = OPD_VAR;
	opd.var = var;
	return opd;
}

/*not used, not implmented*/
opd_t getopd_idx(){
}

opd_t getopd_ind(int offset, reg_t reg){
}

opd_t getopd_addr(char *var){
}

