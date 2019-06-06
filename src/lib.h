
#ifndef VWHELL_LIB
#define VWHELL_LIB

int check_fail(int res, const char *what, int (*cleanup_func)(void),
               const char *cleanup_desc);

#endif