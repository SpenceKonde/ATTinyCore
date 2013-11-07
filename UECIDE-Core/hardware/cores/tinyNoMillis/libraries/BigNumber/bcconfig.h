/*
* config.h
* number.c from GNU bc-1.06 exports some symbols without the bc_ prefix.
* This header file fixes this without touching either number.c or number.h
* (luckily, number.c already wants to include a config.h).
* Clients of number.c should include config.h before number.h.
*/

#include <string.h>
#include <limits.h>
#define NDEBUG 1

#define _zero_		bc_zero
#define _one_		bc_one
#define _two_		bc_two
#define num2str		bc_num2str
#define mul_base_digits bc_mul_base_digits

#define bc_rt_warn		bc_error
#define bc_rt_error		bc_error
#define bc_out_of_memory()	bc_error(0)

void bc_error(const int mesg);

