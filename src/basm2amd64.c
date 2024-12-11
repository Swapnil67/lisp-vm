#define BM_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "bm.h"

void usage(FILE *stream) {
    fprintf(stream, "Usage: ./basm2amd64 <input.basm> \n");
}

Basm basm;

int main(int argc, char *argv[]) {
    if(argc < 2) {
	usage(stderr);
	fprintf(stderr, "ERROR: no input provided.\n");
	exit(1);
    }

    
    basm_translate_source(&basm, cstr_as_sv(argv[1]));

   printf("BITS 64\n");
   printf("%%define SYS_EXIT 60\n");
   printf("segment .text\n");
   printf("_start:\n");

    for(size_t i = 0; i < basm.program_size; ++i) {
	Inst inst = basm.program[i];
	switch(inst.type) {
	
	case INST_NOP: assert(false && "TODO: NOP is not implemented");
	case INST_PUSH: assert(false && "TODO: PUSH is not implemented");
	case INST_DUP: assert(false && "TODO: DUP is not implemented");
	case INST_SWAP: assert(false && "TODO: SWAP is not implemented");
	case INST_PLUSI: assert(false && "TODO: PLUSI is not implemented");
	case INST_MINUSI: assert(false && "TODO: MINUSI is not implemented");
	case INST_MULI: assert(false && "TODO: MULI is not implemented");
	case INST_DIVI: assert(false && "TODO: DIVI is not implemented");
	case INST_MODI: assert(false && "TODO: MODI is not implemented");
	case INST_PLUSF: assert(false && "TODO: PLUSF is not implemented");
	case INST_MINUSF: assert(false && "TODO: MINUSF is not implemented");
	case INST_MULF: assert(false && "TODO: MULF is not implemented");
	case INST_DIVF: assert(false && "TODO: DIVF is not implemented");
	case INST_DROP: assert(false && "TODO: DROP is not implemented");
	case INST_RET: assert(false && "TODO: RET is not implemented");
	case INST_CALL: assert(false && "TODO: CALL is not implemented");
	case INST_NATIVE: assert(false && "TODO: NATIVE is not implemented");
	case INST_JMP: assert(false && "TODO: JMP is not implemented");
	case INST_JMP_IF: assert(false && "TODO: JMP_IF is not implemented");
	case INST_NOT: assert(false && "TODO: NOT is not implemented");
	case INST_EQI: assert(false && "TODO: EQI is not implemented");
	case INST_GEI: assert(false && "TODO: GEI is not implemented");
	case INST_GTI: assert(false && "TODO: GTI is not implemented");
	case INST_LEI: assert(false && "TODO: LEI is not implemented");
	case INST_LTI: assert(false && "TODO: LTI is not implemented");
	case INST_NEI: assert(false && "TODO: NEI is not implemented");
	case INST_EQF: assert(false && "TODO: EQF is not implemented");
	case INST_GEF: assert(false && "TODO: GEF is not implemented");
	case INST_GTF: assert(false && "TODO: GTF is not implemented");
	case INST_LEF: assert(false && "TODO: LEF is not implemented");
	case INST_LTF: assert(false && "TODO: LTF is not implemented");
    case INST_NEF:
	assert(false && "TODO: NEF is not implemented");
	
    case INST_HALT:
	printf("    mov rax, SYS_EXIT\n");
	printf("    mov rdi, 0\n");
	printf("    syscall\n");
	break;

	case INST_ANDB: assert(false && "TODO: ANDB is not implemented");
	case INST_ORB: assert(false && "TODO: ORB is not implemented");
	case INST_XOR: assert(false && "TODO: XOR is not implemented");
	case INST_SHR: assert(false && "TODO: SHR is not implemented");
	case INST_SHL: assert(false && "TODO: SHL is not implemented");
	case INST_NOTB: assert(false && "TODO: NOTB is not implemented");
	case INST_READ8: assert(false && "TODO: READ8 is not implemented");
	case INST_READ16: assert(false && "TODO: READ16 is not implemented");
	case INST_READ32: assert(false && "TODO: READ32 is not implemented");
	case INST_READ64: assert(false && "TODO: READ64 is not implemented");
	case INST_WRITE8: assert(false && "TODO: WRITE8 is not implemented");
	case INST_WRITE16: assert(false && "TODO: WRITE16 is not implemented");
	case INST_WRITE32: assert(false && "TODO: WRITE32 is not implemented");
	case INST_WRITE64: assert(false && "TODO: WRITE64 is not implemented");
	case INST_I2F: assert(false && "TODO: I2F is not implemented");
	case INST_U2F: assert(false && "TODO: U2F is not implemented");
	case INST_F2I: assert(false && "TODO: F2I is not implemented");
	case INST_F2U: assert(false && "TODO: F2U is not implemented");
	case NUMBER_OF_INSTS:
	default:
	    assert(0 && "unknown instruction");
	}
    }
    
    return 0;
}
