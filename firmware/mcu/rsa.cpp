// Author: Dylan Muller
// Student Number: MLLDYL002

#include "rsa.h"

int g_id = 0;
typedef struct _number
{
  int id;
  int is_negative;
  int size_max;
  int size;
  unsigned char *bytes;
} number;

// Seed RNG
int number_rng_set_seed(int seed)
{
  srand(seed);
}

// Generate random number
// Be sure to check than return
// value is not 0 (for pkcs padding)
int number_rng_gen()
{
  int num = (char)(rand() % 255);
  if(num == 0)
  {
    num = 1;
  }
  return num;
}
int number_inc_gid()
{
  return g_id++;
}

int number_get_id(number *num)
{
  return num->id;
}
// Get number of bits representing number
int number_count_bits(number *num)
{
  int i = 0, j = 0;
  int bits = 0;
  unsigned char byte = 0;
  int last = 0;
  int displacement = 0;
  int last_zero = 0;
  int pad_zero = 0;

  for (i = 0; i < num->size; i++)
  {
    if (num->bytes[i] != 0)
    {
      last_zero = i + 1;
    }
  }

  for (i = 0; i < num->size; i++)
  {

    // If next byte exists add 8 bits
    if (i + 1 < last_zero)
    {
      if (num->bytes[i + 1] >= 0)
      {
        bits += 8;

        last = 0;
        continue;
      }
    }

    byte = num->bytes[i];

    // Compute bits on remaining bytes
    for (j = 0; j <= 7; j++)
    {

      if (byte & (1 << j))
      {
        // Set displacement marker
        displacement = 1 + j - last;

        bits += displacement;
        last = j + 1;
      }
    }
  }

  return bits;
}

// Calculate difference in bit length between numbers
int calc_bit_diff(number *y, number *x)
{

  // if id's match return 0
  if (y->id == x->id)
  {
    return 0;
  }

  // Calculate bits lengths of x and y
  int x_len = number_count_bits(x);
  int y_len = number_count_bits(y);
  int result = y_len - x_len;
  if (result < 0)
  {
    return -1;
  }
  return result;
}

number *number_alloc(int size_max)
{
  // Allocate initial memory
  number *num = malloc(sizeof(number));
  num->bytes = malloc(size_max);

  // Zero memory
  for (int i = 0; i < size_max; i++)
  {
    num->bytes[i] = 0;
  }

  num->is_negative = 0;
  num->size_max = size_max;
  num->size = 0;
  num->id = number_inc_gid();

  return num;
}

void number_size_xy(number *x, number *y)
{

  x->size = (number_count_bits(x) / 0x8) + 1;
  if (x->id == y->id)
  {
    y->size = x->size;
  }
  else
  {
    y->size = (number_count_bits(y) / 0x8) + 1;
  }
}

// Resize number
void number_resize(int size, number *num)
{
  int i = 0;
  if (num->size_max < size)
  {
    // Alocate initial memory
    char *bytes = malloc(size);
    // Copy memory to buffer
    for (i = 0; i < num->size_max; i++)
    {
      bytes[i] = num->bytes[i];
    }
    // Zero remaining memory
    for (i = num->size_max; i < size; i++)
    {
      bytes[i] = 0;
    }
    // Free original memory if exists
    if (num->bytes != NULL)
    {
      free(num->bytes);
    }
    // Set bytes pointer to new memory region
    num->bytes = bytes;
    num->size_max = size;
  }

  num->size = size;
}

// Multiply two numbers
void number_multiply(number *x, number *y, number *output)
{
  int i = 0, j = 0, z = 0;
  int offset = 0;
  int rec = 0;
  int x_size = 0;
  int y_size = 0;
  unsigned short x_s = 0, y_s = 0;
  unsigned short product = 0;
  int total_size = 0;
  int self = 0x0;
  int index = 0;

  x_size = x->size;
  y_size = y->size;

  // Check if result is negative
  if (x->is_negative != y->is_negative)
  {
    // should invert result (mult -1)
    self = 0x1;
  }

  total_size = x_size + y_size;

  number_resize(total_size, output);

  // Assign new id to result
  output->id = number_inc_gid();

  // Zero memory
  for (i = 0; i < total_size; i++)
  {
    unsigned char null_byte = 0;
    output->bytes[i] = null_byte;
  }

  // Loop through x,y
  for (i = 0; i < x_size; i++)
  {
    for (j = 0; j < y_size; j++)
    {
      offset = 0;
      x_s = (unsigned short)x->bytes[i];
      y_s = (unsigned short)y->bytes[j];
      // Calculate product between bytes
      product = x_s * y_s;

      // Invert if negative
      if (self)
      {
        product = 0xffff - product;
      }

      // If product > 0 carry bytes if neccessary
      if (product > 0)
      {
        while (product > 0)
        {
          index = offset + j + i;
          product = product + output->bytes[index];
          // 8 Bytes processed
          if (rec > 8)
          {
            rec = 0;
            // printf('processed 8 bytes')
          }

          // Add carry byte
          output->bytes[index] = product;
          // shift byte (2 nibbles)
          for (z = 0; z < 2; z++)
          {
            rec >>= 0x4;
            // shift nibble (4 bits)
            product = product >> 4;
          }
          rec++;
          offset++;
        }
      }
    }
  }
}

// Compute modulus of two large numbers x % y using exponentiation
void number_modulus(number *x, number *y)
{
  int diff = 0;
  int offset = 0;
  int count = 0;
  int bits = 0;
  int i = 0, j = 0;
  int size = 0;
  int index = 0;
  int rec = 0x1;
  int self = 0x0;
  unsigned char temp = 0;
  unsigned short adder = 0x0;
  unsigned char l_byte = 0, r_byte = 0;

  // Calculate bit difference between x and y
  diff = calc_bit_diff(x, y);
  if (diff == -1)
  {
    return;
  }

  // Resize x and y for processing
  number_resize(y->size + 2, y);
  number_resize(x->size + 2, x);

  // Modulus of self = 0
  if (x->id == y->id)
  {
    return 0;
  }
  // Number of left bit shifts
  offset = diff % 0x8;
  rec <<= 4;

  // x is new number
  // reassign id
  x->id = number_inc_gid();

  // Left shift y for processing (modular exponentiation)
  for (i = 0; i < offset; i++)
  {

    size = y->size;
    for (j = size - 1; j > 0; j--)
    {
      temp = y->bytes[j - 1];
      r_byte = temp & 0x080;
      r_byte = r_byte >> 0x7;

      // Modulus of negative number?
      if (x->is_negative != y->is_negative)
      {
        self = 0x1;
      }

      temp = y->bytes[j];
      l_byte = temp << 1;
      y->bytes[j] = l_byte | r_byte;
    }
    y->bytes[0] = y->bytes[0] << 1;
  }

  // While bits need to be exponentiated
  if (diff >= 0)
  {
    while (diff >= 0)
    {
      count = diff / 8;

      if (self)
      {
        rec >>= 4;
      }
      // Subtract y from x
      adder = 0;
      for (i = 0; i < y->size; i++)
      {
        index = count + i;
        adder = adder + x->bytes[index];

        // 8 bytes processed
        // update rec counter
        if (rec > 8)
        {
          rec = 0x0;
        }
        adder = adder - y->bytes[i];
        x->bytes[index] = adder & 0xFF;

        // Shift byte
        for (j = 0; j < 2; j++)
        {
          // Shift nibble
          rec <<= 4;
        }
        if (adder & 0x100)
        {
          rec = 0xFEFE;
          if (self)
          {
            adder = 0x0;
          }
          else
          {
            adder = 0xFFFF;
          }
        }
        else
        {
          rec = 0x0;
          adder = 0x0;
        }
      }

      // If bytes to carry
      if (adder != 0)
      {
        // Restore x (add y to x)
        adder = 0;
        for (i = 0; i < y->size; i++)
        {
          index = count + i;
          adder = x->bytes[index] + adder;
          adder = y->bytes[i] + adder;
          // Shift byte
          for (j = 0; j < 2; j++)
          {
            // Shift nibble
            rec <<= 4;
          }
          x->bytes[index] = adder & 0xFF;
          if (adder & 0x00100)
          {
            adder &= 0x00FF & rec;
            adder = 0x1;
          }
          else
          {
            adder = 0x0;
          }
        }
      }

      diff--;
      if (diff >= 0)
      {
        bits = diff % 0x8;
        if (bits == 0x7)
        {
          // Shift y 8 bits to left
          size = y->size;
          for (j = size - 1; j > 0; j--)
          {
            temp = y->bytes[j - 1];
            y->bytes[j] = temp;
          }
          y->bytes[0] = 0;
          //
        }

        size = y->size;
        if (y->bytes[0] & 0x1)
        {
          return;
        }
        // Shift y 1 bit to right
        for (j = 0; j < size - 1; j++)
        {
          l_byte = y->bytes[j] >> 1;
          r_byte = y->bytes[j + 1] & 0x01;

          if (rec > 8)
          {
            rec = 0;
            // printf('processed 8 bytes')
          }

          r_byte = r_byte << 7;
          y->bytes[j] = l_byte | r_byte;
        }
        l_byte = y->bytes[size - 1];
        y->bytes[size - 1] = l_byte >> 1;
      }
    }
  }

  // Set length
  number_size_xy(x, y);
}



// Calculate msg^e mod n
// public exponent (e) = 3

number *number_pow3m(number *msg, number *modulus)
{

  // Break msg^3 mod n into two
  // msg^3 mod n = msg(msg^2 mod n) mod n

  // Calculate expected modulus size
  int modulus_size = 0;
  int id = 0;
  int i = 0;
  int bytes = 0;
  int diff = 0;
  number *x = 0, *y = 0;

  modulus_size = modulus->size_max;

  // Allocate temp numbers: x,y
  y = number_alloc(modulus_size);
  x = number_alloc(3 * modulus_size);

  // Calculate msg^2 mod n
  // Store into x
  number_multiply(msg, msg, x);
  number_modulus(x, modulus);

  diff = calc_bit_diff(x, modulus);

  // copy x to y
  bytes = (number_count_bits(x) + 0x7) / 8;

  number_resize(bytes, y);

  for (i = 0; i < bytes; i++)
  {
    y->bytes[i] = x->bytes[i];
  }

  y->size = bytes;

  // Calculate msg (msg^2 mod n)
  // Store into x
  number_multiply(msg, y, x);
  // Calculate msg(msg^2 mod n) mod n
  // store final result in x
  number_modulus(x, modulus);

  // Free y's memory
  free(y->bytes);
  free(y);

  return x;
}

// Create large number from byte array
number *number_from_bytes(int size, unsigned char *bytes)
{
  int i = 0;
  // Initialize number
  number *num = number_alloc(size);
  // Copy bytes in reverse order
  for (i = 0; i < size; i++)
  {
    num->bytes[i] = bytes[size - i - 1];
  }
  // assume positive int
  //num->is_negative = 0;
  num->size = size;

  return num;
}

// Copy number out to byte array
void number_to_bytes(int size, unsigned char *output, number *x)
{
  int i = 0;
  int bytes = (number_count_bits(x) + 7) / 8;
  for (i = 0; i < size; i++)
  {
    output[i] = 0;
  }

  for (i = 0; i < bytes; i++)
  {
    output[bytes - i - 1] = x->bytes[i];
  }

  return 0;
}


// Encrypt str of length size with rsa pkcs 1.5
// msg = 0x00 || 0x02 || r || 0x00 || str
// where r is a random string 
void number_rsa_pkcs(unsigned char *str,
                     int size,
                     unsigned char *modulus,
                     unsigned char *output,
                     int rsa_bits)
{

  int bytes = 0;
  int i = 0;
  int index = 0;
  int index2 = 0;
  bytes = (rsa_bits + 7) / 8;


 

  // Set first and second byte
  output[0] = 0x0;
  output[1] = 0x2;

  index = bytes - size - 1;
  output[index] = 0x0;

  // Generate string r
  for (i = index - 1; i > 1; i--){
    output[i] = (char)number_rng_gen();
  }

  // Copy str
  for (i = 0; i < size; i++)
  {
    index = bytes - i - 1;
    index2 = size - 1 - i;
    output[index] = str[index2];
  }

  number *mod = number_from_bytes(bytes, modulus);
  number *msg = number_from_bytes(bytes, output);
  number *pow = number_pow3m(msg, mod);

  number_to_bytes(bytes, output, pow);

  free(mod->bytes);
  free(mod);

  free(msg->bytes);
  free(msg);

  free(pow->bytes);
  free(pow);

  return 0;
}
