#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/cutils.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/boot.h"
#include "system/system.h"

#include "hw/riscv/retro.h"
#include "hw/misc/cia6526.h"

static const MemMapEntry retro_memmap[] = {
    [RETRO_MEM_RAM] =                {  0x00000000,  0x10000     },
    [RETRO_MEM_SOC_RAM] =            {  0x01000000,  0x8000      },
    [RETRO_MEM_SOC_ROM] =            {  0x01100000,  0x8000      },
    [RETRO_MEM_UART0] =              {  0x10000000,  0x100       },
};

static void retro_init(MachineState *machine)
{
    MachineClass *mc = MACHINE_GET_CLASS(machine);
    RetroState *s = RETRO_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();
    RISCVBootInfo boot_info;
    
    if (machine->ram_size != mc->default_ram_size) {
        char *sz = size_to_str(mc->default_ram_size);
        error_report("Invalid RAM size, should be %s", sz);
        g_free(sz);
        exit(EXIT_FAILURE);
    }

    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_RETRO_CPU_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_fatal);

    memory_region_init_ram(machine->ram, OBJECT(&s->soc), "ram", retro_memmap[RETRO_MEM_RAM].size, &error_fatal);
    memory_region_add_subregion(sys_mem, retro_memmap[RETRO_MEM_RAM].base, machine->ram);
    
    if (machine->firmware) {
        hwaddr firmware_load_addr = retro_memmap[RETRO_MEM_RAM].base;
        riscv_load_firmware(machine->firmware, &firmware_load_addr, NULL);
    }
    if (machine->kernel_filename) {
        error_report("Invalid RAM size, should be ROM");
    }
    
    riscv_boot_info_init(&boot_info, &s->soc.cpus);
}

static void retro_cpu_soc_realize(DeviceState *dev, Error **errp) {
    MachineState *ms = MACHINE(qdev_get_machine());
    RetroCpuSoCState *s = RETRO_CPU_SOC(dev);
    MemoryRegion *sys_mem = get_system_memory();

    object_property_set_str(OBJECT(&s->cpus), "cpu-type", ms->cpu_type, &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", 1 /* ms->smp.cpus */, &error_abort); // FIXME: hardcoded to 1
    object_property_set_int(OBJECT(&s->cpus), "resetvec", s->resetvec, &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_fatal);
    
    memory_region_init_ram(&s->ram, OBJECT(dev), "soc.ram", retro_memmap[RETRO_MEM_SOC_RAM].size, &error_fatal);
    memory_region_add_subregion(sys_mem, retro_memmap[RETRO_MEM_SOC_RAM].base, &s->ram);
    
    memory_region_init_rom(&s->rom, OBJECT(dev), "soc.rom", retro_memmap[RETRO_MEM_SOC_ROM].size, &error_fatal);
    memory_region_add_subregion(sys_mem, retro_memmap[RETRO_MEM_SOC_ROM].base, &s->rom);
    
    // DeviceState *uart = qdev_new(TYPE_SERIAL);
    // qdev_prop_set_chr(uart, "chardev", serial_hd(0));
    // sysbus_realize_and_unref(SYS_BUS_DEVICE(uart), &error_fatal); //???
    // if (!sysbus_realize(SYS_BUS_DEVICE(uart), errp)) {
    //     return;
    // }
    // sysbus_mmio_map(SYS_BUS_DEVICE(uart), 0, retro_memmap[RETRO_MEM_UART0].base);
}

static void retro_cpu_soc_init(Object *obj) {
    RetroCpuSoCState *s = RETRO_CPU_SOC(obj);
    object_initialize_child(obj, "cpus", &s->cpus, TYPE_RISCV_HART_ARRAY);
    // object_initialize_child(obj, "uart", &s->uart, TYPE_SERIAL);
}

static const Property retro_soc_props[] = {
    DEFINE_PROP_UINT32("resetvec", RetroCpuSoCState, resetvec, 0x0),
};

static void retro_cpu_soc_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    device_class_set_props(dc, retro_soc_props);
    dc->realize = retro_cpu_soc_realize;
    dc->user_creatable = false;
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
    mc->default_ram_id = "default_ram";
    mc->default_ram_size = retro_memmap[RETRO_MEM_RAM].size;
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
