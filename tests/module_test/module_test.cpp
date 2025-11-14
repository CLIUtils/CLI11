#include <cassert>
#include <cstdio>
#include <cstdlib>

import math;

int main() {
    int a = 1;
    int b = 2;

    int absum = add(a, b);
    int abdif = subtract(a, b);

    assert(a + b == absum);
    assert(a - b == abdif);

    // used this instead of <iostream> to work with older compilers that may choke on <iostream> implicit includes
    printf("OK: export module\n");

    return EXIT_SUCCESS;
}
