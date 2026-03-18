#ifndef GBA_INTERRUPT_H
#define GBA_INTERRUPT_H

#include "gba_regs.h"

#define IRQ_VBLANK (1 << 0)

// Interrupt handler function type
typedef void (*IntFn)(void);

// Prototypes for libgba interrupt functions
void irqSet(u32 mask, IntFn function);
void irqEnable(u32 mask);

#endif // GBA_INTERRUPT_H