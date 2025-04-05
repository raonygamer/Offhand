section .text
extern ItemUseInventoryTransaction_ctor
extern NetworkItemStackDescriptor_ctor

global ??0ItemUseInventoryTransaction@@QEAA@XZ:
??0ItemUseInventoryTransaction@@QEAA@XZ:
    mov rax, [rel ItemUseInventoryTransaction_ctor]
    jmp rax

global ??0NetworkItemStackDescriptor@@QEAA@AEBVItemStack@@@Z:
??0NetworkItemStackDescriptor@@QEAA@AEBVItemStack@@@Z:
    mov rax, [rel NetworkItemStackDescriptor_ctor]
    jmp rax