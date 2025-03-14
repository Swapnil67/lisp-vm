#include "./build.h"


#ifdef _WIN32
#define CFLAGS "/std:c11", "/O2", "/FC", "/W4", "/WX", "/wd4996", "/wd4200", "/wd5105", "/nologo"
#else
#define CFLAGS "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes" ,"-Wconversion", "-Wno-missing-braces",  "-fno-strict-aliasing", "-ggdb", "-std=c11", "-pedantic"
#endif // _WIN32

const char *toolchain[] = {
    "basm", "bme", "bmr", "debasm", "basm2amd64"
};  


#ifdef _WIN32
void build_c_file(const char *input_file, const char *output_file) {
    CMD("cl.exe", CFLAGS, "-o", output_file, input_file);
}
#else
void build_c_file(const char *input_file, const char *output_file)
{
    CMD("cc", CFLAGS, input_file);
}    
#endif // _WIN32


void build_toolchain() {
    MKDIRS("build", "bin");
    
    FOREACH_ARRAY(const char *, tool, toolchain, {
	build_c_file(PATH("src", CONCAT(tool, ".c")), PATH("build", "bin", tool));
    });

}

void build_examples() {
    MKDIRS("build", "examples");

    FOREACH_FILE_IN_DIRS(example, "examples", {
        size_t n = strlen(example);
	if(*example != '.') {
	    assert(n >= 4);	 
	    // * Compare only basm files
	    if(strcmp(example + n - 4, "basm") == 0) {
		CMD(PATH("build", "bin", "basm"),
		    PATH("examples", example),
		    PATH("build", CONCAT(example, ".bm"))); 
	    }
	}
    });
        
}

int main() {
    build_toolchain();
    build_examples();
    return 0;
}


// * gcc -o build.exe build.c 
