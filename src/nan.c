#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

// * Any double whose exponent bits are all set is a NaN, regardless of the mantissa bits.
// * Signalling NaN
// * They are intended to be the result of erroneous computations, like division by zero
// * Quiet NaN
// * Every double with all of its exponent bits set and its highest mantissa bit set is a quiet NaN.

// static void print_bits_simple(int n) {
//     printf("%d = ", n);
//     for(int i = 7; i >= 0; --i) {
// 	printf("%d", !!(n & (1 << i)));
//     }
//     printf("\n");
// // }

// static void print_bytes(uint8_t *bytes, size_t byte_size) {
//     printf("byte_size: %zu\n", byte_size);
//     for(size_t i = 0; i < byte_size; ++i) {
// 	printf("0x%X ", bytes[i]);
//     }    
// }

// * 32 bit SP = s 1111111 1000 0000 0000 0000 0000 0000
// * 64 bit DP = s 11111111111 1000 00000000 00000000 00000000 00000000 00000000 00000000

// * Exponent [52 - 62 bits] => 22 bits
#define EXP_MASK (((1LL << 11LL) - 1LL) << 52LL)
// Mantissa [0 - 51 bits] => 52 bits
#define FRACTION_MASK ((1LL << 52LL) - 1LL)
// 63rd bit
#define SIGN_MASK (1LL - 63LL)
// First four bits of Mantissa
#define TYPE_MASK (((1LL << 4LL) - 1LL) << 48LL)
// 1st 48 bits of mantissa where value is stored
#define VALUE_MASK ((1LL << 48LL) - 1LL)

#define TYPE(n) ((1LL << 3LL) + n)
#define DOUBLE_TYPE 0
#define INTEGER_TYPE 1
#define POINTER_TYPE 2


#define INSPECT_VALUE(type, value, label)                        \
{                                                                \
       type name = (value);                                      \
       printf("%s = \n    ", label);                             \
       print_bits((uint8_t*) &name, sizeof(name));               \
       printf("    is_nan = %d\n", is_nan(name));                \
       printf("    isnan = %d\n", isnan(name));                  \
       printf("    is_inf = %d\n", is_inf(name));                \
       printf("    isinf = %d\n", isinf(name));                  \
}                                                                \
   

static void print_bits(uint8_t *bytes, size_t byte_size) {
    for(int i = (int) byte_size - 1; i >= 0; --i) {
	uint8_t byte = bytes[i];
	// printf("0x%X ", byte);
	for(int j = 7; j >= 0; --j) {
	    printf("%d", !!(byte & (1 << j)));
	}
	printf(" ");
    }
    printf("\n");
}  

static int is_nan(double x) {
    uint64_t y = (*(uint64_t*) &x);
    // * since for NaN FRACTION_MASK is always non-zero because 1st bit of mantissa is set
    // * is always set to 1 => 1000 00000000 00000000 00000000 00000000 00000000 00000000
    return ((y & EXP_MASK) == EXP_MASK) && ((y & FRACTION_MASK) != 0);
}

static int is_inf(double x) {
    uint64_t y = (*(uint64_t*)&x);
    // * Here the FRACTION_MASK is always zero
    // * 01111111 11110000 00000000 00000000 00000000 00000000 00000000 00000000 
    return ((y & EXP_MASK) == EXP_MASK) && ((y & FRACTION_MASK) == 0);
}

static uint64_t get_type(double x) {
    uint64_t y = (*(uint64_t*) &x);
    return (y & TYPE_MASK) >> 48LL;
}

static uint64_t get_value(double x) {
    uint64_t y = *(uint64_t*) &x;
    return (y & VALUE_MASK);    
}

static double set_type(double x, uint64_t type) {
    uint64_t y = *(uint64_t *)&x;
    y = (y & (~TYPE_MASK)) | (((TYPE_MASK >> 48LL) & type) << 48LL);
    return *(double *)&y;
}

static double set_value(double x, uint64_t value) {
    uint64_t y = *(uint64_t*) &x;
    y = (y & (~VALUE_MASK)) | (value & VALUE_MASK);
    return *(double*)&y; 
}

static double mk_inf(void) {
    uint64_t y = EXP_MASK;
    return *(double *) &y;
}

static int is_double(double x) {
    return !isnan(x);
}

static int is_integer(double x) {
    return isnan(x) && get_type(x) == TYPE(INTEGER_TYPE);
}

static int is_pointer(double x) {
    return isnan(x) && get_type(x) == TYPE(POINTER_TYPE);
}

static double as_double(double x) {
    //    return *(double*) get_value(x);
    return x;
}

static uint64_t as_integer(double x) {
    return  get_value(x);
}

static void *as_pointer(double x) {
    return (void*) get_value(x);
}

double box_double(double x) {
    return x;
   //  return set_value(set_type(mk_inf(), TYPE(DOUBLE_TYPE)), x);
}

double box_integer(uint64_t x) {
    return set_value(set_type(mk_inf(), TYPE(INTEGER_TYPE)), x);
}

double box_pointer(void* x) {
    return set_value(set_type(mk_inf(), TYPE(POINTER_TYPE)), (uint64_t)  x);
}


#define VALUES_CAPACITY 256
double values[VALUES_CAPACITY];
size_t values_size = 0;
    
int main(void) {

    const double pi = 3.14159265359;
    INSPECT_VALUE(double, pi, "3.14159265359");
    INSPECT_VALUE(double, mk_inf(), "inf");
    // INSPECT_VALUE(double, box_double(pi), "box_double(pi)");
    // INSPECT_VALUE(double, box_integer(10), "box_integer(10)");
    // INSPECT_VALUE(double, box_pointer(&pi), "box_pointer(&pi)");
    
    assert(pi	== as_double(box_double(pi)));
    assert(10	== as_integer(box_integer(10)));
    assert(&pi	== as_pointer(box_pointer(&pi)));
    
    printf("OK\n");
    
    return 0;
}


// int main(void) {
//     printf("Pointer Conversions\n");
//     double a = 10.10;
//     printf("Long Double: %f\n", a);
//     printf("Address of a:  %p\n",(void*) &a);
//     uint64_t* b = (uint64_t*)&a;
//     printf("Long Int: %llu\n", *b);
    
//     uint8_t *bytes = (uint8_t*)&a;
//     print_bits(bytes, sizeof(a));
//     // printf("first byte: %d\n", bytes[0]);
//     // printf("first byte: %x\n", bytes[0]);

// }

