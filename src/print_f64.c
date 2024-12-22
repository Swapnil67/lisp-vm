
#include <stdio.h>
 #include <stdlib.h>
 #include <inttypes.h>

 double floor(double x) {
     if(x >= 0.0) {
	 return (double) (int64_t) x;
     } else {
	 return (double) ((int64_t) x + 1);
     }
 }

 double frac(double x) {
     return x - floor(x);
 }

 void print_f64_frac(double f, int n) {
     // M = b^(-n)/2
     double M = 1.0;
     for(int i = 0; i < n; ++i) {
	 M *= 0.1;
     }
     M *= 0.5;
     
     // * Base
     double B = 10.0;
     double R = f;
     double U = 0.0;

     printf(".");

     do {
	 U = floor(R * B);
	 R = frac(R * B);
	 M *= B;
	 if(R < M) break;
	 if(R > 1 - M) break;
	 printf("%d", (int)U);
     } while(1);

     if(R > 0.5) {
	 U += 1;
     }
     printf("%d \n", (int)U);
 }

 void print_i64(int64_t n) {
     printf("%"PRIi64, n);
 }

 void print_f64(double f, int precision) {
     print_i64((int64_t) f);
     print_f64_frac(f - floor(f), precision);
 }
 
 int main(void) {
     double pi = 3.14151617;
     print_f64(pi, 10);
     return 0;
 }
