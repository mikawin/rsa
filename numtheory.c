#include "numtheory.h"
#include "randstate.h"

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>

gmp_randstate_t state;

// pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus)
// This function computs a base raised to an exponent modulo another value
//
// v : temporary variable for out
// p : temporary variable for base
// mul_base : value of and multiplication involving base
// temp_exp : temporary variable for exponent
// mul_out : a factor of the multiplication that results in the out variable
void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_t v, p, mul_base, temp_exp, mul_out;
    mpz_inits(v, p, mul_base, temp_exp, mul_out, NULL);
    mpz_set_ui(v, 1);
    mpz_set(p, base);
    mpz_set(temp_exp, exponent);
    while (mpz_cmp_ui(temp_exp, 0) > 0) {
        if (mpz_odd_p(temp_exp)) {
            mpz_mul(mul_out, v, p);
            mpz_mod(v, mul_out, modulus);
        }
        mpz_mul(mul_base, p, p);
        mpz_mod(p, mul_base, modulus);
        mpz_fdiv_q_ui(temp_exp, temp_exp, 2);
    }
    mpz_set(out, v);
    mpz_clears(v, p, mul_base, temp_exp, mul_out, NULL);
}

// is_prime(mpz_t n, uint64_6 iters)
// This function determines if a number is prime using the Miller-Rabin algorithm
//
// There are many variables that have been initialized. They are variables used in the arithmetic
// for the algorithm as well as temp variables to prevent altering.
bool is_prime(mpz_t n, uint64_t iters) {
    mpz_t y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a, exponent;
    mpz_inits(y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a, exponent, NULL);
    mpz_set(test_n, n);
    mpz_sub_ui(test_n, test_n, 1);

    if (mpz_cmp_ui(n, 2) == 0 || mpz_cmp_ui(n, 3) == 0) {
        mpz_clears(y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a, exponent, NULL);
        return true;
    }

    mpz_mod_ui(mod_eval, n, 2);
    // checks if the number is even, 0, or 1 -> these are not prime
    if (mpz_cmp_ui(mod_eval, 0) == 0 || mpz_cmp_ui(n, 0) == 0 || mpz_cmp_ui(n, 1) == 0) {
        mpz_clears(y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a, exponent, NULL);
        return false;
    }

    while (mpz_even_p(r)) {
        mpz_fdiv_q_ui(r, test_n, 2);
        mpz_add_ui(s, s, 1);
        mpz_set(test_n, r);
    }

    for (uint64_t count = 1; count <= iters; count += 1) {
        // range (2, n-2)
        mpz_sub_ui(upper, n, 3);
        mpz_urandomm(rand, state, upper);
        mpz_add_ui(a, rand, 2);
        pow_mod(y, a, r, n);

        mpz_sub_ui(temp_n, n, 1);
        mpz_sub_ui(temp_s, s, 1);

        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, temp_n) != 0) {
            mpz_set_ui(j, 1);

            while (mpz_cmp(j, temp_s) <= 0 && mpz_cmp(y, temp_n) != 0) {
                mpz_set_ui(exponent, 2);
                pow_mod(y, y, exponent, n);
                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a,
                        exponent, NULL);
                    return false;
                }
                mpz_add_ui(j, j, 1);
            }

            if (mpz_cmp(y, temp_n) != 0) {
                mpz_clears(
                    y, rand, j, r, s, temp_s, test_n, temp_n, mod_eval, upper, a, exponent, NULL);
                return false;
            }
        }
    }
    mpz_clears(y, rand, j, r, s, temp_s, temp_n, test_n, mod_eval, upper, a, exponent, NULL);
    return true;
}

// make_prime(mpz_t p, uint64_t bits, uint64_t iters)
// This function makes a prime number by getting a random number and calling isprime() to determine
// if the number is actually prime.
//
// check : a boolean the checks if the number is prime or not
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    bool check = false;
    mpz_urandomb(p, state, bits);
    check = is_prime(p, iters);
    while (!check || mpz_sizeinbase(p, 2) < bits) {
        mpz_urandomb(p, state, bits);
        check = is_prime(p, iters);
    }
}

// gcd(mpz_t d, mpz_t a, mpz_t b)
// This function computes the greatest common divisor of two numbers and returns the value by
// setting the gcd equal to d
//
// temp_b , temp_a : temp variables for b and a respectively
// t : variable used to compute the gcd
void gcd(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t t, temp_b, temp_a;
    mpz_inits(t, temp_b, temp_a, NULL);
    mpz_set(temp_b, b);
    mpz_set(temp_a, a);
    while (mpz_cmp_ui(temp_b, 0) != 0) {
        mpz_set(t, temp_b);
        mpz_mod(temp_b, temp_a, temp_b);
        mpz_set(temp_a, t);
    }
    mpz_set(d, temp_a);
    mpz_clears(t, temp_b, temp_a, NULL);
}

// mod_inverse(mpz_t i, mpz_t a, mpz_t n)
// This function computes the inverse of a modulo
void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    mpz_t q_r, q_t, r, r_prime, t, t_prime, temp_r, temp_rprime, temp_t, temp_tprime;
    mpz_inits(q_r, q_t, r, r_prime, t, t_prime, temp_r, temp_rprime, temp_t, temp_tprime, NULL);

    mpz_set(r, n);
    mpz_set(r_prime, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t_prime, 1);

    mpz_set(temp_r, r);
    mpz_set(temp_rprime, r_prime);
    mpz_set(temp_t, t);
    mpz_set(temp_tprime, t_prime);

    while (mpz_cmp_ui(temp_rprime, 0) != 0) {
        mpz_fdiv_q(q_r, temp_r, temp_rprime);
        mpz_set(q_t, q_r);

        mpz_set(r, temp_rprime);
        mpz_mul(q_r, q_r, temp_rprime);
        mpz_sub(r_prime, temp_r, q_r);

        mpz_set(t, temp_tprime);
        mpz_mul(q_t, q_t, temp_tprime);
        mpz_sub(t_prime, temp_t, q_t);

        mpz_set(temp_r, r);
        mpz_set(temp_rprime, r_prime);
        mpz_set(temp_t, t);
        mpz_set(temp_tprime, t_prime);
    }
    if (mpz_cmp_ui(temp_r, 1) > 0) {
        mpz_set_ui(i, 0);
        mpz_clears(
            q_r, q_t, r, r_prime, t, t_prime, temp_r, temp_rprime, temp_t, temp_tprime, NULL);
        return;
    }
    if (mpz_cmp_ui(temp_t, 0) < 0) {
        mpz_add(t, temp_t, n);
    }
    mpz_set(i, t);
    mpz_clears(q_r, q_t, r, r_prime, t, t_prime, temp_r, temp_rprime, temp_t, temp_tprime, NULL);
}
