#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *endptr = 0;
    const char *cstr = "-69.0";

    unsigned long long int x = strtoull(cstr, &endptr, 10);

    printf("Parsed Value: %lld\n", x);
    printf("End pointer: %ld\n", endptr);
    printf("endptr - cstr: %ld\n", endptr - cstr);
    printf("cstr: %ld\n", strlen(cstr));
    
    
    return 0;
}
