#ifndef HW_RETRO_H
#define HW_RETRO_H

#include "hw/riscv/riscv_hart.h"
#include "hw/boards.h"
#include "qom/object.h"

#define TYPE_RETRO_CPU_SOC "riscv.retro.cpu-soc"
OBJECT_DECLARE_SIMPLE_TYPE(RetroCpuSoCState, RETRO_CPU_SOC)

typedef struct RetroCpuSoCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
} RetroCpuSoCState;


#define TYPE_RETRO_MACHINE MACHINE_TYPE_NAME("retro")
OBJECT_DECLARE_SIMPLE_TYPE(RetroState, RETRO_MACHINE)

typedef struct RetroState {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    RetroCpuSoCState soc;
} RetroState;



#endif