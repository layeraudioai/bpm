#ifndef GBA_REGS_H
#define GBA_REGS_H

#include <stdint.h>

// Typedefs commonly found in gba_types.h
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef uint64_t  u64;
typedef int64_t   s64;

typedef volatile u8   vu8;
typedef volatile u16  vu16;
typedef volatile u32  vu32;

#define REG_BASE 0x04000000

// DMA Registers
#define REG_DMA1SAD     *(vu32*)(REG_BASE + 0x00BC)
#define REG_DMA1DAD     *(vu32*)(REG_BASE + 0x00C0)
#define REG_DMA1CNT     *(vu32*)(REG_BASE + 0x00C4)

#define DMA_DST_FIXED   0x00400000
#define DMA_SRC_INC     0x00000000
#define DMA_REPEAT      0x02000000
#define DMA_32          0x04000000
#define DMA_SPECIAL     0x30000000
#define DMA_ENABLE      0x80000000

// Sound Registers
#define REG_SOUNDCNT_L  *(vu16*)(REG_BASE + 0x0080)
#define REG_SOUNDCNT_H  *(vu16*)(REG_BASE + 0x0082)
#define REG_SOUNDCNT_X  *(vu16*)(REG_BASE + 0x0084)

// Timer Registers
#define REG_TM0D        *(vu16*)(REG_BASE + 0x0100)
#define REG_TM0CNT      *(vu16*)(REG_BASE + 0x0102)
#define TIMER_ENABLE    0x0080

#endif // GBA_REGS_H