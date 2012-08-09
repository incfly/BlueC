/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
/* The final program's entrance.
 * process the command line arguments, then use the function
 * in other module to generate assembly code in plain text.
 * send the assembly code to assembly compiler "as" by pipe.
 * assembly compiler generate the final executable file.
 * */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "posix.h"
#include "parser.h"
#include "lex.h"
#include "ins.h"
#include "emit.h"
#include "dump.h"

struct {
	char output[50];
	char input[50];
	int isAsm;
} build_info = {"a.out", "", 0};

void process_args(int argc, char *argv[]){
	int ch, fail = 0 ;
	while ( (ch = getopt(argc, argv, "So:")) != -1 ){
		switch(ch){
			case 'o':
				strcpy(build_info.output, optarg);
				break;
			case 'S':
				build_info.isAsm = 1;
				strcpy(build_info.output, "a.s");
				break;
			default:
				fail = 1;
		}
	}
	if (optind >= argc){
		printf("Expected argument after options\n");
		fail = 1;
	}
	/*the number of command line arguments except for option is greater than 1
	 * We only process one input source file*/
	if (argc > optind + 1){ 
		printf("only require one argument to identify input file");
		fail = 1;
	}
	if (fail){
		printf("Usage: %s [-o output] [-S] file\n",argv[0]);
		printf("-o specify the output file name, -S means generation of assembly code\n");
		exit(1);
	}
	strcpy(build_info.input, argv[optind]);
	if (build_info.isAsm){
		char *pos = strrchr(build_info.input, '.');
		strncpy(build_info.output, build_info.input, pos - build_info.input);
		sprintf(build_info.output, "%s.s", build_info.output);
	}
}

int main(int argc, char *argv[]){
	process_args(argc, argv);

	FILE * fp = fopen(build_info.input, "r");
	exit_on_error(fp == NULL, "input file does not exist\n");

	/*stream is the global variable storing tokens*/
	token_num= lexAnalysize(fp, &stream);
	fclose(fp);

	program_t program;
	parse_init(&program);
	parse_program();
	emit_context *context = emit_init();
	emit_program(context, &program);
	char * pool = dump_pool_init(3000);
	dump_program(pool, context);

	/*just output the file storing assembly code in plain text*/
	if (build_info.isAsm){
		fp = fopen(build_info.output, "w");
		fprintf(fp, "%s", pool);
		fclose(fp);
	}

	/*generate executable file*/
	else {
		char build_cmd[100];
		fp = popen("as -o a.o", "w");
		fprintf(fp, "%s", pool);
		pclose(fp);

		sprintf(build_cmd, "ld -dynamic-linker /lib/ld-linux.so.2 -o %s"\
				" a.o -L. -lio -lc", build_info.output);
		system(build_cmd);
		system("rm a.o");
	}

	return 0;
}
