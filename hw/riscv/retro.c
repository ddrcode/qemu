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

#include "hw/misc/cia6526.h"


static void retro_init(MachineState *machine)
{
   if (machine->cpu_type == NULL) {
        error_report("No CPU type specified for the machine");
        exit(1);
    }
    RISCVCPU *cpu = RISCV_CPU(cpu_create(machine->cpu_type));
    if (!cpu) {
        error_report("Failed to create CPU");
        exit(1);
    }
    qemu_init_vcpu(CPU(cpu));

    //RISCVMachineState *riscv = RISCV_MACHINE(machine);
    MemoryRegion *ram = g_new(MemoryRegion, 1);

    // Initialize the RAM memory region
    memory_region_init_ram(ram, NULL, "ram", machine->ram_size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), 0x100000, ram);

    // Optionally store RAM in the machine state for future use
    machine->ram = ram;

    cpu->env.satp = 0;
    object_property_add_child(OBJECT(machine), "cpu[*]", OBJECT(cpu));
//   RISCVCPU *cpu = RISCV_CPU(cpu_create(machine->cpu_type));
//   riscv_add_cpu(cpu, machine);  // Ensure this function is correctly called
//   memory_region_allocate(get_system_memory(), 0x80000000, 0x100000);  // Example memory allocation

//    DeviceState *cia = qdev_create(NULL, "cia6526");
//    qdev_prop_set_uint32(cia, "addr", 0x10020000);  // Base address
//    qdev_init_nofail(cia);

//    object_initialize_child(OBJECT(machine), "cia1", &machine->cia1, TYPE_CIA6526);
}

static void retro_machine_init(MachineClass *mc) {
    mc->desc = "RISC-V Retro Machine";
    mc->init = retro_init;
    mc->max_cpus = 4;
    mc->is_default = true;
    mc->default_cpu_type = TYPE_RISCV_CPU_RV32E;
    mc->default_ram_id = "default_ram";
    mc->default_ram_size = 16 * MiB;
}

DEFINE_MACHINE("retro", retro_machine_init)
