#ifndef BM_H_
#define BM_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(__GNUC__) || defined(__clang__)
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#  error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/tsoding/bng"
#endif

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))

#define BM_WORD_SIZE 8
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69
#define BASM_BINDINGS_CAPACITY 1024
#define DEFERED_OPERANDS_CAPACITY 1024
#define BM_NATIVES_CAPACITY 1024
#define BM_MEMORY_CAPACITY (640 * 1000) // * 640KB
// #define BM_MEMORY_CAPACITY 20

#define BASM_COMMENT_SYMBOL ';'
#define BASM_PP_SYMBOL '%'
#define BASM_MAX_INCLUDE_LEVEL 69
#define BASM_ARENA_CAPACITY (100 * 1000 * 1000) // * 100MB

typedef enum {
    ERR_OK = 0,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    TRAP_ILLEGAL_INST,
    ERR_ILLEGAL_INST_ACCESS,
    ERR_DIV_BY_ZERO,
    ERR_ILLEGAL_OPERAND,
    ERR_ILLEGAL_MEMORY_ACCESS,
} Err;

const char *err_as_cstr(Err err);

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_SWAP,
    
    INST_PLUSI,
    INST_MINUSI,

    // * Signed Instructions
    INST_MULTI,
    INST_DIVI,
    INST_MODI,

    // * Unsigned Instructions
    INST_MULTU,
    INST_DIVU,
    INST_MODU,
    
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
    INST_NOT,
    
    INST_EQI,
    INST_GEI,
    INST_GTI,
    INST_LEI,
    INST_LTI,
    INST_NEI,
    
    INST_EQU,
    INST_GEU,
    INST_GTU,
    INST_LEU,
    INST_LTU,
    INST_NEU,
    
    INST_EQF,
    INST_GEF,
    INST_GTF,
    INST_LEF,
    INST_LTF,
    INST_NEF,
    
    INST_HALT,

    INST_ANDB,
    INST_ORB,
    INST_XOR,
    INST_SHR,
    INST_SHL,
    INST_NOTB,

    INST_READ8,
    INST_READ16,
    INST_READ32,
    INST_READ64,
    
    INST_WRITE8,
    INST_WRITE16,
    INST_WRITE32,
    INST_WRITE64,

    INST_I2F,
    INST_U2F,
    INST_F2I,
    INST_F2U,
  
    NUMBER_OF_INSTS
} Inst_Type;

typedef struct {
    size_t count;
    const char *data;
} String_View;

#define SV_Fmt ".*s"
#define SV_Arg(sv) (int) sv.count, sv.data
// USAGE:
    // String_View name = ...
    // printf("Name :%"SV_Fmt"\n", SV_Arg(name))

String_View cstr_as_sv(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);
int sv_eq(String_View a, String_View b);
int sv_to_int(String_View sv);

int inst_by_name(String_View name, Inst_Type *output);
const char *inst_name(Inst_Type type);
int inst_has_operand(Inst_Type type);

const char *inst_type_as_cstr(Inst_Type type);

typedef union {
    uint64_t as_u64;
    int64_t as_i64;
    double as_f64;
    void *as_ptr;
} Word;

Word word_u64(uint64_t u64);
Word word_i64(int64_t i64);
Word word_f64(double f64);
Word word_ptr(void *ptr);

static_assert(sizeof(Word) == BM_WORD_SIZE, "The BM's Word is expected to be 64 bits");

typedef uint64_t Inst_Addr;
typedef uint64_t Memory_Addr;
typedef size_t Arena;

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

    uint8_t memory[BM_MEMORY_CAPACITY];

    int halt;
};

Err bm_execute_inst(Bm *bm);
Err bm_execute_program(Bm *bm, int limit);
void bm_push_native(Bm *bm, Bm_Native native);
void bm_dump_stack(FILE *stream, const Bm *bm);
void bm_load_program_from_file(Bm *bm, const char *file_path);

#define BM_FILE_MAGIC 0x4D42
#define BM_FILE_VERSION 1

PACK(struct Bm_File_Meta {
    uint16_t magic;
    uint16_t version;
    uint64_t program_size;
    uint64_t entry;
    uint64_t memory_size;
    uint64_t memory_capacity;
});

typedef struct Bm_File_Meta Bm_File_Meta;

typedef struct {
    String_View name;
    Word value;
} Binding;

// * Location of all the jumps that have unresolved names
typedef struct {
    // * address of an inst the operand of which refers to a name
    Inst_Addr addr;
    String_View name;
} Defered_Operand;

typedef struct {
    Binding bindings[BASM_BINDINGS_CAPACITY];
    size_t bindings_size;
    
    Defered_Operand defered_operands[DEFERED_OPERANDS_CAPACITY];
    size_t defered_operands_size;

    Inst program[BM_PROGRAM_CAPACITY];
    uint64_t program_size;
    uint64_t entry;
    bool has_entry;
    String_View deferred_entry_binding_name;

    uint8_t memory[BM_MEMORY_CAPACITY];
    size_t memory_size;
    size_t memory_capacity;

    char arena[BASM_ARENA_CAPACITY];
    Arena arena_size;

    size_t include_level;
} Basm;


void* arena_sv_to_cstr(Basm *basm, String_View sv);
void *basm_alloc(Basm *basm, size_t size);
String_View basm_slurp_file(Basm *basm, String_View file_path);
int basm_resolve_binding(Basm *basm, String_View name, Word *output);
int basm_bind_value(Basm *basm, String_View name, Word word);
void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View name);
void print_unresolved_names(const Basm *basm);
void print_names(const Basm *basm);

void basm_save_to_file(Basm *basm, const char *file_path);
void basm_translate_source(Basm *basm, String_View input_file_path);
Word basm_push_string_to_memory(Basm *basm, String_View sv);
bool basm_translate_literal(Basm *basm, String_View sv, Word *output);

#endif // BM_H_


#ifdef BM_IMPLEMENTATION

Word word_u64(uint64_t u64) {
    return (Word) { .as_u64 = u64 };
}

Word word_i64(int64_t i64) {
    return (Word) { .as_i64 = i64 };
}

Word word_f64(double f64) {
    return (Word) { .as_f64 = f64 };
}

Word word_ptr(void *ptr) {
    return (Word) { .as_ptr = ptr };
}

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
    case ERR_ILLEGAL_MEMORY_ACCESS:
	return "ERR_ILLEGAL_MEMORY_ACCESS";
    default:
	assert(0 && "err_as_cstr: Unreachable");
    }   
}

const char *inst_type_as_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return "INST_NOP";
    case INST_PUSH:	return "INST_PUSH";
    case INST_DUP:	return "INST_DUP";
    case INST_SWAP:	return "INST_SWAP";

    case INST_PLUSI:	return "INST_PLUSI";
    case INST_MINUSI:	return "INST_MINUSI";
    
    case INST_DIVI:	return "INST_DIVI";
    case INST_MULTI:	return "INST_MULTI";
    case INST_MODI:	return "INST_MODI";    

    case INST_DIVU:	return "INST_DIVU";
    case INST_MULTU:	return "INST_MULTU";
    case INST_MODU:	return "INST_MODU";
    
    case INST_PLUSF:	return "INST_PLUSF";
    case INST_MINUSF:	return "INST_MINUSF";
    case INST_DIVF:	return "INST_DIVF";
    case INST_MULF:	return "INST_MULF";

    case INST_DROP:	return "INST_DROP";
    case INST_RET:	return "INST_RET";
    case INST_CALL:	return "INST_CALL";
    case INST_NATIVE:	return "INST_NATIVE";
    case INST_JMP:	return "INST_JMP";
    case INST_JMP_IF:	return "INST_JMP_IF";

    case INST_NOT:	return "INST_NOT";
    case INST_HALT:	return "INST_HALT";

    case INST_EQI:	return "INST_EQI";
    case INST_GEI:	return "INST_GEI";
    case INST_GTI:	return "INST_GTI";
    case INST_LEI:	return "INST_LEI";
    case INST_LTI:	return "INST_LTI";
    case INST_NEI:	return "INST_NEI";

    case INST_EQU:	return "INST_EQU";
    case INST_GEU:	return "INST_GEU";
    case INST_GTU:	return "INST_GTU";
    case INST_LEU:	return "INST_LEU";
    case INST_LTU:	return "INST_LTU";
    case INST_NEU:	return "INST_NEU";
    
    case INST_EQF:	return "INST_EQF";
    case INST_GEF:	return "INST_GEF";
    case INST_GTF:	return "INST_GTF";
    case INST_LEF:	return "INST_LEF";
    case INST_LTF:	return "INST_LTF";
    case INST_NEF:	return "INST_NEF";
        
    case INST_ANDB:	return "INST_ANDB";
    case INST_ORB:	return "INST_ORB";
    case INST_XOR:	return "INST_XOR";
    case INST_SHR:	return "INST_SHR";
    case INST_SHL:	return "INST_SHL";
    case INST_NOTB:	return "INST_NOTB";

    case INST_READ8:	return "INST_READ8";
    case INST_READ16:	return "INST_READ16";
    case INST_READ32:	return "INST_READ32";
    case INST_READ64:	return "INST_READ64";

    case INST_WRITE8:	return "INST_WRITE8";
    case INST_WRITE16:	return "INST_WRITE16";
    case INST_WRITE32:	return "INST_WRITE32";
    case INST_WRITE64:	return "INST_WRITE64";

    case INST_I2F:      return "INST_I2F";
    case INST_U2F:      return "INST_U2F";
    case INST_F2I:      return "INST_F2I";
    case INST_F2U:      return "INST_F2U";
       
    case NUMBER_OF_INSTS:
    default:
	assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

int inst_by_name(String_View name, Inst_Type *output) {
    for(Inst_Type type = (Inst_Type) 0; type < NUMBER_OF_INSTS; ++type) {
	if(sv_eq(cstr_as_sv(inst_name(type)), name)) {
	    *output = type;
	    return 1;
	}
    }
    return 0;
}

const char *inst_name(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return "nop";
    case INST_PUSH:	return "push";
    case INST_DUP:	return "dup";
    case INST_SWAP:	return "swap";
    
    case INST_PLUSI:	return "plusi";
    case INST_MINUSI:	return "minusi";
    case INST_MULTI:	return "multi";
    case INST_DIVI:	return "divi";
    case INST_MODI:	return "modi";
    case INST_MULTU:	return "multu";
    case INST_DIVU:	return "divu";
    case INST_MODU:	return "modu";
    
    case INST_PLUSF:	return "plusf";
    case INST_MINUSF:	return "minusf";
    case INST_MULF:	return "mulf";
    case INST_DIVF:	return "divf";
    
    case INST_DROP:	return "drop";
    case INST_RET:	return "ret";
    case INST_CALL:	return "call";
    case INST_NATIVE:	return "native";
    case INST_JMP:	return "jmp";
    case INST_JMP_IF:	return "jmp_if";

    case INST_EQI:	return "eqi";
    case INST_GEI:	return "gei";
    case INST_GTI:	return "gti";
    case INST_LEI:	return "lei";
    case INST_LTI:	return "lti";
    case INST_NEI:	return "nei";

    case INST_EQU:	return "equ";
    case INST_GEU:	return "geu";
    case INST_GTU:	return "gtu";
    case INST_LEU:	return "leu";
    case INST_LTU:	return "ltu";
    case INST_NEU:	return "neu";

    case INST_EQF:	return "eqf";
    case INST_GEF:	return "gef";
    case INST_GTF:	return "gtf";
    case INST_LEF:	return "lef";
    case INST_LTF:	return "ltf";
    case INST_NEF:	return "nef";

    case INST_NOT:	return "not";
    case INST_HALT:	return "halt";
    case INST_ANDB:	return "andb";
    case INST_ORB:	return "orb";
    case INST_XOR:	return "xor";
    case INST_SHR:	return "shr";
    case INST_SHL:	return "shl";
    case INST_NOTB:	return "notb";

    case INST_READ8:    return "read8";
    case INST_READ16:   return "read16";
    case INST_READ32:   return "read32";
    case INST_READ64:   return "read64";

    case INST_WRITE8:   return "write8";
    case INST_WRITE16:  return "write16";
    case INST_WRITE32:  return "write32";
    case INST_WRITE64:  return "write64";

    case INST_I2F:      return "i2f";
    case INST_U2F:      return "u2f";
    case INST_F2I:      return "f2i";
    case INST_F2U:      return "f2u";
    
    case NUMBER_OF_INSTS:
    default:
	assert(0 && "inst_name: unreachable");
    }
}

int inst_has_operand(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return false;
    case INST_PUSH:	return true;
    case INST_DUP:	return true;
    case INST_SWAP:	return true;
    
    case INST_PLUSI:	return false;
    case INST_MINUSI:	return false;
    case INST_MULTI:	return false;
    case INST_DIVI:	return false;
    case INST_MODI:	return false;
    case INST_MULTU:	return false;
    case INST_DIVU:	return false;
    case INST_MODU:	return false;
    
    case INST_PLUSF:	return false;
    case INST_MINUSF:	return false;
    case INST_MULF:	return false;
    case INST_DIVF:	return false;	
    case INST_DROP:	return false;
    case INST_RET:	return false;
    case INST_CALL:	return true;
    case INST_NATIVE:	return true;
    case INST_JMP:	return true;
    case INST_JMP_IF:	return true;
    case INST_NOT:	return false;
    case INST_HALT:	return false;

    case INST_EQI:	return false;
    case INST_GEI:	return false;
    case INST_GTI:	return false;
    case INST_LEI:	return false;
    case INST_LTI:	return false;
    case INST_NEI:	return false;

    case INST_EQU:	return false;
    case INST_GEU:	return false;
    case INST_GTU:	return false;
    case INST_LEU:	return false;
    case INST_LTU:	return false;
    case INST_NEU:	return false;
    
    case INST_EQF:	return false;
    case INST_GEF:	return false;
    case INST_GTF:	return false;
    case INST_LEF:	return false;
    case INST_LTF:	return false;
    case INST_NEF:	return false;

    case INST_ANDB:	return false;
    case INST_ORB:	return false;
    case INST_XOR:	return false;
    case INST_SHR:	return false;
    case INST_SHL:	return false;
    case INST_NOTB:	return false;

    case INST_READ8:    return false;
    case INST_READ16:   return false;
    case INST_READ32:   return false;
    case INST_READ64:   return false;

    case INST_WRITE8:   return false;
    case INST_WRITE16:  return false;
    case INST_WRITE32:  return false;
    case INST_WRITE64:  return false;

    case INST_I2F:      return false;
    case INST_U2F:      return false;
    case INST_F2I:      return false;
    case INST_F2U:      return false;
    
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


#define BINARY_OP(bm, in, out, op)														\
do {																		\
    if((bm)->stack_size < 2) {															\
	return ERR_STACK_UNDERFLOW;														\
    }																		\
    (bm)->stack[(bm)->stack_size - 2].as_##out = (bm)->stack[(bm)->stack_size - 2].as_##in op (bm)->stack[(bm)->stack_size - 1].as_##in;	\
    (bm)->stack_size -= 1;															\
    (bm)->ip += 1;																\
} while(false)																	\

#define CAST_OP(bm, src, dst, cast)            \
do {                                           \
    if((bm)->stack_size < 1) {		       \			
	return ERR_STACK_UNDERFLOW;	       \
    }					       \
    (bm)->stack[(bm)->stack_size - 1].as_##dst = cast (bm)->stack[(bm)->stack_size - 1].as_##src; \
    (bm)->ip += 1;                             \
} while (false);                               \


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
	BINARY_OP(bm, u64, u64, +);
	break;	

    case INST_MINUSI:
	BINARY_OP(bm, u64, u64, -);
	break;
	
    // * Signed Multiplication	
    case INST_MULTI:
	BINARY_OP(bm, i64, i64, *);
	break;

    // * Unsigned Multiplication	
    case INST_MULTU:
	BINARY_OP(bm, u64, u64, *);
	break;

    // * Signed Division	
    case INST_DIVI: {
	if(bm->stack[bm->stack_size - 1].as_i64 == 0) {
	    return ERR_DIV_BY_ZERO;
	}
	BINARY_OP(bm, i64, i64, /);
    } break;

    // * Unsigned Division
    case INST_DIVU: {
	if(bm->stack[bm->stack_size - 1].as_u64 == 0) {
	    return ERR_DIV_BY_ZERO;
	}
	BINARY_OP(bm, u64, u64, /);
    } break;

    // * Signed Modulus
    case INST_MODI: {
	if(bm->stack[bm->stack_size - 1].as_i64 == 0) {
	    return ERR_DIV_BY_ZERO;
	}
	BINARY_OP(bm, i64, i64, %);
    } break;

    // * Unsigned Modulus
    case INST_MODU: {
	if(bm->stack[bm->stack_size - 1].as_u64 == 0) {
	    return ERR_DIV_BY_ZERO;
	}
	BINARY_OP(bm, u64, u64, %);
    } break;
    
    case INST_PLUSF:
	BINARY_OP(bm, f64, f64, +);
	break;
	
    case INST_MINUSF:
	BINARY_OP(bm, f64, f64, -);
	break;

    case INST_MULF:
	BINARY_OP(bm, f64, f64, *);
	break;

    case INST_DIVF:
	BINARY_OP(bm, f64, f64, /);
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

    // * Signed Comparisions
    case INST_EQI:
	BINARY_OP(bm, i64, u64, ==);
	break;

    case INST_GEI:
	BINARY_OP(bm, i64, u64, >=);
	break;

    case INST_GTI:
	BINARY_OP(bm, i64, u64, >);
	break;

    case INST_LEI:
	BINARY_OP(bm,i64, u64, <=);
	break;
	
    case INST_LTI:
	BINARY_OP(bm, i64, u64, <);
	break;

    case INST_NEI:
	BINARY_OP(bm, i64, u64, !=);
	break;

    // * unsigned Comparisions
    case INST_EQU:
	BINARY_OP(bm, u64, u64, ==);
	break;

    case INST_GEU:
	BINARY_OP(bm, u64, u64, >=);
	break;

    case INST_GTU:
	BINARY_OP(bm, u64, u64, >);
	break;

    case INST_LEU:
	BINARY_OP(bm, u64, u64, <=);
	break;
	
    case INST_LTU:
	BINARY_OP(bm, u64, u64, <);
	break;

    case INST_NEU:
	BINARY_OP(bm, u64, u64, !=);
	break;

    // * Floating Comparisions	
    case INST_EQF:
	BINARY_OP(bm, f64, u64, ==);
	break;
	
    case INST_GEF:
	BINARY_OP(bm, f64, u64, >=);
	break;

    case INST_GTF:
	BINARY_OP(bm, f64, u64, >);
	break;

    case INST_LEF:
	BINARY_OP(bm, f64, u64, <=);
	break;
	
    case INST_LTF:
	BINARY_OP(bm, f64, u64, <);
	break;
	
    case INST_NEF:
	BINARY_OP(bm, f64, u64, !=);
	break;
		

    case INST_NOT:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	uint64_t st_top = bm->stack_size - 1;
	bm->stack[st_top].as_u64 = !bm->stack[st_top].as_u64;
	bm->ip += 1;
	break;


    case INST_ANDB:
	BINARY_OP(bm, u64, u64, &);
	break;

    case INST_ORB:
	BINARY_OP(bm, u64, u64, |);
	break;

    case INST_XOR:
	BINARY_OP(bm, u64, u64, ^);
	break;

    case INST_SHR:
	BINARY_OP(bm, u64, u64, >>);
	break;
	
    case INST_SHL:
	BINARY_OP(bm, u64, u64, <<);	
	break;

    case INST_NOTB:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 1].as_u64 = ~bm->stack[bm->stack_size - 1].as_u64;
	bm->ip += 1;
	break;
		
    case INST_HALT:
	bm->halt = 1;
	break;

    case INST_READ8: {
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
	if(addr >= BM_MEMORY_CAPACITY) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}
	bm->stack[bm->stack_size - 1].as_u64 = bm->memory[addr];
	bm->ip += 1;
    }	break;

    case INST_READ16: {
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 1) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}
	bm->stack[bm->stack_size - 1].as_u64 = *(uint16_t*)bm->memory[addr];
	bm->ip += 1;
    }	break;

    case INST_READ32: {
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 3) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}
	bm->stack[bm->stack_size - 1].as_u64 = *(uint32_t*)bm->memory[addr];
	bm->ip += 1;
    } break;

    case INST_READ64: {
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 7) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}
	bm->stack[bm->stack_size - 1].as_u64 = *(uint64_t*)bm->memory[addr];
	bm->ip += 1;
    }	break;

    case INST_WRITE8: {
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}

	const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 1) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}

	bm->memory[addr] = (uint8_t) bm->stack[bm->stack_size - 1].as_u64;
	bm->stack_size -= 2;
	bm->ip += 1;
    }   break;
    

    case INST_WRITE16: {
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}

	const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 1) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}

	*(uint16_t*)&bm->memory[addr] = (uint16_t) bm->stack[bm->stack_size - 1].as_u64;
	bm->stack_size -= 2;
	bm->ip += 1;
    }	break;
    

    case INST_WRITE32: {
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}

	const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 3) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}

	*(uint32_t*)&bm->memory[addr] = (uint32_t)  bm->stack[bm->stack_size - 1].as_u64;
	bm->stack_size -= 2;
	bm->ip += 1;
    }	break;
	

    case INST_WRITE64: {
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}

	const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
	if(addr >= BM_MEMORY_CAPACITY - 7) {
	    return ERR_ILLEGAL_MEMORY_ACCESS;
	}

	*(uint64_t*)&bm->memory[addr] = bm->stack[bm->stack_size - 1].as_u64;
	bm->stack_size -= 2;
	bm->ip += 1;
    }	break;

    case INST_I2F: {
	CAST_OP(bm, i64, f64, (double));
    } break;

    case INST_U2F: {
	CAST_OP(bm, u64, f64, (double));
    } break;    

    case INST_F2I: {
	CAST_OP(bm, f64, i64, (int64_t));
    } break;    

    case INST_F2U: {
	CAST_OP(bm, f64, u64, (uint64_t) (int64_t));
    } break;

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
	    fprintf(stream, "u64:  %"PRIu64", i64: %"PRIi64", f64: %lf, ptr: %p\n",
				bm->stack[i].as_u64,
				bm->stack[i].as_i64,
				bm->stack[i].as_f64,
				bm->stack[i].as_ptr);
	}
    } else {
	fprintf(stream, "  [empty]\n");
    }
    
}

// * Read from file -> bm->program buffer
void bm_load_program_from_file(Bm *bm, const char *file_path) {
    
    // * Open file in read binary mode
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    // * Read the meta information of file
    Bm_File_Meta meta = {0};
    size_t n = fread(&meta, sizeof(meta), 1, f);
    if (n < 1) {
        fprintf(stderr, "ERROR: could not read meta data from file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }
    // * Verify the magic number
    if(meta.magic != BM_FILE_MAGIC) {
        fprintf(stderr,
	"ERROR: %s does not appear to be a valid BM file. "
	"Unexpected magic %04X. Expected %04X.\n", file_path, meta.magic, BM_FILE_MAGIC);
        exit(1);
    }

    // * Verifiy the bm file version
    if(meta.version != BM_FILE_VERSION) {
        fprintf(stderr,
	"ERROR: %s: unsupported version of BM file %d."
	"Expected version %d.\n", file_path, meta.version, BM_FILE_VERSION);
        exit(1);
    }

    if(meta.program_size > BM_PROGRAM_CAPACITY) {
        fprintf(stderr,
	"ERROR: %s: program section is too big. The file contains %"PRIu64" program instruction. But the capacity is %d.\n", file_path, meta.program_size, BM_PROGRAM_CAPACITY);
        exit(1);	
    }

    // * Entry Point
    bm->ip = meta.entry;

    if(meta.memory_capacity > BM_MEMORY_CAPACITY) {
        fprintf(stderr,
	"ERROR: %s: memory section is too big. The file wants %"PRIu64" bytes. But the capacity is %d.\n", file_path, meta.memory_capacity, BM_MEMORY_CAPACITY);
        exit(1);	
    }


    if(meta.memory_size > meta.memory_capacity) {
        fprintf(stderr,
	"ERROR: %s: memory size %"PRIu64" is greater than declared memory capacity %"PRIu64".\n", file_path, meta.memory_size, meta.memory_capacity);
        exit(1);	
    }

    // * Read program instructions
    bm->program_size = fread(bm->program, sizeof(bm->program[0]), meta.program_size, f);
    // printf("bm->program_size: %lld\n", bm->program_size);
    if(bm->program_size != meta.program_size) {
        fprintf(stderr,
	"ERROR: %s: read %"PRIu64" program instructions, but expected %"PRIu64".\n", file_path, bm->program_size, meta.program_size);
        exit(1);	
    }

    // * Read static memory
    n = fread(bm->memory, sizeof(bm->memory[0]), meta.memory_size, f);
    // printf("meta->memory_size: %ld\n", n);
    if(n != meta.memory_size) {
        fprintf(stderr, 
        "ERROR: %s: read %zd bytes, but expected %"PRIu64".\n", file_path, n, meta.memory_size);
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
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }
    // printf("%ld - %ld\n", sv->count, i);

    String_View result = {
        .count = i,
        .data = sv->data,
    };

    if (i < sv->count) {
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
    for (size_t i = 0; (i < sv.count && isdigit(sv.data[i])); ++i) {
        result = result * 10 + sv.data[i] - '0';
    }
    return result;
}

int sv_eq(String_View a, String_View b) {
    if (a.count != b.count) {
        return 0;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

void print_names(const Basm *basm) {
    printf("-------- NAMES: -------\n");
    for(size_t i = 0; i < basm->bindings_size; ++i) {
	printf("%.*s ->  %"PRIu64"\n",
	(int) basm->bindings[i].name.count,
	basm->bindings[i].name.data,
	basm->bindings[i].value.as_u64);
    }
}
 
void print_unresolved_names(const Basm *basm) {
    printf("-------- UNRESOLVED_JMPS: -------\n");
    for(size_t i = 0; i < basm->defered_operands_size; ++i) {
	printf("%"PRIu64" -> %.*s\n",
	basm->defered_operands[i].addr,
	(int) basm->defered_operands[i].name.count,
	basm->defered_operands[i].name.data);
    }
}

void *basm_alloc(Basm *basm, size_t size) {
    assert(basm->arena_size + size <= BASM_ARENA_CAPACITY);
    void *result = basm->arena + basm->arena_size;
    basm->arena_size += size;
    return result;
}

// * Finds the address to which given name is bind to 
int basm_resolve_binding(Basm *basm, String_View name, Word *output) {
    for (size_t i = 0; i < basm->bindings_size; ++i) {
        if (sv_eq(basm->bindings[i].name, name)) {
 	    *output = basm->bindings[i].value;
	    return 1;
        }
    }
    return 0;
}

int basm_bind_value(Basm *basm, String_View name, Word value) {
    assert(basm->bindings_size < BASM_BINDINGS_CAPACITY);

    // * Check if name already bind
    Word ignore = {0};
    if(basm_resolve_binding(basm, name, &ignore)) {
	return 0;
    }

    // printf("Name: %s\n", name.data);
    // printf("%lf\n", word.as_f64);
    
    basm->bindings[basm->bindings_size++] = (Binding) {.name = name, .value = value};
    return 1;
}

void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View name) {
    assert(basm->defered_operands_size < DEFERED_OPERANDS_CAPACITY);
    basm->defered_operands[basm->defered_operands_size++] = (Defered_Operand) {
        .addr = addr,
        .name = name
    };
}

void basm_save_to_file(Basm *basm, const char *file_path) {
    FILE *f  = fopen(file_path, "wb");
    // printf("%ld", sizeof(program[1]));
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    Bm_File_Meta meta = {
        .magic = BM_FILE_MAGIC,
        .version = BM_FILE_VERSION,
	.entry = basm->entry,
        .program_size = basm->program_size,
        .memory_size = basm->memory_size,
        .memory_capacity = basm->memory_capacity,
    };

    // * size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
    fwrite(&meta, sizeof(meta), 1, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }
    
    fwrite(basm->program, sizeof(basm->program[0]), basm->program_size, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }
 
    fwrite(basm->memory, sizeof(basm->memory[0]), basm->memory_size, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}


// void *basm_alloc(Basm *basm, size_t size) {
//     assert(basm->arena_size + size <= BASM_ARENA_CAPACITY);

//     void *result = basm->arena + basm->arena_size;
//     basm->arena_size += size;
//     return result;
// }

void* arena_sv_to_cstr(Basm *basm, String_View sv) {
    assert(basm->arena_size + (sv.count + 1) <= BASM_ARENA_CAPACITY);

    // bring the pointer to the last address in arena
    void *result = basm->arena + basm->arena_size;
    basm->arena_size += (sv.count + 1);
    memcpy(result, sv.data, sv.count);
    // result[basm->arena_size] = '\0'; 
    return result;
}

Word basm_push_string_to_memory(Basm *basm, String_View sv) {
    assert(basm->memory_size + sv.count <= BM_MEMORY_CAPACITY);
    Word result = word_u64(basm->memory_size);

    // * copy sv.count amount of bytes to basm->memory buffer
    memcpy(basm->memory + basm->memory_size, sv.data, sv.count);

    basm->memory_size += sv.count;

    if(basm->memory_size > basm->memory_capacity) {
	basm->memory_capacity = basm->memory_size;
    }

    return result;
}

bool basm_translate_literal(Basm *basm, String_View sv, Word *output) {
    // * Encounter Character
    if(sv.count > 2 && *sv.data == '\'' && sv.data[sv.count - 1] == '\'') {
	if(sv.count - 2 != 1) {
	    return false;
	}
	*output = word_u64((uint64_t)sv.data[1]);
	return true;
    } else if(sv.count >= 2 && *sv.data == '"' && sv.data[sv.count - 1] == '"') {
	// * Encounter String
	sv.data += 1;
	sv.count -= 2;
	*output = basm_push_string_to_memory(basm, sv);
    }
    else {
    	// assert(sv.count < 1024);
	// char cstr[sv.count + 1];
	// char *endptr = 0;
	// memcpy(cstr, sv.data, sv.count);
	// cstr[sv.count] = '\0';

	// TODO Implement this function
	char *cstr = arena_sv_to_cstr(basm, sv);
	char *endptr = 0;
	
	Word result = {0};

	// * Try to parse it as uint64_t first
	result.as_u64 = strtoull(cstr, &endptr, 10);

	if((size_t)(endptr - cstr) != sv.count) {
	    // * Try to parse it as double
	    result.as_f64 = strtod(cstr, &endptr);
	    if((size_t)(endptr - cstr) != sv.count) {
		return false;
	    }
	}

	*output = result;
    }
    return true;
}

void basm_translate_source(Basm *basm, String_View input_file_path) {    
    String_View original_source = basm_slurp_file(basm, input_file_path);
    String_View source = original_source;
    // printf("Source: \n%s\n", source.data);

    int line_number = 0;
    
    while(source.count > 0) {
	String_View line = sv_trim(sv_chop_by_delim(&source, '\n'));
	line_number += 1;
	if(line.count > 0 && *line.data != BASM_COMMENT_SYMBOL) {
	  // printf("#%.*s#\n", (int) line.count, line.data);
	    
	   String_View token = sv_chop_by_delim(&line, ' ');
	   //  printf("#%.*s#\n", (int) token.count, token.data);

	   // * Pre-processor
	   if(token.count > 0 && *token.data == BASM_PP_SYMBOL) {
	       token.count -= 1;
	       token.data += 1;
	       
	       if(sv_eq(token, cstr_as_sv("bind"))) {
		   line = sv_trim(line);
		   // printf("Line: #%.*s#\n", (int) line.count, line.data);
		   
		   String_View name = sv_chop_by_delim(&line, ' ');
		   // printf("Name: #%.*s#\n", (int) name.count, name.data);
		   
		   if(name.count > 0) {
		       line = sv_trim(line);
		       String_View value = line;
		       // printf("Value: #%.*s#\n", (int) value.count, value.data);
		       
		       Word word = {0};
		       if(!basm_translate_literal(basm, value, &word)) {
			   fprintf(stderr, "%"SV_Fmt":%d: ERROR: `%"SV_Fmt"` is not a number\n",
			   SV_Arg(input_file_path), line_number, SV_Arg(value));
			   exit(1);
		       }
		       
		       // * Check if name already bind to some other instructions
		       if(!basm_bind_value(basm, name, word)) {
			   fprintf(stderr, "%"SV_Fmt":%d: ERROR: name `%"SV_Fmt"` is already bound\n",
			   SV_Arg(input_file_path), line_number, SV_Arg(name));
			   exit(1);
		       }
		   }
		   else {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: binding name is not provided\n",
		       SV_Arg(input_file_path), line_number);
		       exit(1);
		   }
	       } else if(sv_eq(token, cstr_as_sv("include"))) {
		   
		   line = sv_trim(line);
		   if(line.count > 0) {
		       if(*line.data == '"' && line.data[line.count - 1] == '"') {
			   line.data += 1;
			   line.count -= 2;    // * For start and end (")

			   // printf("File Name: #%.*s#\n", (int) line.count, line.data);

			   if(basm->include_level + 1 >= BASM_MAX_INCLUDE_LEVEL) {
			       fprintf(stderr,
			       "%"SV_Fmt":%d ERROR: exceeded maximum include level\n",
			       SV_Arg(input_file_path), line_number);
			   }
			   
			   // * Recursively Translate source file
			   basm->include_level += 1;
			   basm_translate_source(basm, line);
			   basm->include_level -= 1;
			   
		       }
		       else {
			   fprintf(stderr, "%"SV_Fmt":%d: ERROR: include file path has to be surrounded by quotation marks\n",
			   SV_Arg(input_file_path), line_number);
			   exit(1);
		       }
		       
		   } else {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: file path is not provided\n",
		       SV_Arg(input_file_path), line_number);
		       exit(1);
		   }
	       } else if(sv_eq(token, cstr_as_sv("entry"))) {
		   if(basm->has_entry) {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: entry point has been already set!\n", SV_Arg(input_file_path), line_number);
		   }
		   
		   line = sv_trim(line);
		   if(line.count == 0) {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: literal or binding name is expected.\n", SV_Arg(input_file_path), line_number);
		       exit(1);
		   }
		   
		   Word entry = {0};
		   if(!basm_translate_literal(basm, line, &entry)) {
		       basm->deferred_entry_binding_name = line;
		   } else {
		       basm->entry = entry.as_u64;
		   }
		   basm->has_entry = true;
	       }
	       else {
		   fprintf(stderr, "%.*s:%d: ERROR: unknown pre-processor directive `%.*s`\n",
		   SV_Arg(input_file_path), line_number, SV_Arg(token));
		   exit(1);		   
	       }
	       
	   } else {
	       // * Label Bindings
	       if(token.count > 0 && token.data[token.count - 1] == ':') {
		   String_View name = {
		       .count = token.count - 1,		   
		       .data = token.data
		   };
		   
		   // fprintf(stdout, "Name: %"SV_Fmt", Address: %"PRIu64"\n", SV_Arg(name), basm->program_size);

		   if(!basm_bind_value(basm, name, word_u64(basm->program_size))) {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: binding `%"SV_Fmt"` is already defined\n",
		       SV_Arg(input_file_path), line_number, SV_Arg(name));
		       exit(1);		       
		   }
		   
		   // * Check any inst after ':'
		   token = sv_trim(sv_chop_by_delim(&line, ' '));
	       }

	       // * Instruction
	       if(token.count > 0) {
		   String_View operand = sv_trim(sv_chop_by_delim(&line, BASM_COMMENT_SYMBOL));
		   // fprintf(stdout, "Token = %"SV_Fmt" at %"PRIu64"\n", SV_Arg(token), basm->program_size);
		   Inst_Type inst_type = INST_NOP;
		   if(inst_by_name(token, &inst_type)) {
		       assert(basm->program_size < BM_PROGRAM_CAPACITY);
		       basm->program[basm->program_size].type = inst_type;
		       
		       if(inst_has_operand(inst_type)) {
			   if(operand.count == 0) {
			       fprintf(stderr, "%.*s:%d: ERROR: instruction `%"SV_Fmt"` requires an operand\n",
			       SV_Arg(input_file_path), line_number,
			       (int) token.count, token.data);
			       exit(1);
			   }
			   // * parse operand as word  
			   if(!basm_translate_literal(
			       basm,
			       operand,
			       &basm->program[basm->program_size].operand)) {
				   // fprintf(stdout, "Operand = %"SV_Fmt" at %"PRIu64"\n", SV_Arg(operand), basm->program_size);
				   // * or parse operand as name
				   basm_push_defered_operand(basm, basm->program_size, operand);
			   }
		       }
		       // Increase the program_size
		       basm->program_size += 1;
		   } else {
		       fprintf(stderr, "%"SV_Fmt":%d: ERROR: unknown instruction `%"SV_Fmt"`\n",
		       SV_Arg(input_file_path), line_number, SV_Arg(token));
		       exit(1);
		   }
	       }
	   }
      }
  }

  // * Dereferencing the jmp names to address
  for(size_t i = 0; i < basm->defered_operands_size; ++i) {
      String_View binding = basm->defered_operands[i].name;   
      if(!basm_resolve_binding(
	  basm,
	  binding,
	  &basm->program[basm->defered_operands[i].addr].operand
      )) {
	  fprintf(stderr, "%"SV_Fmt" ERROR: unknown binding `%"SV_Fmt"`\n",
	  SV_Arg(input_file_path), SV_Arg(binding));
	  exit(1);	  
      }
  }

  if(basm->has_entry && basm->deferred_entry_binding_name.count > 0) {
      Word output = {0};
      if(!basm_resolve_binding(
          basm,
	  basm->deferred_entry_binding_name,
	  &output
      )) {
	  fprintf(stderr, "%"SV_Fmt" ERROR: unknown binding `%"SV_Fmt"`\n",
	  SV_Arg(input_file_path), SV_Arg(basm->deferred_entry_binding_name));
	  exit(1);	  
      }
      basm->entry = output.as_u64;
  }
  // print_names(basm);
  // print_unresolved_names(basm);
}


String_View basm_slurp_file(Basm *basm, String_View file_path)
{
    char *file_path_cstr = basm_alloc(basm, file_path.count + 1);
    if(file_path_cstr == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for the file `%.*s`: %s\n",
                SV_Arg(file_path),
                strerror(errno));
        exit(1);
    }

    memcpy(file_path_cstr, file_path.data, file_path.count);
    file_path_cstr[file_path.count] = '\0';
    
    FILE *f = fopen(file_path_cstr, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n", file_path_cstr, strerror(errno));
        exit(1);
    }

    // fseek(FILE *stream, long offset, int whence);
    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path_cstr, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if(m < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path_cstr, strerror(errno));
        exit(1);
    }

    char *buffer = basm_alloc(basm, (size_t)m);
    if(buffer == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for file: %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path_cstr, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, (size_t)m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: could not read file `%s`: %s\n", file_path_cstr, strerror(errno));
        exit(1);
    }

    fclose(f);
    
    return  (String_View) { .count = n, .data = buffer };
}

#endif // BM_IMPLEMENTATION
