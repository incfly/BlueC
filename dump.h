/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef DUMP_H
#define DUMP_H
#include "ins.h"
#include "emit.h"
#include <string.h>


extern char *dump_pool_init(int len);

extern void dump_opd(char *pool, opd_t opd);
extern void dump_opd_reg(char *pool, opd_t);
extern void dump_opd_imm(char *pool, opd_t);
extern void dump_opd_idx(char *pool, opd_t);
extern void dump_opd_ind(char *pool, opd_t);
extern void dump_opd_var(char *pool, opd_t);

extern void dump_ins(char *pool, ins_t ins);
extern void dump_ins_opcode(char *, ins_t);
extern void dump_program(char *pool, emit_context *context);

#endif
