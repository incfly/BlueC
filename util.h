/*	
 * Author: Jianfei Hu (C)
 * Date:   2012/08/07
 */
#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>

#define max(a, b) (a > b ? a : b)

extern void exit_on_error(int conv, char *str);

/* in many situation, the dynamic array in c is needed. 
 * I plan to implement the data structure by realloc(), here is the 
 * definition. but i haven't implement this and all the data field in 
 * struct is defined as fixed size array. it's not a good idea. 
 * i'm sorry. :-( */
typedef struct array_t{
	int element_size;
	int len;
	int capacity;
	void *data;
}array_t;

extern array_t *array_init(int elem_size, int len);
extern array_t *array_add(void *add_elem);
extern void * array_get(int pos);
extern int array_destroy(array_t *arr);

#endif
