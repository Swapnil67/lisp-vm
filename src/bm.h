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

typedef int64_t Word;

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_PLUS,
    INST_MINUS,
    INST_MUL,
    INST_DIV,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_HALT,
    INST_PRINT_DEBUG,
} Inst_Type;

const char *inst_type_as_cstr(Inst_Type type);

// Instruction Structure
typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

// Machine Structure
typedef struct {
    Word stack[BM_STACK_CAPACITY];
    Word stack_size;
    
    Inst program[BM_PROGRAM_CAPACITY];
    Word program_size;
    Word ip;
    
    int halt;
} Bm;

#define MAKE_INST_PUSH(value)	{ .type = INST_PUSH, .operand = value }
#define MAKE_INST_DUP(addr)	{ .type = INST_DUP, .operand = addr }
#define MAKE_INST_MUL()		{ .type = INST_MUL }
#define MAKE_INST_DIV()		{ .type = INST_DIV }
#define MAKE_INST_PLUS		{ .type = INST_PLUS }
#define MAKE_INST_MINUS()	{ .type = INST_MINUS }
#define MAKE_INST_JMP(addr)	{ .type = INST_JMP, .operand = addr }
#define MAKE_INST_HALT		{ .type = INST_HALT }

Err bm_execute_inst(Bm *bm);
Err bm_execute_program(Bm *bm, int limit);
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

Inst bm_translate_line(String_View line);
size_t bm_translate_source(String_View source, Inst *program, size_t program_capacity);

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
    case INST_PLUS:		return "INST_PLUS";
    case INST_MINUS:		return "INST_MINUS";
    case INST_DIV:		return "INST_DIV";
    case INST_MUL:		return "INST_MUL";
    case INST_JMP:		return "INST_JMP";
    case INST_JMP_IF:		return "INST_JMP_IF";
    case INST_EQ:		return "INST_EQ";
    case INST_HALT:		return "INST_HALT";
    case INST_PRINT_DEBUG:	return "INST_PRINT_DEBUT";
    default: assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

// * Execute basm program
Err bm_execute_program(Bm *bm, int limit) {
    while(limit != 0 && !bm->halt) {
	Err err = bm_execute_inst(bm);
	if(err != ERR_OK) {
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
    if(bm->ip < 0 || bm->ip >= bm->program_size) {
	return ERR_ILLEGAL_INST_ACCESS;
    }
    
    Inst inst = bm->program[bm->ip];
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
    case INST_DUP:
	if(bm->stack_size >= BM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}
	
	if(bm->stack_size - inst.operand <= 0) {
	    return ERR_STACK_UNDERFLOW;
	}

	if(inst.operand < 0) {
	    return ERR_ILLEGAL_OPERAND;
	}

	// * Push the operand to the top of stack  [Operand relative to current stack position]
	bm->stack[bm->stack_size] = bm->stack[bm->stack_size - 1 - inst.operand];
	bm->stack_size += 1;
	bm->ip += 1;
	break;
    case INST_PLUS:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] += bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_MINUS:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] -= bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_MUL:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] *= bm->stack[bm->stack_size - 1];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_DIV:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
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
    case INST_JMP_IF:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	// * If top of the stack it True
	if(bm->stack[bm->stack_size - 1]) {
	    bm->stack_size -= 1;
	    bm->ip = inst.operand;
	}
	else {
	    bm->ip += 1;
	}
	break;
    case INST_EQ:
	if(bm->stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	bm->stack[bm->stack_size - 2] = bm->stack[bm->stack_size - 1] == bm->stack[bm->stack_size-2];
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    case INST_HALT:
	bm->halt = 1;
	break;

    case INST_PRINT_DEBUG:
	if(bm->stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	printf("%lld", bm->stack[bm->stack_size - 1]);
	bm->stack_size -= 1;
	bm->ip += 1;
	break;
    default:
	return TRAP_ILLEGAL_INST;
    }

    return ERR_OK;
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
    size_t i = sv.count - 1;
    while(i >= 0 && isspace(sv.data[i])) {
	i -= 1;
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


Inst bm_translate_line(String_View line) {
    // printf("Instruction Length: %ld\n", line.count);
    
    line = sv_trim_left(line);
    String_View inst_name = sv_chop_by_delim(&line, ' ');

    if(sv_eq(inst_name, cstr_as_sv("push"))) {
	line = sv_trim_left(line);
	int operand = sv_to_int(sv_trim_right(line));
	return (Inst) { .type = INST_PUSH, .operand = operand };
    }
    else if(sv_eq(inst_name, cstr_as_sv("dup"))) {
	line = sv_trim_left(line);
	int operand = sv_to_int(sv_trim_right(line));
	return (Inst) { .type = INST_DUP, .operand = operand };
    }
    else if(sv_eq(inst_name, cstr_as_sv("plus"))) {
	line = sv_trim_left(line);
	return (Inst) { .type = INST_PLUS }; 
    }
    else if(sv_eq(inst_name, cstr_as_sv("jmp"))) {
	line = sv_trim_left(line);
	int operand = sv_to_int(sv_trim_right(line));
	return (Inst) { .type = INST_JMP, .operand = operand }; 
    }
    else {
	fprintf(stderr, "ERROR: unknown instruction `%.*s`", (int) inst_name.count, inst_name.data);
	exit(1);
    }
 }

 
size_t bm_translate_source(String_View source, Inst *program, size_t program_capacity) {
    size_t program_size = 0;
    while(source.count > 0) {
	assert(program_size < program_capacity);
	String_View line = sv_chop_by_delim(&source, '\n');
	// printf("#%.*s#\n", (int) line.count, line.data);
	if(line.count > 0) {
	    program[program_size++] = bm_translate_line(line);
	}
    }
    return program_size;;
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
