#define BM_IMPLEMENTATION
#include "./bm.h"

#include <stdarg.h>

static void panic(const char *fmt, ...) {
    fprintf(stderr, "ERROR: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void panic_error(const char *fmt, ...) {
    fprintf(stderr, "ERROR: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, " :%s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

// * This will contain the output of your program
static Arena actual_arena = {0};

static String_View sv_from_arena(Arena *arena) {
    return (String_View) {
	.count = arena->size,
	.data = arena->buffer,
    };
}

static Err bmr_write(Bm *bm) {
    if(bm->stack_size < 2) {
	return ERR_STACK_UNDERFLOW;
    }
    Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
    uint64_t count = bm->stack[bm->stack_size - 1].as_u64;
    // printf("%"PRIi64"\n", count);
    if(addr >= BM_MEMORY_CAPACITY) {
	return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= BM_MEMORY_CAPACITY) {
	return ERR_ILLEGAL_MEMORY_ACCESS;	
    }

    // fwrite(&bm->memory[addr], sizeof(bm->memory[0]), count, stdout);
    char *chunk = arena_alloc(&actual_arena, count);
    memcpy(chunk, &bm->memory[addr], count);
    
    bm->stack_size -= 2;

    return ERR_OK;
}

static void usage(FILE *stream) {
    fprintf(stream, "Usage: ./bmr -p <usage.bm> [-ao <actual-output.txt>] [-eo <expected-output.txt]\n");    
}

static char *shift(int *argc, char ***argv) {
    assert(*argc > 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

static const char *parse_cstr_value(const char *flag, int *argc, char ***argv) {
    if(*argc == 0) {
	panic("no value provided for flag `%s`", flag);
    }
    return shift(argc, argv);
}

static void compare_outputs(String_View expected, String_View actual) {
    for(size_t line_number = 0; expected.count > 0 && actual.count > 0; line_number++) {
	String_View expected_line = sv_chop_by_delim(&expected, '\n');
	String_View actual_line = sv_chop_by_delim(&actual, '\n');

	if(!sv_eq(expected_line, actual_line)) {
	    fprintf(stderr, "Expected output differs from the actual one.\n");
	    fprintf(stderr, "    Expected line %zu: `"SV_Fmt"`\n", line_number, SV_Arg(expected_line));
	    fprintf(stderr, "    Actual line   %zu: `"SV_Fmt"`\n", line_number, SV_Arg(actual_line));
	    exit(1);
	}
    }

    if(expected.count > 0) {
	panic("Expected output is bigger");
    }

    if(actual.count > 0) {
	panic("Actual output is bigger");
    }
}

int main(int argc, char *argv[]) {
    
    shift(&argc, &argv);        // Skip the program name

    const char *program_file_path = NULL;
    const char *actual_output_file_path = NULL;
    const char *expected_output_file_path = NULL;
    while(argc > 0) {
	const char *flag = shift(&argc, &argv);
	if(strcmp(flag, "-p") == 0) {
	    program_file_path = parse_cstr_value(flag, &argc, &argv);
	}
	else if(strcmp(flag, "-ao") == 0) {
	    actual_output_file_path = parse_cstr_value(flag, &argc, &argv);
	}
	else if(strcmp(flag, "-eo") == 0) {
	    expected_output_file_path = parse_cstr_value(flag, &argc, &argv);
	}
	else {
	    panic("unknown flag `%s`", flag);
	}
    }

    if(!program_file_path) {
	usage(stderr);
	panic("program file path is not provided.");
    }

    // * Check no file provided at all
    if(actual_output_file_path == NULL && expected_output_file_path == NULL) {
	usage(stderr);
	panic("at least -ao pr -eo is expected");
    }
    
    // * NOTE: The structure might be quite big due to it's arena. Better to allocate it in the static memory.
    static Bm bm = {0};

    bm_load_program_from_file(&bm, program_file_path);

    bm.natives_size = 8;
    bm.natives[7] = bmr_write;

    // * This will execute the program and will save it to `actual_arena` using `bmr_write`
    Err err = bm_execute_program(&bm, -1);
    if (err != ERR_OK) {
	panic(err_as_cstr(err));
    }

     // * Save the program o/p to expected_output_file_path
    if(actual_output_file_path) {
	FILE *output_file_fd = fopen(actual_output_file_path, "wb");
	
	if(!output_file_fd) {
	    panic_error("ERROR: could not save output to file `%s`", actual_output_file_path);
	}

	// * write to file
	fwrite(actual_arena.buffer, sizeof(actual_arena.buffer[0]), actual_arena.size, output_file_fd);
	if(ferror(output_file_fd)) {
	    panic_error("ERROR: could not save output to file `%s`", expected_output_file_path);
	}

	fclose(output_file_fd);
    }

    if(expected_output_file_path) {
	static Arena expected_arena = {0};
	String_View expected_output = arena_slurp_file(&expected_arena, cstr_as_sv(expected_output_file_path));
	String_View actual_output = sv_from_arena(&actual_arena);

	// printf("Expected o/p: %.*s\n", expected_output);
	// printf("Actual o/p: %.*s\n", actual_output);

	compare_outputs(expected_output, actual_output);
	printf("Expected Output\n");
    }

    return 0;
}
