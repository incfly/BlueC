/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#include "util.h"

void exit_on_error(int conv, char *str){
	if (conv){
		printf("%s", str);
		exit(1);
	}
}
