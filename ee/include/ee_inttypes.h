#ifndef __INTTYPES_H__
#define __INTTYPES_H__

#include <stdint.h>

#define PRId8 #hhd
#define PRId16 #hd
#define PRId32 #d
#define PRId64 #lld

#define PRIdFAST8 #lld
#define PRIdFAST16 #lld
#define PRIdFAST32 #lld
#define PRIdFAST64 #lld

#define PRIdLEAST8 #hhd
#define PRIdLEAST16 #hd
#define PRIdLEAST32 #d
#define PRIdLEAST64 #ld

#define PRIdMAX #jd
#define PRIdPTR #d

#define PRIi8 #hhi
#define PRIi16 #hi
#define PRIi32 #i
#define PRIi64 #lli

#define PRIiFAST8 #lli
#define PRIiFAST16 #lli
#define PRIiFAST32 #lli
#define PRIiFAST64 #lli

#define PRIiLEAST8 #hhi
#define PRIiLEAST16 #hi
#define PRIiLEAST32 #i
#define PRIiLEAST64 #li

#define PRIiMAX #ji
#define PRIiPTR #i

#define PRIu8 #hhu
#define PRIu16 #hu
#define PRIu32 #u
#define PRIu64 #llu

#define PRIuFAST8 #llu
#define PRIuFAST16 #llu
#define PRIuFAST32 #llu
#define PRIuFAST64 #llu

#define PRIuLEAST8 #hhu
#define PRIuLEAST16 #hu
#define PRIuLEAST32 #u
#define PRIuLEAST64 #lu

#define PRIuMAX #ju
#define PRIuPTR #u

#define PRIo8 #hho
#define PRIo16 #ho
#define PRIo32 #o
#define PRIo64 #llo

#define PRIoFAST8 #llo
#define PRIoFAST16 #llo
#define PRIoFAST32 #llo
#define PRIoFAST64 #llo

#define PRIoLEAST8 #hho
#define PRIoLEAST16 #ho
#define PRIoLEAST32 #o
#define PRIoLEAST64 #lo

#define PRIoMAX #jo
#define PRIoPTR #o

#define PRIx8 #hhx
#define PRIx16 #hx
#define PRIx32 #x
#define PRIx64 #llx

#define PRIxFAST8 #llx
#define PRIxFAST16 #llx
#define PRIxFAST32 #llx
#define PRIxFAST64 #llx

#define PRIxLEAST8 #hhx
#define PRIxLEAST16 #hx
#define PRIxLEAST32 #x
#define PRIxLEAST64 #lx

#define PRIxMAX #jx
#define PRIxPTR #x

#define PRIX8 #hhX
#define PRIX16 #hX
#define PRIX32 #X
#define PRIX64 #llX

#define PRIXFAST8 #llX
#define PRIXFAST16 #llX
#define PRIXFAST32 #llX
#define PRIXFAST64 #llX

#define PRIXLEAST8 #hhX
#define PRIXLEAST16 #hX
#define PRIXLEAST32 #X
#define PRIXLEAST64 #lX

#define PRIXMAX #jX
#define PRIXPTR #X

#define SCNd8 #hhd
#define SCNd16 #hd
#define SCNd32 #d
#define SCNd64 #lld

#define SCNdFAST8 #lld
#define SCNdFAST16 #lld
#define SCNdFAST32 #lld
#define SCNdFAST64 #lld

#define SCNdLEAST8 #hhd
#define SCNdLEAST16 #hd
#define SCNdLEAST32 #d
#define SCNdLEAST64 #ld

#define SCNdMAX #jd
#define SCNdPTR #d

#define SCNi8 #hhi
#define SCNi16 #hi
#define SCNi32 #i
#define SCNi64 #lli

#define SCNiFAST8 #lli
#define SCNiFAST16 #lli
#define SCNiFAST32 #lli
#define SCNiFAST64 #lli

#define SCNiLEAST8 #hhi
#define SCNiLEAST16 #hi
#define SCNiLEAST32 #i
#define SCNiLEAST64 #li

#define SCNiMAX #ji
#define SCNiPTR #i

#define SCNu8 #hhu
#define SCNu16 #hu
#define SCNu32 #u
#define SCNu64 #llu

#define SCNuFAST8 #llu
#define SCNuFAST16 #llu
#define SCNuFAST32 #llu
#define SCNuFAST64 #llu

#define SCNuLEAST8 #hhu
#define SCNuLEAST16 #hu
#define SCNuLEAST32 #u
#define SCNuLEAST64 #lu

#define SCNuMAX #ju
#define SCNuPTR #u

#define SCNo8 #hho
#define SCNo16 #ho
#define SCNo32 #o
#define SCNo64 #llo

#define SCNoFAST8 #llo
#define SCNoFAST16 #llo
#define SCNoFAST32 #llo
#define SCNoFAST64 #llo

#define SCNoLEAST8 #hho
#define SCNoLEAST16 #ho
#define SCNoLEAST32 #o
#define SCNoLEAST64 #lo

#define SCNoMAX #jo
#define SCNoPTR #o

#define SCNx8 #hhx
#define SCNx16 #hx
#define SCNx32 #x
#define SCNx64 #llx

#define SCNxFAST8 #llx
#define SCNxFAST16 #llx
#define SCNxFAST32 #llx
#define SCNxFAST64 #llx

#define SCNxLEAST8 #hhx
#define SCNxLEAST16 #hx
#define SCNxLEAST32 #x
#define SCNxLEAST64 #lx

#define SCNxMAX #jx
#define SCNxPTR #x

#define SCNX8 #hhX
#define SCNX16 #hX
#define SCNX32 #X
#define SCNX64 #llX

#define SCNXFAST8 #llX
#define SCNXFAST16 #llX
#define SCNXFAST32 #llX
#define SCNXFAST64 #llX

#define SCNXLEAST8 #hhX
#define SCNXLEAST16 #hX
#define SCNXLEAST32 #X
#define SCNXLEAST64 #lX

#define SCNXMAX #jX
#define SCNXPTR #X

typedef struct { intmax_t rem; intmax_t quot; } imaxdiv_t;

#define imaxabs(a) ((intmax_t)llabs((long long)a))
#define imaxdiv(a,b) ((imaxdiv_t)lldiv((lldiv_t)a,(lldiv_t)b))
#define strtoimax(a,b,c,d) ((intmax_t) strtoll(a,b,c,d))
#define strtoumax(a,b,c,d) ((uintmax_t) strtoull(a,b,c,d))
#define wcstoimax(a,b,c,d) ((intmax_t) wcstoll(a,b,c,d))
#define wcstoumax(a,b,c,d) ((uintmax_t) wcstoull(a,b,c,d))

#endif /*__INTTYPES_H__*/
