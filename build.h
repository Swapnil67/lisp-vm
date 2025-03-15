#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include "windows.h"
#else
#    include <unistd.h>
#    include <sys/stat.h>
#    include <sys/wait.h>
#    include <dirent.h>
#endif // _WIN32


#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif // _WIN32 

#define PATH_SEP_SIZE (sizeof(PATH_SEP) - 1)

#define FOREACH_VARGS(param, arg, args, body)		\
do {							\
    va_start(args, param);				\
    for(const char *arg = va_arg(args, const char *);	\
        arg != NULL;					\
	arg = va_arg(args, const char *))		\
    {							\
	body;						\
    }							\
    va_end(args);					\
} while(0)


#define FOREACH_ARRAY(type, item, items, body)	\
do {						\
    for(size_t i = 0;				\
        i < sizeof(items) / sizeof((items)[0]);	\
        ++i) {					\
        type item = items[i];			\
        body;					\   
    }						\
} while(0)


#define FOREACH_FILE_IN_DIRS(file, dirpath, body)	\
do {							\
    struct dirent *dp = NULL;				\
    DIR *dir = opendir(dirpath);			\
    while(dp = readdir(dir)) {				\
	const char *file = dp->d_name;			\
	body;						\
    }							\
} while(0)


const char *concat_sep_impl(const char *sep, ...) {
    const size_t sep_len = strlen(sep);
    size_t length = 0;		// * For Memory Allocation
    size_t seps_count = 0;	// * For Path Separator
    va_list args;
    
    // * Calculate size which needs to be allocated to create a PATH
    FOREACH_VARGS(sep, arg, args, {
	length += strlen(arg);
	seps_count += 1;
    });

    assert(length > 0);
    seps_count -= 1;
    
    // * Allocate memory for path string
    char *result = malloc(length + seps_count * sep_len + 1);
    
    length = 0;
    FOREACH_VARGS(sep, arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;

	// * Do not add PATH_SEPARATOR after last arg
	if(seps_count > 0) {
	    memcpy(result + length, sep, sep_len);	    
	    length += sep_len;
	    seps_count--;
	}

    });    

    result[length] = '\0';

    // printf("Path: %s\n", result);
    
    return result;
}

// Macro Variadic Arguments
#define CONCAT_SEP(sep, ...) concat_sep_impl(sep, __VA_ARGS__, NULL)

#define PATH(...) CONCAT_SEP(PATH_SEP, __VA_ARGS__)

void mkdirs_impl(int ignore, ...) {
    size_t length = 0;		// * For Memory Allocation
    size_t seps_count = 0;	// * For Path Separator
    va_list args;
    
    // * Calculate size which needs to be allocated to create a PATH
    // * Also count how may path separators we would need
    FOREACH_VARGS(ignore, arg, args, {
	length += strlen(arg);
	seps_count += 1;
    });

    assert(length > 0);
    seps_count -= 1;

    // * Allocate memory for path string
    char *result = malloc(length + seps_count * PATH_SEP_SIZE + 1);    
    
    length = 0;
    FOREACH_VARGS(ignore, arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;

	// * Do not add PATH_SEPARATOR after last arg
	if(seps_count > 0) {
	    memcpy(result + length, PATH_SEP, PATH_SEP_SIZE);	    
	    length += PATH_SEP_SIZE;
	    seps_count -= 1;
	}

	result[length] = '\0';

	printf("[INFO] mkdir %s\n", result);
	if(mkdir(result, 0755) < 0) {
	    if(errno == EEXIST) {
		fprintf(stderr, "[WARN] directory %s already exists\n", result);
	    }
	    else {	
		fprintf(stderr, "[ERROR] could not create directory %s: %s\n",
		        result, strerror(errno));	   
		exit(1);
	    }
	}

    });    
   
}

#define MKDIRS(...) mkdirs_impl(69, __VA_ARGS__, NULL)

const char *concat_impl(int ignore, ...) {
    size_t length = 0;
    va_list args;

    // * Calculate size which needs to be allocated to create a PATH
    FOREACH_VARGS(ignore, arg, args, {
	// printf("Arg: %s\n", arg);
	length += strlen(arg);
    });
    // printf("length: %d\n", length);
    char *result = malloc(length + 1);

    // * Concat the strings
    length = 0;
    FOREACH_VARGS(ignore, arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;
    });

    result[length] = '\0';

    return result;
}

#define CONCAT(...) concat_impl(69, __VA_ARGS__, NULL)

void cmd_impl(int ignore, ...) {

    size_t argc = 0;
    
    va_list args;
    FOREACH_VARGS(ignore, arg, args, {
	argc += 1;
    });

    // sizeof(const char *) = 8
    const char **argv = malloc(sizeof(const char *) * (argc + 1));

    argc = 0;    
    FOREACH_VARGS(ignore, arg, args, {
	argv[argc++] = arg;
    });

    assert(argc >= 1);

    pid_t pid = fork();
    if(pid == -1) {
	fprintf(stderr, "[ERROR] could not fork a child process: %s\n", strerror(errno));
	exit(1);
    }
    
    if(pid == 0) {
	if(execvp(argv[0], (char * const*)argv) < 0) {
	    fprintf(stderr, "[ERROR] could not execute child process: %s\n", strerror(errno));
	    exit(1);
	}
    }
    else {
	wait(NULL);
    }

}

#define CMD(...)					\
do {							\
printf("[INFO] %s\n", CONCAT_SEP(" ", __VA_ARGS__));	\
cmd_impl(69, __VA_ARGS__, NULL);			\
} while(0)    
