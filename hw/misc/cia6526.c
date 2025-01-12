#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "qemu/timer.h"
#include "hw/qdev-properties.h"

#include "hw/misc/cia6526.h"


static void cia6526_reset(DeviceState *dev)
{
    CIA6526State *s = CIA6526(dev);
    memset(s->registers, 0, sizeof(s->registers));
    s->timer_a = 0xFFFF;
    s->timer_b = 0xFFFF;
}

static uint64_t cia6526_read(void *opaque, hwaddr offset, unsigned size)
{
    CIA6526State *s = (CIA6526State *)opaque;

    if (offset < sizeof(s->registers)) {
        return s->registers[offset];
    }
    return 0xFF;
}

static void cia6526_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    CIA6526State *s = (CIA6526State *)opaque;

    if (offset < sizeof(s->registers)) {
        s->registers[offset] = value;

        // Handle specific behaviors, like resetting timers or setting interrupts
        switch (offset) {
        case 0x04:  // Timer A low byte
            s->timer_a = (s->timer_a & 0xFF00) | value;
            break;
        case 0x05:  // Timer A high byte
            s->timer_a = (s->timer_a & 0x00FF) | (value << 8);
            break;
        // Handle other registers as needed
        }
    }
}

static const MemoryRegionOps cia6526_ops = {
    .read = cia6526_read,
    .write = cia6526_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void cia6526_realize(DeviceState *dev, Error **errp)
{
    CIA6526State *s = CIA6526(dev);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mmio);
    memory_region_init_io(&s->mmio, OBJECT(s), &cia6526_ops, s, "cia6526", 0x10);  // 16 registers
}


static void cia6526_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->reset = cia6526_reset;
}

static const TypeInfo cia6526_info = {
    .name = "cia6526",
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(CIA6526State),
    .class_init = cia6526_class_init,
};

static void cia6526_register_types(void)
{
    type_register_static(&cia6526_info);
}

type_init(cia6526_register_types);

