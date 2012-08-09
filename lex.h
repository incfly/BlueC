/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef LEX_H
#define LEX_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <regex.h>

typedef enum TOKEN_TYPE{
	TOKEN_BLANK  = 0, 
	TOKEN_KEY_RETURN ,
	TOKEN_KEY_INT,
	TOKEN_KEY_CHAR,
	TOKEN_KEY_VOID,
	TOKEN_KEY_IF,
	TOKEN_NUMBER ,
	TOKEN_ID ,
	TOKEN_PAREN_L ,
	TOKEN_PAREN_R ,
	TOKEN_BRACE_L ,
	TOKEN_BRACE_R ,
	TOKEN_KEY_COMMA ,
	TOKEN_OPR_ADD ,
	TOKEN_OPR_SUB ,
	TOKEN_OPR_MUL ,
	TOKEN_OPR_DIV ,
	TOKEN_OPR_ASSIGN ,
	TOKEN_CMP_EQ,
	TOKEN_CMP_NE,
	TOKEN_CMP_LS,
	TOKEN_CMP_GT,
	TOKEN_KEY_COLON ,
	TOKEN_KIND  /*the kind number of token*/
} TOKEN_TYPE;



/*define token structure*/
typedef struct token
{
	int type; /*identify the token type*/
	char *str;
	int line;/*the line number of token's position, used for error prompt*/
} token;

extern int token_num;
extern const char * regex[]; 

extern int STREAM_SIZE;
extern const int STREAM_INC;

extern int lexAnalysize(FILE *, token **stream_pos);

extern void showTokens(int , token *);

#endif
