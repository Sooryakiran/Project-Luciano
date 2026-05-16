#include "debug.h"
#include "types.h"

__attribute__((noreturn))
void exception_handler(void);


void exception_handler() {
    k_panic("exception occured");
}