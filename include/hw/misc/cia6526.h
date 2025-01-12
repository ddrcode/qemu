#ifndef HW_MISC_CIA6526_H
#define HW_MISC_CIA6526_h

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "qemu/timer.h"
#include "hw/qdev-properties.h"

typedef struct {
    SysBusDevice parent_obj;

    uint8_t registers[16];  // CIA 6526 has 16 registers
    uint16_t timer_a;
    uint16_t timer_b;
    QEMUTimer *timer_a_event;
    QEMUTimer *timer_b_event;
    qemu_irq irq;
} CIA6526State;

#define TYPE_CIA6526 "cia6526"

#endif
