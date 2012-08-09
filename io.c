/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */

/* This file define the IO build-in function for the language
 * the function is implemented by system call. And compile to 
 * dynamic library, libio.so.
 * */

int read(){
	char str[10];
	int v, i = 0;
	__asm__("movl $3, %%eax\n\t"
			"movl $0, %%ebx\n\t"
			"movl %0, %%ecx\n\t"
			"movl $10, %%edx\n\t"
			"int $0x80"
			:
			:"c"(str)
			);

	if (str[0] == '-')
		i++;
	for ( v = 0; str[i] != 10; i++)
		v = v* 10 + str[i] -'0';
	if (str[0] == '-')
		v *= -1;
	return v;
}

void write(int v){
	char str[10], temp[10];
	int i = 0, j = 0, negative = 0;
	if (v < 0){
		negative = 1;
		v *= -1;
	}
	for ( ; v; v /= 10)
		temp[i++] = v%10 + '0';

	/*if input integer is negative, just put '-' at the end of 
	 *temp array, than the reversion of temp would be expected
	 *string of int value*/
	if (negative)
		temp[i++] = '-';
	for ( ; j < i; j++)
		str[j] = temp[i-1-j];
	str[j++] = '\n';
	__asm__("movl $4, %%eax\n\t"
			"movl $1, %%ebx\n\t"
			"movl %0, %%ecx\n\t"
			"movl %1, %%edx\n\t"
			"int $0x80"
			:
			/*We have to output i+1 characters, including newline \n*/
			:"c"(str), "d"(i+1)
		   );
}

