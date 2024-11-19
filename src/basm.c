#define BM_IMPLEMENTATION
#include "./bm.h"

Bm bm = {0};

int main(int argc, char **argv) {
    // bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program));

    if(argc < 3) {
	fprintf(stderr, "Usage: ./basm <input.basm> <output.bm>\n");
	fprintf(stderr, "ERROR: expected input and output\n");
	exit(1);
    }

    const char *input_file_path = argv[1];
    const char *output_file_path = argv[2];

    // * Read the basm file
    String_View source = sv_slurp_file(input_file_path);

    // * Interpret the program
    bm.program_size = bm_translate_source(
        source,
        bm.program,
        BM_PROGRAM_CAPACITY);

    // * Save the executable
    bm_save_program_to_file(&bm, output_file_path);
		         
    return 0;
}
