#define BM_IMPLEMENTATION
#include "./bm.h"

Basm basm = {0};

// * Convert BASM Assembly To BASM vm executable

int main2(void);
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


int main2(void) {
    String_View str = cstr_as_sv("  Swapnil       Adsul\n  ");
    str = sv_trim(str);
    String_View sv = sv_chop_by_delim(&str, ' ');
    printf("%.*s\n", (int) sv.count, sv.data);
    // int ans = sv_eq(str, cstr_as_sv("Swapnil Adsul\n"));
    // printf("%s\n", str.data);
    // printf("%s\n", sv_trim(str).data);    
    return 0;
}

int main(int argc, char **argv) {

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
    
    // * Translate the source in to bm virtural machine [Interpret the program]
    basm_translate_source(&basm, cstr_as_sv(input_file_path));

    // * Save the executable
    basm_save_to_file(&basm, output_file_path);

    printf("%zd Bytes of memory used\n ", basm.arena_size);
		         
    return 0;
}
