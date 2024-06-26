#ifndef _UTIL_H_
#define _UTIL_H_
static inline int randrange (int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}
#endif


