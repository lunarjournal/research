// Author: Dylan Muller
// Student Number: MLLDYL002

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void number_rsa_pkcs(unsigned char *str,
                     int size,
                     unsigned char *modulus,
                     unsigned char *output,
                     int rsa_bits);

int number_rng_set_seed(int seed);
