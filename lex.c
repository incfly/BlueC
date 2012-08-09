/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
/*
 * This file defines the lexical token type, and use posix regex to 
 * analyze tokens from source file.
 * */
#include "lex.h"
#include "util.h"

int STREAM_SIZE = 50;
const int STREAM_INC = 10;

const char *regex[]={
	[TOKEN_BLANK] = "\\s+",
	[TOKEN_KEY_RETURN] = "return",
	[TOKEN_KEY_INT] = "int",
	[TOKEN_KEY_CHAR] = "char",
	[TOKEN_KEY_VOID] = "void",
	[TOKEN_KEY_IF] = "if",
	[TOKEN_NUMBER] = "[0-9]+",
	[TOKEN_ID] = "[a-z]+",
	[TOKEN_PAREN_L] = "\\(",
	[TOKEN_PAREN_R] = "\\)",
	[TOKEN_BRACE_L] = "\\{",
	[TOKEN_BRACE_R] = "\\}",
	[TOKEN_KEY_COMMA] = ",",
	[TOKEN_OPR_ADD] = "\\+",
	[TOKEN_OPR_SUB] = "\\-",
	[TOKEN_OPR_MUL] = "\\*" ,
	[TOKEN_OPR_DIV] = "\\/",
	[TOKEN_OPR_ASSIGN] = "=",
	[TOKEN_CMP_EQ] = "==",
	[TOKEN_CMP_NE] = "!=",
	[TOKEN_CMP_LS] = "<",
	[TOKEN_CMP_GT] = ">",
	[TOKEN_KEY_COLON] = ";"
};


/*read the whole file to buffer
 * keep in mind to allocate n+1 space and add NULL at the end of the buffer!
 * */
char *readAll(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char * c = (char* )malloc(sizeof(char) * len + 1);
	fread(c, sizeof(char), len, fp);
	c[len] = '\0';
	return c;
}

int getstr_newline(char *str){
	int cnt = 0;
	for ( ; strchr(str, '\n') != NULL; str = strchr(str, '\n') + 1, cnt++);
	return cnt;
}

/*
 * lexAnalysize() is used to analysize characters and get tokens from 
 * source file by regex.
 * fin:			the open file pointer of source file
 * stream_pos:	the pointer of the token stream array, I put the tokens 
 * input the array
 * return:		the number of tokens get from source file
 * */
int  lexAnalysize(FILE *fin, token **stream_pos){
	int i, result, curToken, maxLen, line = 1, token_num = 0;
	regex_t reg;
	regmatch_t pm[1];
	char err_msg[100];

	char *buf = readAll(fin);
	token *stream = (token *) malloc(sizeof(token) * STREAM_SIZE);
	char *start, *tok_str;
	for ( start = buf; *start; start += maxLen){
		curToken =  maxLen = -1;
		tok_str = NULL;
		for ( i = 0; i < TOKEN_KIND; i++){
			exit_on_error( regcomp(&reg, regex[i], REG_EXTENDED) != 0,
					"error in compile regex");
			result = regexec(&reg, start, 1, pm, 0);
			if ( result != 0 || pm[0].rm_so != 0 || pm[0].rm_eo <= maxLen)	continue;
			maxLen = pm[0].rm_eo;
			curToken = i;
			if (tok_str)
				free(tok_str);
			tok_str = (char *)malloc(sizeof(char) * maxLen + 1);
			strncpy(tok_str, start, maxLen);
			tok_str[maxLen] = '\0';
		}
		if (curToken == TOKEN_BLANK){
			line += getstr_newline(tok_str);
			continue;
		}
		sprintf(err_msg, "Line %d: error: unrecognized token\n", line);
		exit_on_error(curToken == -1, err_msg);
		if (token_num >= STREAM_SIZE){
			STREAM_SIZE += STREAM_INC;
			stream = (token *) realloc(stream, sizeof(token) * STREAM_SIZE);
		}
		stream[token_num].type = curToken;
		stream[token_num].str = tok_str;
		stream[token_num].line = line;
		token_num++;
	}
	free(buf);
	*stream_pos = stream;
	return token_num;
}
