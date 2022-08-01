#include <stdio.h>
#include "randstate.h"
#include "rsa.h"
#include "numtheory.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define OPTIONS "vhi:o:n:"

int main(int argc, char **argv) {
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *pvfile = fopen("rsa.priv", "r");

    bool print_verbose = false;
    bool print_help = false;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {

        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n':
            pvfile = fopen(optarg, "r");
            if (pvfile == NULL) {
                printf("Error: Unable to open the private file.\n");
                fclose(pvfile);
                fclose(infile);
                fclose(outfile);
                exit(EXIT_FAILURE);
            }
            break;
        case 'v': print_verbose = true; break;
        case 'h': print_help = true; break;
        default: break;
        }
    }
    if (print_help) {
        printf("SYNOPSIS\n");
        printf("    Decrypts data using RSA decryption.\n");
        printf("    Encrypted data is encrypted by the encrypt program.\n");
        printf("USAGE\n");
        printf("   ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey\n");
        printf("OPTIONS\n");
        printf("   -h              Display program help and usage.\n");
        printf("   -v              Display verbose program output.\n");
        printf("   -i infile       Input file of data to decrypt (default: stdin).\n");
        printf("   -o outfile      Output file for decrypted data (default: stdout).\n");
        printf("   -n pvfile       Public key file (default: rsa.priv).\n");

    } else {
        mpz_t n, d;
        mpz_inits(n, d, NULL);

        rsa_read_priv(n, d, pvfile);

        if (print_verbose) {
            gmp_printf("n (%zu bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
            gmp_printf("d (%zu bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
        }
        rsa_decrypt_file(infile, outfile, n, d);

        fclose(outfile);
        fclose(infile);
        fclose(pvfile);
        mpz_clears(n, d, NULL);
    }
}
