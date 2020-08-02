#include "config.h"

void VADD_Ref(Data_t const a[], Data_t const b[], Data_t c[])
{
    for (int t = 0; t < T; ++t)
    {
        c[t] = a[t] + b[t];
    }
}