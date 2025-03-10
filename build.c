#include "./build.h"

#define CFLAGS "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes" ,"-Wconversion", "-fno-strict-aliasing", "-ggdb", "-std=c11", "-pedantic"


const char *toolchain[] = {
    "basm", "bme", "bmr", "debasm", "basm2amd64"
};  

int main() {
    // CMD("cd ", PATH("build", "bin"));
    MKDIRS("build", "bin");
    
    FOREACH_ARRAY(const char *, tool, toolchain, {
	CMD("cc", CFLAGS, "-o",
	    PATH("build", "bin", tool),
	    PATH("src", CONCAT(tool, ".c")));
	});

	MKDIRS("build", "examples");
	
#if 0


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
