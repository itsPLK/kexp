#ifndef API_H
#define API_H

#include "types.h"

#define API(id, ret, name, args) ret name args;
#include "syscalls-imports.h"
#undef API

#define API(ret, name, args) ret(*name) args;
#include "libc-imports.h"
#include "libkernel-imports.h"
#undef API

/* Both take the payload args: when a valid KEXP_API table is present the
 * function pointers are taken from it (zero dlsym calls), otherwise each
 * import is resolved through syscall 0x24f as before. */
void init_libkernel_api(payload_args_t *args);
void init_libc_api(payload_args_t *args);

/* libkernel getpid, resolved either from the API table or via dlsym in
 * init_libkernel_api(). Named lk_getpid because `getpid` collides with the
 * raw syscall stub generated from syscalls-imports.h. */
extern int (*lk_getpid)(void);

#endif