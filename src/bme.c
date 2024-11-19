#define BM_IMPLEMENTATION
#include "./bm.h"

Bm bm = {0};

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: ./bme <input.bm>\n");
		fprintf(stderr, "ERROR: expected input\n");
		exit(1);
	}

	bm_load_program_from_file(&bm, argv[1]);
	bm_dump_stack(stdout, &bm);
	for (int i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; ++i) {
		// printf("%s\n", inst_type_as_cstr(bm.program[bm.ip].type));
		Trap trap = bm_execute_inst(&bm);
		if (trap != TRAP_OK)
		{
	    fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
	    exit(1);
	}
	// bm_dump_stack(stdout, &bm);
	}
		bm_dump_stack(stdout, &bm);
    return 0;
}
