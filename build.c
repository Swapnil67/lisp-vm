#include "./build.h"

#define CFLAGS "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes" ,"-Wconversion" "-fno-strict-aliasing", "-std=c11", "-pedantic"


const char *toolchain[] = {
    "basm", "bme", "bmr", "debasm", "basm2amd64"
};  

int main() {
    MKDIRS("build", "bin");
    
    FOREACH_ARRAY(const char *, tool, toolchain, {
	printf("Building %s...\n", CONCAT(tool, ".c"));
	// CMD(CFLAGS, "-o",
	//     PATH("build", "bin", tool)
	//     PATH("src", CONCAT(tool, ".c")));
	});

#if 0

    MKDIRS("build", "examples");	
    FOREACH_FILE_IN_DIRS(example, "examples", {
        size_t n = strlen(example);
	assert(n >= 4);
	if(strcmp(example + n - 4, "basm") == 0) {
	    cmd(PATH("build", "bin", "basm")
	        "-g",
		example,
		PATH("build", CONCAT(example, ".bm"))); 
	});	
    }
    
	    
#endif
    
    return 0;
}


// * gcc -o build.exe build.c 
