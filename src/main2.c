#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
    

int main(void) {
    char *endptr = 0;
    const char *cstr = "-69.0";

    unsigned long long int x = strtoull(cstr, &endptr, 10);

    printf("Parsed Value: %lld\n", x);
    printf("End pointer: %ld\n", endptr);
    printf("endptr - cstr: %ld\n", endptr - cstr);
    printf("cstr: %ld\n", strlen(cstr));


    double a = 1.0;
    printf("double: %f\n", a);
    
    uint64_t b = (uint64_t)a;
    printf("uint64_t: %"PRIu64"\n", b);
    
    
    return 0;
}
