#define BM_IMPLEMENTATION
#include "./bm.h"

// * Convert BASM Assembly To BASM vm executable

int main(int argc, char **argv);

static char *shift(int *argc, char ***argv) {
    assert(*argc > 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

static void usage(FILE *stream, const char *program) {
    fprintf(stream, "Usage: %s <input.basm> <output.bm>\n", program);
}

int main(int argc, char **argv) {

    // int have_symbol_table = 0;
    const char *program = shift(&argc, &argv);
    if(argc == 0) {
	usage(stderr, program);
	fprintf(stderr, "ERROR: expected input\n");
	exit(1);
    }

    const char *input_file_path = shift(&argc, &argv);
    if(argc == 0) {
	usage(stderr, program);
	fprintf(stderr, "ERROR: expected output\n");
	exit(1);
    }

    const char *output_file_path = shift(&argc, &argv);
   

    // * Read the basm file
    // String_View source = sv_slurp_file(input_file_path);
    // printf("Source: \n%s\n", source.data);

    static Basm basm = {0};
    // * Translate the source in to bm virtural machine [Interpret the program]
    basm_translate_source(&basm, cstr_as_sv(input_file_path));

    if(!basm.has_entry) {
	fprintf(stderr, "%s: ERROR: entry point for a BM program is not provided. Use preprocessor directive %%entry to provide the entry point. Examples:\n", input_file_path);
	fprintf(stderr, " %%entry main\n");
	fprintf(stderr, " %%entry 10\n");
	exit(1);
    }
    
    // * Save the executable
    basm_save_to_file(&basm, output_file_path);

    // printf("%zd Bytes of memory used\n ", basm.arena.size);
		         
    return 0;
}
