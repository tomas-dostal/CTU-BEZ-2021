#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define INPUT_SIZE 8
int check_requirements(const unsigned char hash[EVP_MAX_MD_SIZE], uint32_t hash_len, long initial_zeros) {
    int flag = 1;

    int counter = 0; // first bits counter
    // cycle through char array
    for (long i = 0; i < hash_len; i++) {
        unsigned char b = hash[i];

        // look for bits in each char.
        for(int bit = 7; 0 <= bit; bit --) {
            if (counter <= initial_zeros) {
                if ((char) ((b >> bit) & 0x01) == 1) {
                    flag = 0;
                    break;
                }
                counter++;
            }
        }
        if(counter >= initial_zeros)
            return flag;
    }
    return flag;
}

int main (int argc, char * argv[]) {
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s number_od_zero_bits\n", argv[0]);
        return 7;
    }
    // binary has to take only one parameter - number of zero bits (from start) required
    char *ptr;
    long number_of_zeros = strtol(argv[1], &ptr, 10);

    if(number_of_zeros > 384){
        fprintf(stderr, "Too many (%s) zero bits required", argv[0]);
        return 8;
    }

    /* Init OpenSSL hash functions, deprecated */
    OpenSSL_add_all_digests();

    const EVP_MD* type = EVP_get_digestbyname("sha384");
    if (type == NULL)
        return 1;

    // let's try 8byte uint as a key
    for(uint64_t input = 0; input < LONG_MAX; input++){

        EVP_MD_CTX* ctx = EVP_MD_CTX_create(); // create context for hashing
        if (ctx == NULL)
            return 2;

        auto context_initialized = EVP_DigestInit_ex(ctx, type, NULL);
        if (! context_initialized)
            return 3;
        unsigned  char * in_string = (unsigned char *) &input;

        auto message_processed = EVP_DigestUpdate(ctx, in_string, INPUT_SIZE);
        if (! message_processed)
            return 4;

        unsigned char hash[EVP_MAX_MD_SIZE]; // char array for hash - 64 bytes (max for sha 512)
        unsigned int hashed_length;  // total len of hash

        auto message_hashed = EVP_DigestFinal_ex(ctx, hash, &hashed_length);
        if (! message_hashed)
            return 5;

        EVP_MD_CTX_destroy(ctx); // destroy the context

        if(check_requirements(hash, hashed_length, number_of_zeros)){
            // output: first line: hex content of the message
            for (unsigned long i = 0; i < INPUT_SIZE; i++)
                printf("%02x", in_string[i]);
            printf("\n");
            // second line: hash
            for (unsigned int i = 0; i < hashed_length; i++)
                printf("%02x", hash[i]);
            printf("\n");

            return 0;
        }
    }
}


