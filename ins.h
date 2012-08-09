/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef INS_H
#define INS_H
#include "parser.h"

typedef enum reg_t{
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_ESI,
	REG_EDI,
	REG_ESP,
	REG_EBP,
	REG_NONE
} reg_t; 

typedef enum opcode_t{
	INS_MOV,
	INS_ADD,
	INS_SUB,
	INS_MUL,
	INS_DIV,
	INS_CMP,
	INS_JMP, 
	INS_JE,
	INS_JNE,
	INS_JG,
	INS_JL,
	INS_CALL,
	INS_RET, 
	INS_INT,
	INS_PUSH, 
	INS_POP,
	INS_LEAVE
} opcode_t;

/* The operator type.*/
typedef enum opdtype_t{
	OPD_IMM,
	OPD_REG,
	OPD_VAR, 
	OPD_IDX,/*index: base(offset-reg, index-reg, size)*/
	OPD_IND,/*indirect: off-imm(reg)*/
	OPD_VAR_ADDR /*the variable's address, not used in fact*/
} opdtype_t;

/*operand type*/
typedef struct opd_t{
	int imm;
	reg_t reg;

	char *idx_addr;/*索引寻址中base*/
	reg_t idx_offset;
	reg_t idx_index;
	int idx_size;

	/*indirect addressing*/
	int idr_offset;
	reg_t idr_addr;

	var_t var;

	opdtype_t type;
} opd_t;

/*base(off-reg, index-reg, size)*/
/*instruction data structure*/
typedef struct ins_t{
	opd_t lopd;/*left operand*/
	opd_t ropd;/*right operand*/
	opcode_t opcode;
	int size; /*operand's byte size, in fact, all 4 byte*/
	int opd_num; /*the number of operand*/
	char *call;
} ins_t;

/*arguments: left operand, right operand, operand size*/
extern ins_t getins_mov(opd_t , opd_t, int size);
extern ins_t getins_add(opd_t , opd_t, int );
extern ins_t getins_sub(opd_t , opd_t, int );
extern ins_t getins_mul(opd_t , opd_t, int );
extern ins_t getins_div(opd_t , opd_t, int );

extern ins_t getins_push(opd_t , int );
extern ins_t getins_pop(opd_t , int );
extern ins_t getins_jmp(opd_t , int );
extern ins_t getins_je(opd_t , int );
extern ins_t getins_jne(opd_t , int );
extern ins_t getins_jg(opd_t , int );
extern ins_t getins_jl(opd_t , int );


extern ins_t getins_cmp(opd_t , opd_t, int );
extern ins_t getins_call(char *);

extern ins_t getins_int(opd_t );
extern ins_t getins_ret();
extern ins_t getins_leave();


extern opd_t getopd_reg(reg_t r);
extern opd_t getopd_imm(int v);
extern opd_t getopd_var(var_t var);
extern opd_t getopd_idx();
extern opd_t getopd_ind(int offset, reg_t reg);
extern opd_t getopd_addr(char *var);

#endif
