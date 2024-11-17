#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69

typedef int64_t Word;

typedef enum {
    INST_PUSH,
    INST_PLUS,
    INST_MINUS,
    INST_MUL,
    INST_DIV,
    INST_JMP,
    INST_HALT,
} Inst_Type;


// Instruction Structure
typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

typedef enum {
    TRAP_OK = 0,
    TRAP_STACK_OVERFLOW,
    TRAP_STACK_UNDERFLOW,
    TRAP_ILLEGAL_INST,
    ERR_ILLEGAL_INST_ACCESS,
    ERR_DIV_BY_ZERO,
} Trap;


// Machine Structure
typedef struct {
    Word stack[BM_STACK_CAPACITY];
    Word stack_size;
    
    Inst program[BM_PROGRAM_CAPACITY];
    Word program_size;
    Word ip;
    
    int halt;
} Bm;


#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAKE_INST_PUSH(value) { .type = INST_PUSH, .operand = value }
#define MAKE_INST_MUL() { .type = INST_MUL }
#define MAKE_INST_DIV() { .type = INST_DIV }
#define MAKE_INST_PLUS { .type = INST_PLUS }
#define MAKE_INST_MINUS() { .type = INST_MINUS }
#define MAKE_INST_JMP(addr) { .type = INST_JMP, .operand = addr }
#define MAKE_INST_HALT(addr) { .type = INST_HALT, .operand = addr }

const char *inst_type_as_cstr(Inst_Type type) {
    switch(type) {
    case INST_PUSH: return "INST_PUSH";
    case INST_PLUS: return "INST_PLUS";
    case INST_MINUS: return "INST_MINUS";
    case INST_DIV: return "INST_DIV";
    case INST_MUL: return "INST_MUL";
    case INST_JMP: return "INST_JMP";
    case INST_HALT: return "INST_HALT";
    default: assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

const char *trap_as_cstr(Trap trap) {
    switch(trap) {
    case TRAP_OK:
	return "TRAP_OK";
    case TRAP_STACK_OVERFLOW:
	return "TRAP_STACK_OVERFLOW";
    case TRAP_STACK_UNDERFLOW:
	return "TRAP_STACK_UNDERFLOW";
    case TRAP_ILLEGAL_INST:
	return "TRAP_ILLEGAL_INST";
    case ERR_DIV_BY_ZERO:
	return "ERR_DIV_BY_ZERO";
    case ERR_ILLEGAL_INST_ACCESS:
	return "ERR_ILLEGAL_INST_ACCESS";
    default:
	assert(0 && "trap_as_cstr: Unreachable");
    }
}

int bm_execute_inst(Bm *bm) {

    if(bm->ip < 0 || bm->ip >= bm->program_size) {
	return ERR_ILLEGAL_INST_ACCESS;
    }
    
    Inst inst = bm->program[bm->ip];
    switch(inst.type) {
    case INST_PUSH:
	if(bm->stack_size >= BM_STACK_CAPACITY) {
	    return TRAP_STACK_OVERFLOW;
	}
	bm->stack[bm->stack_size++] = inst.operand;
	bm->ip += 1;
	break;
    case INST_PLUS:
	if(bm->stack_size < 2) {
	    return TRAP_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] += bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_MINUS:
	if(bm->stack_size < 2) {
	    return TRAP_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] -= bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_MUL:
	if(bm->stack_size < 2) {
	    return TRAP_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] *= bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_DIV:
	if(bm->stack_size < 2) {
	    return TRAP_STACK_UNDERFLOW;
	}
	if(bm->stack[bm->stack_size - 1] == 0) {
	    return ERR_DIV_BY_ZERO;
	}
	bm->stack[bm->stack_size - 2] /= bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_JMP:
	bm->ip = inst.operand;
	break;
    case INST_HALT:
	bm->halt = 1;
	break;
    default:
	return TRAP_ILLEGAL_INST;
    }

    return TRAP_OK;
}

void bm_dump_stack(FILE *stream, const Bm *bm) {
    fprintf(stream, "Stack:\n");
    if(bm->stack_size > 0) {
	for(Word i = 0; i < bm->stack_size; ++i) {
	    fprintf(stream, "  %lld\n", bm->stack[i]);
       }
   } else {
       fprintf(stream, "  [empty]\n");
   }
}

void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size) {
    assert(bm->program_size < BM_PROGRAM_CAPACITY);
    memcpy(bm->program, program, sizeof(program[0]) * program_size);
    bm->program_size = program_size;
}


Bm bm = {0};
Inst program[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_PLUS,
    MAKE_INST_JMP(1),   
};


int main(void) {

    bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program));
    
    bm_dump_stack(stdout, &bm);
    for(int i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; ++i) {
	printf("%s\n", inst_type_as_cstr(program[bm.ip].type));
	Trap trap = bm_execute_inst(&bm);
	if(trap != TRAP_OK) {
	    fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
	    exit(1);
	}
	bm_dump_stack(stdout, &bm);
    }
    return 0;
}
