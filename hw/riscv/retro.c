#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qemu/module.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/boot.h"
#include "exec/address-spaces.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "system/system.h"
#include "hw/char/serial-mm.h"



#define TYPE_RISCV32_RETRO_MACHINE MACHINE_TYPE_NAME("riscv32-retro-machine")
#define RISCV32_RETRO_MACHINE(obj) \
    OBJECT_CHECK(RISCV32RetroMachineState, (obj), TYPE_RISCV32_RETRO_MACHINE)

typedef struct RISCV32RetroMachineState {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
} RISCV32RetroMachineState;

enum {
    RETRO_RAM,
    RETRO_ROM,
    RETRO_UART0,
};

static const struct MemmapEntry {
    hwaddr base;
    hwaddr size;
} riscv32_retro_memmap[] = {
    [RETRO_RAM]   = { 0x80000000,   64 * KiB },
    [RETRO_ROM]   = { 0x00000000,   64 * KiB },
    [RETRO_UART0] = { 0x10000000,      0x100 },
};

static void riscv32_retro_machine_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = riscv32_retro_memmap;
    RISCV32RetroMachineState *s = RISCV32_RETRO_MACHINE(machine);
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *rom = g_new(MemoryRegion, 1);

    /* Initialize CPU */
    object_initialize_child(OBJECT(machine), "cpu", &s->cpus,
                            TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->cpus), "cpu-type",
                            RISCV_CPU_TYPE_NAME("rv32i"), &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", 1, &error_abort);
    object_property_set_bool(OBJECT(&s->cpus), "realized", true, &error_abort);

    /* RAM */
    memory_region_init_ram(main_mem, NULL, "riscv32_retro.ram",
                           memmap[RETRO_RAM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[RETRO_RAM].base, main_mem);

    /* ROM */
    memory_region_init_rom(rom, NULL, "riscv32_retro.rom",
                           memmap[RETRO_ROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[RETRO_ROM].base, rom);

    /* UART */
    serial_mm_init(system_memory, memmap[RETRO_UART0].base, 0,
                   NULL, 115200, serial_hd(0), DEVICE_LITTLE_ENDIAN);

    /* Load firmware or kernel */
    // riscv_load_kernel(RISCV_CPU(s->cpus.harts[0]),
    //                   machine->kernel_filename,
    //                   machine->initrd_filename,
    //                   machine->kernel_cmdline,
    //                   NULL);
}

static void riscv32_retro_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "RISC-V 32-bit retro machine";
    mc->init = riscv32_retro_machine_init;
    mc->max_cpus = 1;
    mc->default_cpu_type = RISCV_CPU_TYPE_NAME("rv32i");
}

static const TypeInfo riscv32_retro_machine_type_info = {
    .name = TYPE_RISCV32_RETRO_MACHINE,
    .parent = TYPE_MACHINE,
    .class_init = riscv32_retro_machine_class_init,
    .instance_size = sizeof(RISCV32RetroMachineState),
};

static void riscv32_retro_machine_types(void)
{
    type_register_static(&riscv32_retro_machine_type_info);
}

type_init(riscv32_retro_machine_types)

