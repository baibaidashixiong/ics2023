#include <isa.h>



word_t *csr_register(word_t index){
    switch (index)
    {
    case 0x300 :
        printf("mstatus:%d\n", cpu.csr.mstatus);
        return &(cpu.csr.mstatus);
    case 0x305 :
        printf("mtvec:0x%x\n", cpu.csr.mtvec);
        return &(cpu.csr.mtvec);
    case 0x341 :
        printf("mepc:%x\n", cpu.csr.mepc);
        return &(cpu.csr.mepc);
    case 0x342 : 
        printf("mcause:%d\n", cpu.csr.mcause);
        return &(cpu.csr.mcause);
    default:
        panic("Wrong csr reg");
    }
}