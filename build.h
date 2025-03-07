#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_SIZE (sizeof(PATH_SEPARATOR) - 1)

#define FOREACH_VARGS(arg, args, body)			\
do {							\
    va_start(args, ignore);				\
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


const char *path_impl(int ignore, ...) {
    size_t length = 0;  // * For Memory Allocation
    size_t seps = 0;	// * For Path Separator
    va_list args;
    
    // * Calculate size which needs to be allocated to create a PATH
    FOREACH_VARGS(arg, args, {
	length += strlen(arg);
	seps += 1;
    });

    assert(length > 0);
    seps -= 1;
    
    // * Allocate memory for path string
    char *result = malloc(length + seps * PATH_SEPARATOR_SIZE + 1);
    
    length = 0;
    FOREACH_VARGS(arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;

	// * Do not add PATH_SEPARATOR after last arg
	if(seps > 0) {
	    memcpy(result + length, PATH_SEPARATOR, PATH_SEPARATOR_SIZE);	    
	    length += PATH_SEPARATOR_SIZE;
	    seps--;
	}

    });    

    result[length] = '\0';
    
    return result;
}

// Macro Variadic Arguments
#define PATH(...) path_impl(69, __VA_ARGS__, NULL)

void mkdirs_impl(int ignore, ...) {
    size_t length = 0;  // * For Memory Allocation
    size_t seps = 0;	// * For Path Separator
    va_list args;
    
    // * Calculate size which needs to be allocated to create a PATH
    FOREACH_VARGS(arg, args, {
	length += strlen(arg);
	seps += 1;
    });

    assert(length > 0);
    seps -= 1;

    // * Allocate memory for path string
    char *result = malloc(length + seps * PATH_SEPARATOR_SIZE + 1);    
    
    length = 0;
    FOREACH_VARGS(arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;

	// * Do not add PATH_SEPARATOR after last arg
	if(seps > 0) {
	    memcpy(result + length, PATH_SEPARATOR, PATH_SEPARATOR_SIZE);	    
	    length += PATH_SEPARATOR_SIZE;
	    seps -= 1;
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
		exit(EXIT_FAILURE);
	    }
	}

    });    
   
    
    assert(length > 0);    
}

#define MKDIRS(...) mkdirs_impl(69, __VA_ARGS__, NULL);

const char *concat_impl(int ignore, ...) {
    size_t length = 0;
    va_list args;

    // * Calculate size which needs to be allocated to create a PATH
    FOREACH_VARGS(arg, args, {
	// printf("Arg: %s\n", arg);
	length += strlen(arg);
    });
    printf("length: %d\n", length);
    char *result = malloc(length + 1);

    // * Concat the strings
    length = 0;
    FOREACH_VARGS(arg, args, {
	size_t n = strlen(arg);
	memcpy(result + length, arg, n);
	length += n;
    });

    result[length] = '\0';

    return result;
}

#define CONCAT(...) concat_impl(69, __VA_ARGS__, NULL)
