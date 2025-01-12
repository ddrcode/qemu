#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "qemu/guest-random.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/qdev-properties.h"
#include "hw/char/serial-mm.h"
#include "target/riscv/cpu.h"
#include "hw/core/sysbus-fdt.h"
#include "target/riscv/pmu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/iommu.h"
#include "hw/riscv/riscv-iommu-bits.h"
#include "hw/riscv/virt.h"
#include "hw/riscv/boot.h"
#include "hw/riscv/numa.h"
#include "kvm/kvm_riscv.h"
#include "hw/firmware/smbios.h"
#include "hw/intc/riscv_aclint.h"
#include "hw/intc/riscv_aplic.h"
#include "hw/intc/sifive_plic.h"
#include "hw/misc/sifive_test.h"
#include "hw/platform-bus.h"
#include "chardev/char.h"
#include "system/device_tree.h"
#include "system/system.h"
#include "system/tcg.h"
#include "system/kvm.h"
#include "system/tpm.h"
#include "system/qtest.h"
#include "hw/pci/pci.h"
#include "hw/pci-host/gpex.h"
#include "hw/display/ramfb.h"
#include "hw/acpi/aml-build.h"
#include "qapi/qapi-visit-common.h"
#include "hw/virtio/virtio-iommu.h"

static void retro_init(MachineState *machine)
{
    RISCVMachineState *riscv = RISCV_MACHINE(machine);
    MemoryRegion *ram = g_new(MemoryRegion, 1);

    // Initialize the RAM memory region
    memory_region_init_ram(ram, NULL, "ram", machine->ram_size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), 0x80000000, ram);

    // Optionally store RAM in the machine state for future use
    riscv->ram = ram;
//   RISCVCPU *cpu = RISCV_CPU(cpu_create(machine->cpu_type));
//   riscv_add_cpu(cpu, machine);  // Ensure this function is correctly called
//   memory_region_allocate(get_system_memory(), 0x80000000, 0x100000);  // Example memory allocation
}

static void retro_machine_init(MachineClass *mc) {
    mc->desc = "RISC-V Retro Machine";
    mc->init = retro_init;
    mc->max_cpus = 1;
    mc->is_default = true;
    mc->default_ram_id = "ram";
    mc->default_ram_size = 16 * MiB;
}

DEFINE_MACHINE("retro", retro_machine_init)
