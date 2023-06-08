#include "../include/fixedptc.h"
#include <stdio.h>
#include <math.h>

void fixedpt_DUT() {
    printf("*** demo:\n");
    fixedpt a = fixedpt_rconst(1.2);
    fixedpt b = fixedpt_fromint(10);
    int c = 0;
    if (b > fixedpt_rconst(7.9)) {
      c = fixedpt_toint(fixedpt_div(fixedpt_mul(a + FIXEDPT_ONE, b), fixedpt_rconst(2.3)));
    }
    printf("%d\n", c);
}

void float_REF() {
    printf("*** demo:\n");
    float a = 1.2;
    float b = 10;
    int c = 0;
    if (b > 7.9) {
      c = (a + 1) * b / 2.3;
    }
    printf("%d\n", c);
}

int main() {
    printf("--------- fixedpt_DUT ---------\n");
    fixedpt_DUT();
    printf("---------- float_REF ----------\n");
    float_REF();
}
