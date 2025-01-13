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

static void retro_init(MachineState *machine)
{
    if (machine->cpu_type == NULL) {
        error_report("No CPU type specified for the machine");
        exit(1);
    }

    MemoryRegion *ram = g_new(MemoryRegion, 1);

    // Initialize the RAM memory region
    memory_region_init_ram(ram, NULL, "ram", 0x10000, &error_fatal);
    memory_region_add_subregion(get_system_memory(), 0, ram);

    // MemoryRegion *cpu_ram = g_new(MemoryRegion, 2);
    // memory_region_init_ram(ram, NULL, "cpu_ram", 0x10000, &error_fatal);
    // memory_region_add_subregion(get_system_memory(), 0x010000, cpu_ram);

    // Optionally store RAM in the machine state for future use
    machine->ram = ram;

    // Create and initialize the hart array
    // Object *hart_array = object_new(TYPE_RISCV_HART_ARRAY);
    // object_property_set_int(hart_array, "num-harts", machine->smp.cpus, &error_abort);
    // object_property_set_str(hart_array, "cpu-type", machine->cpu_type, &error_abort);
    // object_property_add_child(OBJECT(machine), "hart-array", hart_array);

    //RISCVCPU *cpu = RISCV_CPU(object_property_get_link(hart_array, "cpu[*]", NULL));
    // qemu_init_vcpu(CPU(hart_array));
    // cpu_address_space_init(CPU(hart_array), 0, "cpu-ram", ram);
    // if (!cpu) {
    //     error_report("Failed to create CPU");
    //     exit(1);
    // }
    

    // Explicitly set the satp mode to 'bare' (no address translation)
   // cpu->env.satp = 0;

    // Add the CPU to the machine
    //object_property_add_child(OBJECT(machine), "cpu[*]", OBJECT(cpu));
    // object_property_set_str(OBJECT(cpu), "cpu-type", machine->cpu_type, &error_abort);
    // object_property_set_int(OBJECT(cpu), "num-harts", 1, &error_abort);
    
    // RetroState *state = RETRO_MACHINE(machine);
    // object_initialize_child(OBJECT(machine), "cpus", &state->cpus, TYPE_RISCV_HART_ARRAY);

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
