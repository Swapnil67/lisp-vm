#ifndef BM_H_
#define BM_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69
#define LABEL_CAPACITY 1024
#define DEFERED_OPERANDS_CAPACITY 1024
#define BM_NATIVES_CAPACITY 1024

typedef enum {
    ERR_OK = 0,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    TRAP_ILLEGAL_INST,
    ERR_ILLEGAL_INST_ACCESS,
    ERR_DIV_BY_ZERO,
    ERR_ILLEGAL_OPERAND,
} Err;

const char *err_as_cstr(Err err);

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_SWAP,
    
    INST_PLUSI,
    INST_MINUSI,
    INST_MULI,
    INST_DIVI,

    INST_PLUSF,
    INST_MINUSF,
    INST_MULF,
    INST_DIVF,

    INST_DROP,
    INST_RET,
    INST_CALL,
    INST_NATIVE,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_NOT,
    INST_GEF,
    INST_HALT,
    INST_PRINT_DEBUG,

    NUMBER_OF_INSTS
} Inst_Type;

const char *inst_name(Inst_Type type);
int inst_has_operand(Inst_Type type);


const char *inst_type_as_cstr(Inst_Type type);
typedef uint64_t Inst_Addr;

typedef union {
    uint64_t as_u64;
    int64_t as_i64;
    double as_f64;
    void *as_ptr;
} Word;

static_assert(sizeof(Word) == 8, "The BM's Word is expected to be 64 bits");

// Instruction Structure
typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

typedef struct Bm Bm;

typedef Err (*Bm_Native)(Bm*);

// Machine Structure
struct Bm {
    Word stack[BM_STACK_CAPACITY];
    uint64_t stack_size;
    
    Inst program[BM_PROGRAM_CAPACITY];
    uint64_t program_size;
    Inst_Addr ip;

    Bm_Native natives[BM_NATIVES_CAPACITY];
    size_t natives_size;

    int halt;
};

#define MAKE_INST_PUSH(value)	{ .type = INST_PUSH, .operand = value }
#define MAKE_INST_DUP(addr)	{ .type = INST_DUP, .operand = addr }
#define MAKE_INST_MULI()	{ .type = INST_MULI }
#define MAKE_INST_DIVI()	{ .type = INST_DIVI }
#define MAKE_INST_PLUSI		{ .type = INST_PLUSI }
#define MAKE_INST_MINUSI()	{ .type = INST_MINUSI }
#define MAKE_INST_JMP(addr)	{ .type = INST_JMP, .operand = addr }
#define MAKE_INST_HALT		{ .type = INST_HALT }

Err bm_execute_inst(Bm *bm);
Err bm_execute_program(Bm *bm, int limit);
void bm_push_native(Bm *bm, Bm_Native native);
void bm_dump_stack(FILE *stream, const Bm *bm);
void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size);
void bm_load_program_from_file(Bm *bm, const char *file_path);
void bm_save_program_to_file(const Bm *bm, const char *file_path);

typedef struct {
    size_t count;
    const char *data;
} String_View;


String_View cstr_as_sv(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);
int sv_eq(String_View a, String_View b);
int sv_to_int(String_View sv);
String_View sv_slurp_file(const char *file_path);

typedef struct {
    String_View name;
    Inst_Addr addr;
} Label;

// * Location of all the jumps that have unresolved labels
typedef struct {
    // * address of an inst the operand of which refers to a label
    Inst_Addr addr;
    String_View label;
} Defered_Operand;

typedef struct {
    Label labels[LABEL_CAPACITY];
    size_t labels_size;
    Defered_Operand defered_operands[DEFERED_OPERANDS_CAPACITY];
    size_t defered_operands_size;
} Basm;

Inst_Addr basm_find_label_addr(Basm *basm, String_View name);
void basm_push_label(Basm *basm, String_View name, Inst_Addr addr);
void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View label);
void print_unresolved_labels(const Basm *basm);
void print_labels(const Basm *basm);

void bm_translate_source(String_View source, Bm *bm, Basm *basm);
Word number_literal_as_word(String_View sv);

#endif // BM_H_


#ifdef BM_IMPLEMENTATION

const char *err_as_cstr(Err err) {
    switch(err) {
    case ERR_OK:
	return "ERR_OK";
    case ERR_STACK_OVERFLOW:
	return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW:
	return "ERR_STACK_UNDERFLOW";
    case TRAP_ILLEGAL_INST:
	return "TRAP_ILLEGAL_INST";
    case ERR_DIV_BY_ZERO:
	return "ERR_DIV_BY_ZERO";
    case ERR_ILLEGAL_INST_ACCESS:
	return "ERR_ILLEGAL_INST_ACCESS";
    case ERR_ILLEGAL_OPERAND:
	return "ERR_ILLEGAL_OPERAND";
    default:
	assert(0 && "err_as_cstr: Unreachable");
    }   
}

const char *inst_type_as_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP:		return "INST_NOP";
    case INST_PUSH:		return "INST_PUSH";
    case INST_DUP:		return "INST_DUP";
    case INST_SWAP:		return "INST_SWAP";

    case INST_PLUSI:		return "INST_PLUSI";
    case INST_MINUSI:		return "INST_MINUSI";
    case INST_DIVI:		return "INST_DIVI";
    case INST_MULI:		return "INST_MULI";

    case INST_PLUSF:		return "INST_PLUSF";
    case INST_MINUSF:		return "INST_MINUSF";
    case INST_DIVF:		return "INST_DIVF";
    case INST_MULF:		return "INST_MULF";

    case INST_DROP:		return "INST_DROP";
    case INST_RET:		return "INST_RET";
    case INST_CALL:		return "INST_CALL";
    case INST_NATIVE:		return "INST_NATIVE";
    case INST_JMP:		return "INST_JMP";
    case INST_JMP_IF:		return "INST_JMP_IF";
    case INST_EQ:		return "INST_EQ";
    case INST_NOT:		return "INST_NOT";
    case INST_GEF:		return "INST_GEF";
    case INST_HALT:		return "INST_HALT";
    case INST_PRINT_DEBUG:	return "INST_PRINT_DEBUT";
    case NUMBER_OF_INSTS:
    default:
	assert(0 && "inst_type_as_cstr: Unreachable");
    }
}


const char *inst_name(Inst_Type type) {
    switch(type) {
    case INST_NOP:		return "nop";
    case INST_PUSH:		return "push";
    case INST_DUP:		return "dup";
    case INST_SWAP:		return "swap";
    case INST_PLUSI:		return "plusi";
    case INST_MINUSI:		return "minusi";
    case INST_MULI:		return "muli";
    case INST_DIVI:		return "divi";
    case INST_PLUSF:		return "plusf";
    case INST_MINUSF:		return "minusf";
    case INST_MULF:		return "mulf";
    case INST_DIVF:		return "divf";	
    case INST_DROP:		return "drop";
    case INST_RET:		return "ret";
    case INST_CALL:		return "call";
    case INST_NATIVE:		return "native";
    case INST_JMP:		return "jmp";
    case INST_JMP_IF:		return "jmp_if";
    case INST_EQ:		return "eq";
    case INST_NOT :		return "not";
    case INST_GEF :		return "gef";
    case INST_HALT:		return "halt";
    case INST_PRINT_DEBUG:	return "print_debug";
    case NUMBER_OF_INSTS:
    default:
	assert(0 && "inst_name: unreachable");
    }
}


int inst_has_operand(Inst_Type type) {
    switch(type) {
    case INST_NOP:		return 0;
    case INST_PUSH:		return 1;
    case INST_DUP:		return 1;
    case INST_SWAP:		return 1;
    case INST_PLUSI:		return 0;
    case INST_MINUSI:		return 0;
    case INST_MULI:		return 0;
    case INST_DIVI:		return 0;
    case INST_PLUSF:		return 0;
    case INST_MINUSF:		return 0;
    case INST_MULF:		return 0;
    case INST_DIVF:		return 0;	
    case INST_DROP:		return 0;
    case INST_RET:		return 0;
    case INST_CALL:		return 1;
    case INST_NATIVE:		return 1;
    case INST_JMP:		return 1;
    case INST_JMP_IF:		return 1;
    case INST_EQ:		return 0;
    case INST_NOT:		return 0;
    case INST_GEF:		return 0;
    case INST_HALT:		return 0;
    case INST_PRINT_DEBUG:	return 0;
    case NUMBER_OF_INSTS:
    default:
	assert(0 && "inst_has_operand: unreachable");
    }
}


// * Execute basm program
Err bm_execute_program(Bm *bm, int limit) {
    while (limit != 0 && !bm->halt) {
	Err err = bm_execute_inst(bm);
	if (err != ERR_OK) {
	    return err;
	}
	if(limit > 0) {
	    --limit;
	}
    }
    return ERR_OK;
}

// * Execute Single Instruction
Err bm_execute_inst(Bm *bm) {
    if (bm->ip >= bm->program_size) {
	return ERR_ILLEGAL_INST_ACCESS;
    }

    // * Take the instruction to execute
    Inst inst = bm->program[bm->ip];
    // printf("%s\n", inst_type_as_cstr(inst.type));
    
    switch(inst.type) {
    case INST_NOP:
	bm->ip += 1;
	break;
	
    case INST_PUSH:
	if(bm->stack_size >= BM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}
	bm->stack[bm->stack_size++] = inst.operand;
	bm->ip += 1;
	break;

    case INST_DROP:
	if(bm->stack_size >= BM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case INST_DUP:
       if(bm->stack_size >= BM_STACK_CAPACITY) {
	   return ERR_STACK_OVERFLOW;
       }

       if(bm->stack_size - inst.operand.as_u64 <= 0) {
	   return ERR_STACK_UNDERFLOW;    
       }

       // * Push the operand to the top of stack  [Operand relative to current stack position]
       const uint64_t idx = bm->stack_size - 1 - inst.operand.as_u64;
       bm->stack[bm->stack_size].as_u64 = bm->stack[idx].as_u64;
       bm->stack_size += 1;
       bm->ip += 1;
       break;


   case INST_SWAP:
       if(inst.operand.as_u64 >= bm->stack_size) { 
	   return ERR_STACK_UNDERFLOW;
       }

       const uint64_t a = bm->stack_size - 1;
       const uint64_t b = bm->stack_size - 1 - inst.operand.as_u64;

       Word t = bm->stack[a];
       bm->stack[a] = bm->stack[b];
       bm->stack[b] = t;

       bm->ip += 1;
       break;


   case INST_PLUSI:
       if(bm->stack_size < 2) {
	   return ERR_STACK_UNDERFLOW;
       }
       bm->stack[bm->stack_size - 2].as_u64 += bm->stack[bm->stack_size - 1].as_u64;
       bm->stack_size -= 1;
       bm->ip += 1;
       break;

   case INST_MINUSI:
      if(bm->stack_size < 2) {
	  return ERR_STACK_UNDERFLOW;
      }
      bm->stack[bm->stack_size - 2].as_u64 -= bm->stack[bm->stack_size - 1].as_u64;
      bm->stack_size -= 1;
      bm->ip += 1;
      break;

  case INST_MULI:
      if(bm->stack_size < 2) {
	  return ERR_STACK_UNDERFLOW;
      }
      bm->stack[bm->stack_size - 2].as_u64 *= bm->stack[bm->stack_size - 1].as_u64;
      bm->stack_size -= 1;
      bm->ip += 1;
      break;

  case INST_DIVI:
      if(bm->stack_size < 2) {
	  return ERR_STACK_UNDERFLOW;
      }
      if(bm->stack[bm->stack_size - 1].as_u64 == 0) {
	  return ERR_DIV_BY_ZERO;
      }
      bm->stack[bm->stack_size - 2].as_u64 /= bm->stack[bm->stack_size - 1].as_u64;
      bm->stack_size -= 1;
      bm->ip += 1;
      break;
	  
  case INST_PLUSF:
     if(bm->stack_size < 2) {
	 return ERR_STACK_UNDERFLOW;
     }
     bm->stack[bm->stack_size - 2].as_f64 += bm->stack[bm->stack_size - 1].as_f64;
     bm->stack_size -= 1;
     bm->ip += 1;
     break;
	
    case INST_MINUSF:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2].as_f64 -= bm->stack[bm->stack_size - 1].as_f64;
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case INST_MULF:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
 	bm->stack[bm->stack_size - 2].as_f64 *= bm->stack[bm->stack_size - 1].as_f64;
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case INST_DIVF:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2].as_f64 /= bm->stack[bm->stack_size - 1].as_f64;
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case INST_RET:
	if(bm->stack_size < 1) {	    
	    return ERR_STACK_UNDERFLOW;
	}
	bm->ip = bm->stack[bm->stack_size - 1].as_u64;
	bm->stack_size -= 1;
	break;

    case INST_CALL:
	if(bm->stack_size >= BM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}

	// * Put the return address to the top of stack
	bm->stack[bm->stack_size++].as_u64 = bm->ip + 1;
	// * jump to the address of function
	bm->ip = inst.operand.as_u64;
	break;
	
    case INST_NATIVE:
	// * Trying to return non existing native function
	if(inst.operand.as_u64 > bm->natives_size) {
	    return ERR_ILLEGAL_OPERAND;
	}
	// * call the native functions
	bm->natives[inst.operand.as_u64](bm);
	bm->ip += 1;
	break;

    case INST_JMP:
	bm->ip = inst.operand.as_u64;
	break;

    case INST_JMP_IF:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	// * If top of the stack it True
	if(bm->stack[bm->stack_size - 1].as_u64) {
	    bm->ip = inst.operand.as_u64;
	}
	else {
	    bm->ip += 1;
	}
	bm->stack_size -= 1;
	break;

    case INST_EQ:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	uint64_t a1 = bm->stack_size - 1;
	uint64_t b1 = bm->stack_size - 2;
	bm->stack[b1].as_u64 = bm->stack[a1].as_u64 == bm->stack[b1].as_u64;
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case INST_GEF:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	uint64_t a2 = bm->stack_size - 1;
	uint64_t b2 = bm->stack_size - 2;
	bm->stack[b2].as_u64 = bm->stack[a2].as_u64 >= bm->stack[b2].as_u64;
	bm->stack_size -= 1;
	bm->ip += 1;
	break;


    case INST_NOT:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	uint64_t st_top = bm->stack_size - 1;
	bm->stack[st_top].as_u64 = !bm->stack[st_top].as_u64;
	bm->ip += 1;
	break;


    case INST_HALT:
	bm->halt = 1;
	break;

    case INST_PRINT_DEBUG:	    
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	fprintf(stdout, "u64:  %llu, i64: %lld, f64: %lf, ptr: %p\n",	
	    bm->stack[bm->stack_size - 1].as_u64,	
	    bm->stack[bm->stack_size - 1].as_i64,
	    bm->stack[bm->stack_size - 1].as_f64,
	    bm->stack[bm->stack_size - 1].as_ptr);

		  
	bm->stack_size -= 1;
	bm->ip += 1;
	break;

    case NUMBER_OF_INSTS:
    default:
	return TRAP_ILLEGAL_INST;
    }
    return ERR_OK;
}

void bm_push_native(Bm *bm, Bm_Native native) {
    assert(bm->natives_size < BM_NATIVES_CAPACITY);
    bm->natives[bm->natives_size++] = native;
}

void bm_dump_stack(FILE *stream, const Bm *bm) {
    fprintf(stream, "Stack:\n");
    if(bm->stack_size > 0) {
	for(Inst_Addr i = 0; i < bm->stack_size; ++i) {
	    fprintf(stream, "u64:  %llu, i64: %lld, f64: %lf, ptr: %p\n",
				bm->stack[i].as_u64,
				bm->stack[i].as_i64,
				bm->stack[i].as_f64,
				bm->stack[i].as_ptr);
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


// * Read from file -> bm->program buffer
void bm_load_program_from_file(Bm *bm, const char *file_path) {
    // * Open file in read binary mode
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    // fseek(FILE *stream, long offset, int whence);
    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    assert((m % sizeof(bm->program[0])) == 0);
    assert((size_t)m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0]));

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    // * Read the file into buffer
    // size_t fread(void * array_buffer, size_t size, size_t count, FILE * file_stream)
    bm->program_size = fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]) , f);

    // * Check if any errors
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

// * Save the program to file
void bm_save_program_to_file(const Bm *bm, const char *file_path)
{
    FILE *f  = fopen(file_path, "wb");
    // printf("%ld", sizeof(program[1]));
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    // * size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
    fwrite(bm->program, sizeof(bm->program[0]), bm->program_size, f);

    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

String_View cstr_as_sv(const char *cstr) {
    return (String_View) {
	.count = strlen(cstr),
	.data = cstr,
    };
}

// * Trim spaces from left
String_View sv_trim_left(String_View sv) {
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[i])) {
	i += 1;
    }
    return (String_View) {
	.count = sv.count - i,
	.data = sv.data + i
    };
}

// * Trim spaces from right
String_View sv_trim_right(String_View sv) {
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
	i += 1;
    }

    return (String_View) {
	.count = sv.count - i,
	.data = sv.data,
    };
}

String_View sv_trim(String_View sv) {
    return sv_trim_right(sv_trim_left(sv));
}


String_View sv_chop_by_delim(String_View *sv, char delim) {

    size_t i = 0;
    while(i < sv->count && sv->data[i] != delim) {
	i += 1;
    }

//    printf("%ld - %ld\n", sv->count, i); 

    String_View result = {
	.count = i,
	.data = sv->data,
    };
    
    if(i < sv->count) {
	sv->count -= i + 1;
	sv->data += i + 1;
    }
    else {
	sv->count -= i;
	sv->data += i;
    }

    return result;
}


int sv_to_int(String_View sv) {
    int result = 0;
    for(size_t i = 0; (i < sv.count && isdigit(sv.data[i])); ++i) {
	result = result * 10 + sv.data[i] - '0';
    }
    return result;
}

int sv_eq(String_View a, String_View b) {
    if(a.count != b.count) {
	return 0;
    }
    else {
	return memcmp(a.data, b.data, a.count) == 0;
    }
}

// * Find the address of a particular label in Basm 
Inst_Addr basm_find_label_addr(Basm *basm, String_View name) {
    for(size_t i = 0; i < basm->labels_size; ++i) {
	if(sv_eq(basm->labels[i].name, name)) {
	    return basm->labels[i].addr;
	}
    }
    fprintf(stderr, "ERROR: label `%.*s` does not exists\n",
    (int) name.count, name.data);
    exit(1);

}

void basm_push_label(Basm *basm, String_View name, Inst_Addr addr) {
    assert(basm->labels_size < LABEL_CAPACITY);
    basm->labels[basm->labels_size++] = (Label){ .name = name, .addr = addr };
}

void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View label) {
    assert(basm->defered_operands_size < DEFERED_OPERANDS_CAPACITY);
    basm->defered_operands[basm->defered_operands_size++] = (Defered_Operand) {
	.addr = addr,
	.label = label
    };
}

void print_labels(const Basm *basm) {
    printf("-------- LABELS: -------\n");
    for(size_t i = 0; i < basm->labels_size; ++i) {
	printf("%.*s ->  %llu\n",
	(int) basm->labels[i].name.count,
	basm->labels[i].name.data,
	basm->labels[i].addr);
    }
}
 
void print_unresolved_labels(const Basm *basm) {
    printf("-------- UNRESOLVED_JMPS: -------\n");
    for(size_t i = 0; i < basm->defered_operands_size; ++i) {
	printf("%lld -> %.*s\n",
	basm->defered_operands[i].addr,
	(int) basm->defered_operands[i].label.count,
	basm->defered_operands[i].label.data);
    }
}

Word number_literal_as_word(String_View sv) {
    assert(sv.count < 1024);
    char cstr[sv.count + 1];
    char *endptr = 0;
    
    memcpy(cstr, sv.data, sv.count);
    cstr[sv.count] = '\0';

    Word result = {0};

    // * Try to parse it as uint64_t first
    result.as_u64 = strtoull(cstr, &endptr, 10);

    if((size_t)(endptr - cstr) != sv.count) {
	// * Try to parse it as double
	result.as_f64 = strtod(cstr, &endptr);
	if((size_t)(endptr - cstr) != sv.count) {
	    fprintf(stderr, "ERROR: `%s` is not a number literal\n", cstr);
	    exit(1);
	}
    }
    
    return result;
}

void bm_translate_source(String_View source, Bm *bm, Basm *basm) {
    bm->program_size = 0;
    while(source.count > 0) {
	assert(bm->program_size < BM_PROGRAM_CAPACITY);
	String_View line = sv_trim(sv_chop_by_delim(&source, '\n'));
	
	if(line.count > 0 && *line.data != '#') {
	  // printf("#%.*s#\n", (int) line.count, line.data);
	    
	   String_View token = sv_chop_by_delim(&line, ' ');
	   // printf("#%.*s#\n", (int) token.count, token.data);
		   
	   // * check if there is any label
	   if(token.count > 0 && token.data[token.count - 1] == ':') {
	       String_View label = {
		   .count = token.count - 1,		   
		   .data = token.data
	       };
	       basm_push_label(basm, label, bm->program_size);
	       // * Check any inst after ':'
	       token = sv_trim(sv_chop_by_delim(&line, ' '));
	   }
	   
	   if(token.count > 0) {
	       String_View operand = sv_trim(sv_chop_by_delim(&line, '#'));
	       if(sv_eq(token, cstr_as_sv(inst_name(INST_NOP)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_NOP,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_PUSH)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_PUSH,
		       .operand = number_literal_as_word(operand)
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_DROP)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_DROP,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_DUP)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_DUP,
		       .operand = { .as_i64 = sv_to_int(operand) }
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_PLUSI)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_PLUSI,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_MULI)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_MULI,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_MINUSI)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_MINUSI,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_DIVI)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_DIVI,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_EQ)))) {		   
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_EQ,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_GEF)))) {		   
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_GEF,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_NOT)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_NOT,
		   };
	       }	       
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_JMP)))) {
		   if(operand.count > 0 && isdigit(*operand.data)) {
		       // * operand as absolute address
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_JMP,
			   .operand = { .as_i64 = sv_to_int(operand) },
		       };
		   } else {
		       // * operand as label
		       basm_push_defered_operand(basm, bm->program_size, operand);
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_JMP,
		       };
		   }
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_JMP_IF)))) {
		   if(operand.count > 0 && isdigit(*operand.data)) {
		       // * operand as absolute address
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_JMP_IF,
			   .operand = { .as_i64 = sv_to_int(operand) },
		       };
		   } else {
		       // * operand as label
		       basm_push_defered_operand(basm, bm->program_size, operand);
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_JMP_IF,
		       };
		   }
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_CALL)))) {
		   if(operand.count > 0 && isdigit(*operand.data)) {
		       // * operand as absolute address
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_CALL,
			   .operand = { .as_i64 = sv_to_int(operand) },
		       };
		   } else {
		       // * operand as label
		       basm_push_defered_operand(basm, bm->program_size, operand);
		       bm->program[bm->program_size++] = (Inst) {
			   .type = INST_CALL,
		       };
		   }
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_PLUSF)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_PLUSF,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_MINUSF)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_MINUSF,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_MULF)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_MULF,
		   };
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_DIVF)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_DIVF,
		   };
	       }	       
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_HALT)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_HALT,
		   };		   
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_SWAP)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_SWAP,
		       .operand = { .as_i64 = sv_to_int(operand) }
		   };		   
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_PRINT_DEBUG)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_PRINT_DEBUG
		   };		   
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_RET)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_RET
		   };		   
	       }
	       else if(sv_eq(token, cstr_as_sv(inst_name(INST_NATIVE)))) {
		   bm->program[bm->program_size++] = (Inst) {
		       .type = INST_NATIVE,
		       .operand = { .as_i64 = sv_to_int(operand) }
		   };		   
	       }
	       else {
		   fprintf(stderr, "ERROR: unknown instruction `%.*s`\n",
		   (int) token.count, token.data);
		   exit(1);
	       }
	       
	  }
      }
  }

 // print_labels(basm);
 // print_unresolved_labels(basm);

  // * Dereferencing the jump labels to address
  for(size_t i = 0; i < basm->defered_operands_size; ++i) {
      Inst_Addr addr = basm_find_label_addr(basm, basm->defered_operands[i].label);
      // printf("Addr : %lld\n", addr);
      // printf("label : %s\n", basm->defered_operands[i].label.data);
      // printf("label addr : %lld\n", basm->defered_operands[i].addr);
      // printf("bm->program[6].tyep : %s\n", inst_type_as_cstr(bm->program[6].type));
//      printf("label : %s\n", inst_type_as_cstr(bm->program[basm->defered_operands[i].addr].type));
     bm->program[basm->defered_operands[i].addr].operand.as_u64 = addr;
 }
}


String_View sv_slurp_file(const char *file_path)
{
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    // fseek(FILE *stream, long offset, int whence);
    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if(m < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    char *buffer = malloc(m);
    if(buffer == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for file: %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);

    return  (String_View) { .count = n, .data = buffer };
}

#endif // BM_IMPLEMENTATION
