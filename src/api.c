#include "api.h"

#define API(ret, name, args) ret(*name) args = 0;
#include "libc-imports.h"
#include "libkernel-imports.h"
#undef API

int (*lk_getpid)(void) = 0;

static inline void *resolve_symbol(SceKernelModule handle, const char *name) {
  void *addrp;
  if (dlsym(handle, name, &addrp))
    __builtin_trap();
  return addrp;
}

/* Fill every import from the launcher-provided table. Returns 0 when the
 * extension is present and valid (and everything was assigned), -1 when the
 * caller must fall back to the dlsym resolver. Idempotent: once the table has
 * been applied, later calls report success without touching anything. */
static int api_try_table(payload_args_t *args) {
  static int table_applied = 0;

  if (table_applied)
    return 0;

  if (args == 0 || args->api_entries == 0 ||
      args->api_magic != KEXP_API_MAGIC || args->api_count != KEXP_API_COUNT)
    return -1;

  if (((uintptr_t)args->api_entries & (sizeof(void *) - 1)) != 0)
    return -1;

  void **table = args->api_entries;

  sceKernelSendNotificationRequest =
      (typeof(sceKernelSendNotificationRequest))table[KEXP_API_NOTIFY];
  sysctlbyname = (typeof(sysctlbyname))table[KEXP_API_SYSCTLBYNAME];
  pthread_create = (typeof(pthread_create))table[KEXP_API_PTHREAD_CREATE];
  pthread_join = (typeof(pthread_join))table[KEXP_API_PTHREAD_JOIN];
  lk_getpid = (typeof(lk_getpid))table[KEXP_API_GETPID];

  malloc = (typeof(malloc))table[KEXP_API_MALLOC];
  free = (typeof(free))table[KEXP_API_FREE];
  memcpy = (typeof(memcpy))table[KEXP_API_MEMCPY];
  memset = (typeof(memset))table[KEXP_API_MEMSET];
  strcmp = (typeof(strcmp))table[KEXP_API_STRCMP];
  memcmp = (typeof(memcmp))table[KEXP_API_MEMCMP];
  vsnprintf = (typeof(vsnprintf))table[KEXP_API_VSNPRINTF];

  table_applied = 1;
  return 0;
}

void init_libkernel_api(payload_args_t *args) {
  if (api_try_table(args) == 0)
    return;

#define API(ret, name, args)                                                   \
  name = (ret(*) args)resolve_symbol(LIBKERNEL_HANDLE, #name);

#include "libkernel-imports.h"

#undef API

  lk_getpid = (int (*)(void))resolve_symbol(LIBKERNEL_HANDLE, "getpid");
}

void init_libc_api(payload_args_t *args) {
  if (api_try_table(args) == 0)
    return;

#define API(ret, name, args)                                                   \
  name = (ret(*) args)resolve_symbol(LIBC_HANDLE, #name);

#include "libc-imports.h"

#undef API
}
