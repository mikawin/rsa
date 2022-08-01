#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gmp.h"
#include "rsa.h"
#include "numtheory.h"
#include "randstate.h"

// rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters)
// This function creates a new public key by creating p, q, n, and e.
void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    uint64_t p_bits, q_bits, upperbound, lowerbound;
    upperbound = (2 * nbits) / 4;
    lowerbound = nbits / 4;
    p_bits = (random() % upperbound) + lowerbound;
    q_bits = nbits - p_bits;

    // to ensure p and q bits are at least n bits
    p_bits += 1;
    q_bits += 1;

    mpz_t divisor, sub_p, sub_q, totient;
    mpz_inits(divisor, sub_p, sub_q, totient, NULL);

    make_prime(p, p_bits, iters);
    make_prime(q, q_bits, iters);

    mpz_mul(n, p, q);

    mpz_sub_ui(sub_p, p, 1);
    mpz_sub_ui(sub_q, q, 1);
    mpz_mul(totient, sub_p, sub_q);

    while (mpz_cmp_ui(divisor, 1) != 0) {
        mpz_urandomb(e, state, nbits);
        gcd(divisor, e, totient);
    }
    mpz_clears(divisor, sub_p, sub_q, totient, NULL);
}

// rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile)
// This function writes the public key.
void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, "%Zx\n", e);
    gmp_fprintf(pbfile, "%Zx\n", s);
    gmp_fprintf(pbfile, "%s\n", username);
}

// rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char usernmae[], FILE *pbfile)
// This function reads the public key.
void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n", n);
    gmp_fscanf(pbfile, "%Zx\n", e);
    gmp_fscanf(pbfile, "%Zx\n", s);
    gmp_fscanf(pbfile, "%s\n", username);
}

// rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q)
// This function creates the private key by creating d and e.
void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t term, sub_p, sub_q;
    mpz_inits(term, sub_p, sub_q, NULL);
    mpz_sub_ui(sub_p, p, 1);
    mpz_sub_ui(sub_q, q, 1);
    mpz_mul(term, sub_p, sub_q);
    mod_inverse(d, e, term);
    mpz_clears(term, sub_q, sub_p, NULL);
}

// rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile)
// This function writes the private key.
void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", n);
    gmp_fprintf(pvfile, "%Zx\n", d);
}

// rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile)
// This functions reads the private key.
void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n", n);
    gmp_fscanf(pvfile, "%Zx\n", d);
}

// rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n)
// This function encrypts the message by calling the powmod function.
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    pow_mod(c, m, e, n);
}

// rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpzt_t e)
// This function encrypts the contents of an input file and writes them to the output file.
void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    mpz_t c, m;
    mpz_inits(c, m, NULL);

    size_t k = 0;
    k = (mpz_sizeinbase(n, 2) - 1) / 8;

    uint8_t *encrypt_block = (uint8_t *) calloc(k, sizeof(uint8_t));
    size_t bytes_read = 1;

    encrypt_block[0] = 0xFF;

    while (feof(infile) == 0) {
        bytes_read = fread(&encrypt_block[1], sizeof(uint8_t), k - 1, infile);
        if (bytes_read > 0) {
            mpz_import(m, bytes_read + 1, 1, sizeof(uint8_t), 1, 0, encrypt_block);
            rsa_encrypt(c, m, e, n);
            gmp_fprintf(outfile, "%Zx\n", c);
        }
    }
    mpz_clears(c, m, NULL);
    free(encrypt_block);
}

// rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n)
// This function decrypts the message by calling the powmod function.
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    pow_mod(m, c, d, n);
}

// rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d)
// This function decrypts the contents of an input file and writes them to the output file.
void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    mpz_t c, m;
    mpz_inits(c, m, NULL);

    size_t k = 0;
    k = (mpz_sizeinbase(n, 2) - 1) / 8;
    uint8_t *decrypt_block = (uint8_t *) calloc(k, sizeof(uint8_t));
    size_t j = 0;

    while (feof(infile) == 0) {
        if (gmp_fscanf(infile, "%Zx\n", c) > 0) {
            rsa_decrypt(m, c, d, n);
            mpz_export(decrypt_block, &j, 1, sizeof(uint8_t), 1, 0, m);
            fwrite(&decrypt_block[1], sizeof(uint8_t), j - 1, outfile);
        }
    }
    free(decrypt_block);
    mpz_clears(c, m, NULL);
}

// rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n)
// This function produces a signature by signing a message using the private key.
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    pow_mod(s, m, d, n);
}

// rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n)
// This function checks if the inverse of the signature is equal to the encrypted message.
bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    mpz_t t;
    mpz_inits(t, NULL);
    pow_mod(t, s, e, n);
    if (mpz_cmp(t, m) == 0) {
        mpz_clears(t, NULL);
        return true;
    }
    mpz_clears(t, NULL);
    return false;
}
