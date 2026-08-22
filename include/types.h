#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define ALIGN_UP(addr, align) (addr + (align - 1)) & ~(align - 1)
#define ALIGN_DOWN(addr, align) addr & ~(align - 1)

#define O_WRONLY 1

#define AF_UNIX 1
#define AF_INET 2
#define AF_INET6 28
#define SOCK_STREAM 1
#define SOCK_DGRAM 2

#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define IPPROTO_IPV6 41

#define IPV6_2292PKTOPTIONS 25
#define IPV6_PKTINFO 46
#define IPV6_NEXTHOP 48
#define IPV6_RTHDR 51
#define IPV6_TCLASS 61

typedef int32_t SceKernelModule;
typedef int32_t socklen_t;

typedef struct {
  int32_t type;
  int32_t reqId;
  int32_t priority;
  int32_t msg_id;
  int32_t target_id;
  int32_t user_id;
  int32_t unk1;
  int32_t unk2;
  int32_t app_id;
  int32_t error_num;
  int32_t unk3;
  unsigned char use_icon_image_uri;
  char message[0x400];
  char icon_uri[0x400];
  char unk[0x400];
} NotificationRequest;

/* ---------------------------------------------------------------------------
 * Optional pre-resolved API table (v2 args extension).
 *
 * Some launchers cannot let the payload resolve its own imports: syscall 0x24f
 * (dlsym) is refused for a WebProcess, so a kexp spawned from the PS5 browser
 * would hit __builtin_trap() in resolve_symbol(). Such launchers instead fill
 * api_entries[] with the runtime addresses of every imported function and set
 * api_magic/api_count; kexp then performs zero dlsym calls. Launchers that do
 * not know these addresses simply leave the extension zeroed (legacy 0x28
 * block) and kexp falls back to its internal resolver as before.
 *
 * Table layout is a fixed contract (index -> function), matching the order of
 * libkernel-imports.h + libc-imports.h below:
 *   [0] sceKernelSendNotificationRequest   (libkernel)
 *   [1] sysctlbyname                       (libkernel)
 *   [2] pthread_create                     (libkernel)
 *   [3] pthread_join                       (libkernel)
 *   [4] getpid                             (libkernel)
 *   [5] malloc                             (libSceLibcInternal)
 *   [6] free                               (libSceLibcInternal)
 *   [7] memcpy                             (libSceLibcInternal)
 *   [8] memset                             (libSceLibcInternal)
 *   [9] strcmp                             (libSceLibcInternal)
 *   [10] memcmp                            (libSceLibcInternal)
 *   [11] vsnprintf                         (libSceLibcInternal)
 * ------------------------------------------------------------------------- */
#define KEXP_API_MAGIC 0x4B585032U /* 'KXP2' */
#define KEXP_API_COUNT 12

enum {
  KEXP_API_NOTIFY = 0,
  KEXP_API_SYSCTLBYNAME,
  KEXP_API_PTHREAD_CREATE,
  KEXP_API_PTHREAD_JOIN,
  KEXP_API_GETPID,
  KEXP_API_MALLOC,
  KEXP_API_FREE,
  KEXP_API_MEMCPY,
  KEXP_API_MEMSET,
  KEXP_API_STRCMP,
  KEXP_API_MEMCMP,
  KEXP_API_VSNPRINTF,
};

typedef struct {
  int master_pipe[2];
  int victim_pipe[2];
  uintptr_t allproc;
  char *elfldr_ptr;
  size_t elfldr_size;
  /* v2 extension: optional pre-resolved API table (see above). Legacy
   * launchers pass only the 0x28-byte prefix; reading past it is safe on the
   * heap and misinterpretation is ruled out by validating magic, count and
   * pointer before use. */
  uint32_t api_magic;
  uint32_t api_count;
  void **api_entries;
} payload_args_t;

_Static_assert(offsetof(payload_args_t, elfldr_size) == 0x20,
               "legacy prefix must stay 0x28 bytes");
_Static_assert(offsetof(payload_args_t, api_magic) == 0x28,
               "api_magic offset broke the v2 args contract");
_Static_assert(offsetof(payload_args_t, api_count) == 0x2C,
               "api_count offset broke the v2 args contract");
_Static_assert(offsetof(payload_args_t, api_entries) == 0x30,
               "api_entries offset broke the v2 args contract");
_Static_assert(sizeof(payload_args_t) == 0x38,
               "payload_args_t size broke the v2 args contract");

typedef struct {
  uint32_t cnt;
  uint32_t in;
  uint32_t out;
  uint32_t size;
  char *buffer;
} pipebuf_t;

typedef struct {
  int master_pipe[2];
  int victim_pipe[2];
  pipebuf_t pipebuf;
} karw_ctx_t;

typedef struct {
  uintptr_t cb3;
  uintptr_t cr3;
  uintptr_t mmio;
  uintptr_t dmap;
  uintptr_t kmdp;
  uintptr_t ktext;
  uintptr_t ksize;
  uintptr_t kdata;
  uintptr_t kaslr;
  uintptr_t softc;
  uintptr_t krodata;
  uintptr_t allproc;
  uintptr_t fdt_ofiles;
} kaddrs_t;

#endif