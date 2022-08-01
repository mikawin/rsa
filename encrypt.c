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
    FILE *pbfile = fopen("rsa.pub", "r");

    bool print_verbose = false;
    bool print_help = false;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {

        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n':
            pbfile = fopen(optarg, "r");
            // if the file doesn't exist, print error and exit
            if (pbfile == NULL) {
                printf("Error: Unable to open public file.\n");
                fclose(pbfile);
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
        printf("    Encrypts data using RSA encryption.\n");
        printf("    Encrypted data is decrypted by the decrypt program.\n");
        printf("USAGE\n");
        printf("   ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey\n");
        printf("OPTIONS\n");
        printf("   -h              Display program help and usage.\n");
        printf("   -v              Display verbose program output.\n");
        printf("   -i infile       Input file of data to encrypt (default: stdin).\n");
        printf("   -o outfile      Output file for encrypted data (default: stdout).\n");
        printf("   -n pbfile       Public key file (default: rsa.pub).\n");

    } else {
        mpz_t m, n, e, s;
        mpz_inits(m, n, e, s, NULL);
        char user[32];

        rsa_read_pub(n, e, s, user, pbfile);
        if (print_verbose == true) {
            printf("user = %s\n", user);
            gmp_printf("s (%zu bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
            gmp_printf("n (%zu bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
            gmp_printf("e (%zu bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
        }
        mpz_set_str(m, user, 62);

        if (!rsa_verify(m, s, e, n)) {
            printf("Signature was not verified.\n");
            exit(EXIT_FAILURE);
        }

        rsa_encrypt_file(infile, outfile, n, e);

        fclose(infile);
        fclose(outfile);
        fclose(pbfile);
        mpz_clears(m, n, e, s, NULL);
    }
}
