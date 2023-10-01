#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include <gmp.h>

#include "logger.h"

#define SIZE_PRIME 512
#define E 65537

// Generate a random prime number for the rsa and store it in res
void randomPrime(mpz_t res) {
  // make var tmp
  mpz_t tmp;
  mpz_init(tmp);

  // buffer to hold bits of random number
  char buffer[SIZE_PRIME];

  // Hide cursor for loading bar
  printf("\e[?251"); 
  // Print starting bar
  printf("%sGenerating random number:%s [                                ]", green, clear);
  
  // Set random seed
  srand((unsigned int)time(0));

  // Generate random number in buffer
  for (int i = 0; i < SIZE_PRIME; i++) {
    // Set bits of buffer
    buffer[i] = rand() % 0xff;

    // Print the loading bar
    printf("\r");
    fflush(stdout);
    printf("%sGenerating random number:%s [", green, clear);
    int curr = 32 * ((double)i / SIZE_PRIME);
    for (int i = 0; i < 32; i++) {
      if (i <= curr) printf("#");
      if (i > curr) printf("_");
    }
    printf("]");
  }
  // Newline, re-show cursor
  printf("\n\e[?25h");

  // Top 2 bits set high
  buffer[0] |= 0xC0;
  // Bottom bit set to 1 (ensure odd)
  buffer[SIZE_PRIME - 1] |= 0x01;

  // import into mpz_t res
  mpz_import(res, SIZE_PRIME, 1, sizeof(buffer[0]), 0, 0, buffer);

  // Get next prime, check if it is good
  mpz_nextprime(res, res);
  mpz_mod_ui(tmp, res, E);
  printf("%sGenerated prime.\n", info);

  while (!mpz_cmp_ui(tmp, 1)) {
    mpz_nextprime(res, res);
    mpz_mod_ui(tmp, res, E);
    printf("%sGenerated prime.\n", info);
  }
}

int main(int argc, char** argv) {
  // Parse command line arguments
  bool showkeys = false;
  for (int i = 0; i < argc; i++)
    if (strcmp(argv[i], "--show-keys\0") == 0) showkeys = true;

  // Declare big vars
  mpz_t p;
  mpz_t q;
  mpz_t n;
  mpz_t e;
  mpz_t d;
  mpz_t tmp1;
  mpz_t tmp2;
  mpz_t phiN;

  mpz_t C;
  mpz_t M;

  // Initialise big vars
  mpz_init(p);
  mpz_init(q);
  mpz_init(n);
  mpz_init(e);
  mpz_init(d);
  mpz_init(tmp1);
  mpz_init(tmp2);
  mpz_init(phiN);

  mpz_init(C);
  mpz_init(M);

  // Generate primes
  randomPrime(p);
  randomPrime(q);
  printf("\n");
  while (mpz_cmp(p, q) == 0) randomPrime(q);
  if (showkeys) {
    gmp_printf("%sGenerated p: %s%Zd\n", green, clear, p);
    gmp_printf("%sGenerated q: %s%Zd\n", green, clear, q);
  } else {
    printf("%sGenerated p.\n", success);
    printf("%sGenerated q.\n", success);
  }

  // Generate n
  mpz_mul(n, p, q);
  if (showkeys)
    gmp_printf("%sGenerated n: %s%Zd\n", green, clear, n);
  else
    printf("%sGenerated n.\n\n", success);

  // Generate phi(n)
  mpz_sub_ui(tmp1, p, 1);
  mpz_sub_ui(tmp2, q, 1);
  mpz_mul(phiN, tmp1, tmp2);
  if (showkeys)
    gmp_printf("%sGenerated phi(n): %s%Zd\n", green, clear,
               phiN);
  else
    printf("%sGenerated phi(n).\n\n", success);

  // Generate e
  mpz_set_ui(e, E);
  if (showkeys)
    gmp_printf("%sGenerated e: %s%Zd\n", green, clear, e);
  else
    printf("%sGenerated e.\n", success);

  // Generate d
  mpz_invert(d, e, phiN);
  if (showkeys)
    gmp_printf("%sGenerated d: %s%Zd\n", green, clear, d);
  else
    printf("%sGenerated d.\n\n", success);

  // ENCRYPTING AN INTEGER - 37
  int initial = 37;
  int result = 0;

  // Encrypt
  mpz_set_ui(M, initial);
  mpz_powm(C, M, e, n);

  // Decrypt
  mpz_powm(M, C, d, n);
  result = mpz_get_ui(M);

  // Print results
  printf("%sInitial: %s%d\n", green, clear, initial);
  gmp_printf("%sEncrypted: %s%Zd\n", green, clear, C);
  printf("%sResult: %s%d\n", green, clear, result);

  // ENCRYPTING A STRING

  // Declare message
  //char message[SIZE_PRIME] =
      //"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque "
      //"venenatis sollicitudin neque et lobortis. Quisque nec elit nec sem "
      //"porta maximus. Proin dignissim id risus malesuada sodales. Nunc "
      //"interdum, dui sit amet dignissim sollicitudin, magna sem auctor dui, at "
      //"euismod sem ex at diam. Cras consectetur mi ac sem fermentum aliquam. "
      //"Sed in pellentesque mauris. Aliquam sagittis dictum ipsum et "
      //"condimentum. Donec neque elit, efficitur vel orci nec, laoreet interdum "
      //"metus. Nam in nulla nibh viverra fusce.\0";
  char message[SIZE_PRIME];
  printf("\n%sENTER MESSAGE: %s", green, clear);
  fgets(message, 512, stdin);
  printf("\n");

  mpz_import(M, SIZE_PRIME, 1, sizeof(message[0]), 0, 0, message);
  printf("%sMessage (string form): %s%s\n", green, clear,
         message);
  gmp_printf("%sMessage (number form): %s%Zd\n", green, clear,
             M);

  // Encrypt message
  mpz_powm(C, M, e, n);
  gmp_printf("%sEncrypted message: %s%Zd\n", green, clear, C);

  // Decrypt message
  mpz_powm(M, C, d, n);
  gmp_printf("%sDecrypted message (number form): %s%Zd\n", green,
             clear, M);

  char decrypted[SIZE_PRIME];
  size_t size = 0;
  mpz_export(decrypted, &size, 1, 1, 1, 0, M);
  printf("%sDecrypted message (string form): %s%s\n", green,
         clear, decrypted);

  // Free big vars
  mpz_clear(p);
  mpz_clear(q);
  mpz_clear(n);
  mpz_clear(e);
  mpz_clear(d);
  mpz_clear(tmp1);
  mpz_clear(tmp2);
  mpz_clear(phiN);

  mpz_clear(C);
  mpz_clear(M);

  return 0;
}
