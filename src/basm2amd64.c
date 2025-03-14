#define BM_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "bm.h"

#include "natives.asm.h"

static void usage(FILE *stream) {
    fprintf(stream, "Usage: ./basm2amd64 <input.basm> \n");
}

Basm basm = {0};

int main(int argc, char *argv[]) {
    if(argc < 2) {
	usage(stderr);
	fprintf(stderr, "ERROR: no input provided.\n");
	exit(1);
    }

    basm_translate_source(&basm, cstr_as_sv(argv[1]));

    printf("BITS 64\n");
    printf("%%define BM_STACK_CAPACITY %d\n", BM_STACK_CAPACITY);
    printf("%%define BM_WORD_SIZE %d\n", BM_WORD_SIZE);
    printf("%%define STDOUT 1\n");
    printf("%%define SYS_EXIT 60\n");
    printf("%%define SYS_WRITE 1\n");
    printf("segment .text\n");
    printf("global _start\n");

    size_t jmp_if_escape_count = 0;
    for(size_t i = 0; i < basm.program_size; ++i) {
	Inst inst = basm.program[i];

	if(i == basm.entry) {
	    printf("_start:\n");
	}
	
	printf("inst_%zu:\n", i);
	switch(inst.type) {
	case INST_NOP: assert(false && "TODO: NOP is not implemented");
	case INST_PUSH: {
	    printf("    ;; push %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rsi, [stack_top]\n");
	    // printf("    mov QWORD [rsi], %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rax, 0x%"PRIX64"\n", inst.operand.as_u64);
	    printf("    mov QWORD [rsi], rax\n");
	    printf("    add QWORD [stack_top], BM_WORD_SIZE\n");
	} break;
	
	// * Drops the top of stack
	case INST_DROP: {
	    printf("    ;; drop\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;
	
	case INST_DUP: {
	    printf("    ;; dup %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rsi, [stack_top]\n");
	    // * Get the value which needs to be duplicated in rdi
	    printf("    mov rdi, rsi\n");
	    printf("    sub rdi, BM_WORD_SIZE * (%"PRIu64" + 1)\n", inst.operand.as_u64);
	    // * Copy the value to rsi via rax
	    printf("    mov rax, [rdi]\n");
	    printf("    mov [rsi], rax\n");
	    // * Incr the stack ptr
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;
	
	case INST_SWAP: {
	    printf("    ;; swap %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rdi, rsi\n");
	    printf("    sub rdi, BM_WORD_SIZE * %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rax, [rsi]\n");         // temp1 = a
	    printf("    mov rbx, [rdi]\n");         // temp2 = b
	    printf("    mov [rsi], rbx\n");         // a = temp2
	    printf("    mov [rdi], rax\n");         // b = temp1
	} break;
	
	case INST_PLUSI: {
	    printf("    ;; plusi\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");	    
	    printf("    add rax, rbx\n");
	    printf("    mov [rsi], rax\n");
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;
	
	case INST_MINUSI: {
	    printf("    ;; minusi\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");	    
	    printf("    sub rax, rbx\n");
	    printf("    mov [rsi], rax\n");
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;
	
	case INST_MULTI: {
	    printf("    ;; TODO multi\n");
	} break;

	case INST_MULTU: {
	    printf("    ;; TODO multu\n");
	} break;
	
	case INST_DIVI: {
	    printf("    ;; divi\n");
	    // * Get the 2nd argument into 'rbx'
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    // * Get the 1st argument into 'rax'
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    // * 'rdx' register will have remainder
	    printf("    xor rdx, rdx\n");
	    printf("    idiv rbx\n");
	    // * Quotient will be store in 'rax'
	    printf("    mov [rsi], rax\n");
	    // * store quotent to top of stack
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;

	case INST_MODI: {
	    printf("    ;; modi\n");
	    // * Get the 2nd argument into 'rbx'
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    // * Get the 1st argument into 'rax'
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    // * 'rdx' register will have remainder
	    printf("    xor rdx, rdx\n");
	    printf("    idiv rbx\n");
	    // * Quotient will be store in 'rax'
	    printf("    mov [rsi], rdx\n");
	    // * store quotent to top of stack
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;	
	
	// * Unsigned Division
	case INST_DIVU: {
	    printf("    ;; divu\n");
	    // * Get the 2nd argument into 'rbx'
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    // * Get the 1st argument into 'rax'
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    // * 'rdx' register will have remainder
	    printf("    xor rdx, rdx\n");
	    printf("    div rbx\n");
	    // * Quotient will be store in 'rax'
	    printf("    mov [rsi], rax\n");
	    // * store quotent to top of stack
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;
	
	// * Unsigned Modulus
	case INST_MODU: {
	    printf("    ;; modu\n");
	    // * Get the 2nd argument into 'rbx'
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    // * Get the 1st argument into 'rax'
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    // * 'rdx' register will have remainder
	    printf("    xor rdx, rdx\n");
	    printf("    div rbx\n");
	    // * Quotient will be store in 'rax'
	    printf("    mov [rsi], rdx\n");
	    // * store quotent to top of stack
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	} break;	


	case INST_PLUSF:  {
	    printf("    ;; TODO plusf\n");
	} break;
	case INST_MINUSF: {
	    printf("    ;; TODO minusf\n");
	} break;
	case INST_MULF:  {
	    printf("    ;; TODO mulf\n");
	} break;
	case INST_DIVF:  {
	    printf("    ;; TODO divf\n");
	} break;
	
	case INST_NATIVE: {
	    if(inst.operand.as_u64 == 3) {
		printf("    ;; native print_i64\n");
		printf("    call print_i64\n");
	    } else if(inst.operand.as_u64 == 7) {
		printf("    ;; native write\n");
		// * Move 'size' to 'rdx' register
		printf("    MOV r11, [stack_top]\n");
		printf("    sub r11, BM_WORD_SIZE\n");
		printf("    mov rdx, [r11]\n");

		// * offset from the memory
		printf("    sub r11, BM_WORD_SIZE\n");
		printf("    mov rsi, [r11]\n");
		printf("    add rsi, memory\n");
		
		// * Set file-descriptor to 'rdi' register
		printf("    mov rdi, STDOUT\n");
		printf("    mov rax, SYS_WRITE\n");
		printf("    mov [stack_top], r11\n");
		printf("    syscall\n");
	    }
	    else {
		assert(false && "Unsupported native functions\n");
	    }
	} break;

	case INST_RET: {
	    // * Takes the return addr from the top of stack
	    printf("    ;; ret\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    // * Copy the ret addr to rax register
	    printf("    mov rax, [rsi]\n");
	    // * offset the ret addr to specific inst
	    printf("    mov rbx, BM_WORD_SIZE\n");
	    printf("    mul rbx\n");
	    printf("    add rax, inst_map\n");
	    printf("    mov [stack_top], rsi\n");
	    printf("    jmp [rax]\n");
	    
	} break;
	
	case INST_CALL: {
	    printf("    ;; call\n");
	    // * Save the next addr to top of stack
	    printf("    mov rsi, [stack_top]\n");
	    printf("    mov QWORD [rsi], %zu\n", i + 1);    // * ip + 1
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");

	    // * jmp to the operand
	    printf("    mov rdi, inst_map\n");
	    printf("    add rdi, BM_WORD_SIZE * %"PRIu64"\n", inst.operand.as_u64);
	    printf("    jmp [rdi]\n");
	} break;

	case INST_JMP: {
	    printf("    ;; jmp %"PRIu64"\n", inst.operand.as_u64);
	    // * Address to Label Translation
	    printf("    mov rdi, inst_map\n");
	    printf("    add rdi, BM_WORD_SIZE * %"PRIu64"\n", inst.operand.as_u64);	    
	    printf("    jmp [rdi]\n");
	} break;	

	case INST_JMP_IF: {
	    printf("    ;; jmp_if %"PRIu64"\n", inst.operand.as_u64);
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    printf("    mov [stack_top], rsi\n");
	    printf("    cmp rax, 0\n");
	    printf("    je jmp_if_escape_%zu\n", jmp_if_escape_count);
	    
	    // * Address to Label Translation
	    printf("    mov rdi, inst_map\n");
	    printf("    add rdi, BM_WORD_SIZE * %"PRIu64"\n", inst.operand.as_u64);	    
	    printf("    jmp [rdi]\n");
	    
	    printf("jmp_if_escape_%zu:\n", jmp_if_escape_count);
	    jmp_if_escape_count += 1;
	} break;
		
	case INST_NOT:  {
	    printf("    ;; not\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");
	    printf("    cmp rax, 0\n");
	    printf("    mov rax, 0\n");
	    printf("    setz al\n");
	    printf("    mov [rsi], rax\n");
	} break;
	
	case INST_EQI: {
	    printf("    ;; eqi\n");
	    printf("    mov rsi, [stack_top]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rbx, [rsi]\n");
	    printf("    sub rsi, BM_WORD_SIZE\n");
	    printf("    mov rax, [rsi]\n");	    
	    printf("    cmp rax, rbx\n");
	    printf("    mov rax, 0\n");
	    // * Sets zero flag in al register
	    printf("    setz al\n");
	    printf("    mov [rsi], rax\n");
	    printf("    add rsi, BM_WORD_SIZE\n");
	    printf("    mov [stack_top], rsi\n");
	    
	} break;
	
	case INST_GEI:  {
	    printf("    ;; TODO gei\n");
	} break;
	case INST_GTI: {
	    printf("    ;; TODO gti\n");
	} break;
	case INST_LEI: {
	    printf("    ;; TODO lei\n");
	} break;
	case INST_LTI: assert(false && "TODO: LTI is not implemented");
	case INST_NEI: assert(false && "TODO: NEI is not implemented");

	// * Unsigned Comparisions
	case INST_EQU: {
	    printf("    ;; TODO equ\n");
	} break;
	
	case INST_GEU: assert(false && "TODO: GEU is not implemented");
	case INST_GTU: {
	    printf("    ;; TODO gtu\n");
	} break;
	case INST_LEU: {
	    printf("    ;; TODO leu\n");
	} break;
	case INST_LTU: assert(false && "TODO: LTU is not implemented");
	case INST_NEU: assert(false && "TODO: NEU is not implemented");

	// * Floating Comparisions
	case INST_EQF:  {
	    printf("    ;; TODO eqf\n");
	} break;
	case INST_GEF:  {
	    printf("    ;; TODO gef\n");
	} break;
	case INST_GTF:  {
	    printf("    ;; TODO gtf\n");
	} break;
	case INST_LEF:  {
	    printf("    ;; TODO lef\n");
	} break;
	case INST_LTF:  {
	    printf("    ;; TODO ltf\n");
	} break;
	
	case INST_NEF: {
	    printf("    ;; TODO nef\n");
	} break;
	
	case INST_HALT: {
	    printf("    ;; halt\n");	    
	    printf("    mov rax, SYS_EXIT\n");
	    printf("    mov rdi, 0\n");
	    printf("    syscall\n");
	} break;

	case INST_ANDB:  {
	    printf("    ;; TODO andb\n");
	} break;
	case INST_ORB: assert(false && "TODO: ORB is not implemented");
	case INST_XOR:  {
	    printf("    ;; TODO xor\n");
	} break;
	case INST_SHR: assert(false && "TODO: SHR is not implemented");
	case INST_SHL: assert(false && "TODO: SHL is not implemented");
	case INST_NOTB: assert(false && "TODO: NOTB is not implemented");
	case INST_READ8: {
	    printf("    ;; read8\n");
	    // * move addr of tos to r11 pointer
	    printf("    mov r11, [stack_top]\n");
	    printf("    sub r11, BM_WORD_SIZE\n");
	    // * Copy the addr from tos to rsi register
	    printf("    mov rsi, [r11]\n");
	    // * Add the offset from memory
	    printf("    add rsi, memory\n");
	    // * Read the BYTE to low 8 bits of rax register
	    printf("    xor rax, rax\n");
	    printf("    mov al, BYTE [rsi]\n");
	    // * Save the value to the tos
	    printf("    mov [r11], rax\n");
	} break;	
	case INST_READ16: assert(false && "TODO: READ16 is not implemented");
	case INST_READ32: assert(false && "TODO: READ32 is not implemented");
	case INST_READ64: assert(false && "TODO: READ64 is not implemented");
	case INST_WRITE8: {
	    printf("    ;; write8\n");
	    // * move addr of tos to r11 register
	    printf("    mov r11, [stack_top]\n");
	    printf("    sub r11, BM_WORD_SIZE\n");
	    
	    // * Get the value which needs to be written in rax register
	    printf("    mov rax, [r11]\n");

	    // * Copy the addr from top of stack to rsi register & offset memory
	    printf("    sub r11, BM_WORD_SIZE\n");
	    printf("    mov rsi, [r11]\n");
	    printf("    add rsi, memory\n");

	    // * mov rax low bit value to rsi register
	    printf("    mov BYTE [rsi], al\n");

	    // * Change the tos
	    printf("    mov [stack_top], r11\n");
	} break;
	case INST_WRITE16: assert(false && "TODO: WRITE16 is not implemented");
	case INST_WRITE32: assert(false && "TODO: WRITE32 is not implemented");
	case INST_WRITE64: assert(false && "TODO: WRITE64 is not implemented");
	case INST_I2F: {
	    printf("    ;; TODO i2f\n");
	} break;
	case INST_U2F: assert(false && "TODO: U2F is not implemented");
	case INST_F2I: {
	    printf("    ;; TODO f2i\n");
	} break;
	case INST_F2U: assert(false && "TODO: F2U is not implemented");
	case NUMBER_OF_INSTS:
	default:
	    assert(0 && "unknown instruction");
	}
    }

    printf("    ret\n");
    printf("segment .data\n");
    printf("stack_top: dq stack\n");
    printf("inst_map: dq");
    for(size_t i = 0; i < basm.program_size; ++i) {
	printf(" inst_%zu,", i);
    }
    printf("\n");

    // * Initialized Memory
    #define ROW_SIZE 10
    #define ROW_COUNT(size) ((size + ROW_SIZE - 1) / ROW_SIZE)
    printf("memory: \n");
    // * since basm.memory is contiguous stream of memory
    // * print memory in rows of size ROW_SIZE [2D Array]
    for(size_t row = 0; row < ROW_COUNT(basm.memory_size); ++row) {
	printf(" db ");
	for(size_t col = 0; col < ROW_SIZE && (row * ROW_SIZE + col) < basm.memory_size; ++col) {
	    printf(" %u,", basm.memory[row * ROW_SIZE + col]);
	}
	printf("\n");
    }
    // * Uninitialized Memory
    printf(" times %lu db 0\n", BM_MEMORY_CAPACITY - basm.memory_size);
    #undef ROW_SIZE
    #undef ROW_COUNT
    printf("segment .bss\n");
    printf("stack:	resq BM_STACK_CAPACITY\n");

    return 0;
}
