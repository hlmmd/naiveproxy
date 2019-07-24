#include <stdio.h>

#include "nproxy.h"

int main()
{
    // start_tcp_nproxy(0, 0, 6000, 0, 0);
    start_nproxys();
    while (1)
        ;
    return 0;
}