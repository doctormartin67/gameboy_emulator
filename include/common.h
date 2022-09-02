#ifndef COMMON_H
#define COMMON_H

#define BIT(a, n) (((a) & (1 << (n))) ? 1 : 0)
#define SET_BIT(a, n, on) a = (on ? ((a) | (1 << (n))) : ((a) & ~(1 << (n))))
#define FLAG_Z_BIT 7
#define FLAG_N_BIT 6
#define FLAG_H_BIT 5
#define FLAG_C_BIT 4
#define LO_SHIFT(a) (uint16_t)a >> 8
#define HI_SHIFT(a) (uint16_t)a << 8

#endif
