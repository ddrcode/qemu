#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/boot.h"

#include "hw/riscv/retro.h"
#include "hw/misc/cia6526.h"

static const MemMapEntry retro_memmap[] = {
    [RETRO_DEV_SOC_ROM] =            {  0x01000000,  0x8000      },
    [RETRO_DEV_RAM] =                {  0x00000000,  0x10000     },
};

static void retro_init(MachineState *machine)
{
    MachineClass *mc = MACHINE_GET_CLASS(machine);
    RetroState *s = RETRO_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();

    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_RETRO_CPU_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_fatal);

    MemoryRegion *ram = g_new(MemoryRegion, 1);

    memory_region_init_ram(ram, NULL, "ram", 0x10000, &error_fatal);
    memory_region_add_subregion(get_system_memory(), 0, ram);
    // memory_region_add_subregion(sys_mem, retro_memmap[RETRO_DEV_RAM].base, machine->ram);

    machine->ram = ram;


}

static void retro_cpu_soc_realize(DeviceState *dev, Error **errp) {
    MachineState *ms = MACHINE(qdev_get_machine());
    RetroCpuSoCState *s = RETRO_CPU_SOC(dev);
    // MemoryRegion *sys_mem = get_system_memory();
    // const MemMapEntry *memmap = retro_memmap;

    object_property_set_str(OBJECT(&s->cpus), "cpu-type", ms->cpu_type, &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", ms->smp.cpus, &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_fatal);
    //
    // MemoryRegion *ram = g_new(MemoryRegion, 1);
    // memory_region_init_ram(ram, OBJECT(dev), "ram", 0x10000, &error_fatal);
    // memory_region_add_subregion(sys_mem, 0, ram);
    // machine->ram = ram;
}

static void retro_cpu_soc_init(Object *obj) {
    RetroCpuSoCState *s = RETRO_CPU_SOC(obj);
    object_initialize_child(obj, "cpus", &s->cpus, TYPE_RISCV_HART_ARRAY);
}

static void retro_cpu_soc_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = retro_cpu_soc_realize;
}

static void retro_machine_class_init(ObjectClass *oc, void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "RISC-V Retro Machine";
    mc->init = retro_init;
    mc->max_cpus = 1;
    mc->min_cpus = 1;
    mc->default_cpus = 1;
    mc->is_default = true;
    mc->default_cpu_type = TYPE_RISCV_CPU_RV32E;
    // mc->default_ram_id = "default_ram";
    mc->default_ram_size = 16 * MiB;
}

static const TypeInfo retro_machine_types[] = {
    {
        .name = TYPE_RETRO_CPU_SOC,
        .parent = TYPE_DEVICE,
        .instance_size = sizeof(RetroCpuSoCState),
        .instance_init = retro_cpu_soc_init,
        .class_init = retro_cpu_soc_class_init,
    },
    {
        .name = TYPE_RETRO_MACHINE,
        .parent = TYPE_MACHINE,
        .instance_size = sizeof(RetroState),
        .class_init = retro_machine_class_init,
    }
};

DEFINE_TYPES(retro_machine_types)
