#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


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

    // initialize init_size char array
    int init_size = 10;
    char * chars = (char*) calloc( init_size, sizeof(char));
    for(int x = 0; x < init_size; x++)
        chars[x]=0;

    /* Inicializace OpenSSL hash funkci, deprecated */
    OpenSSL_add_all_digests();

    const EVP_MD* type = EVP_get_digestbyname("sha384");
    if (type == NULL)
        return 1;

    int x=0;
    // spagetti whatever, it is C, it is ugly itself
    while(1){

        // generate sequential ascii chars (33 to 125 to be printable)
        while(x < init_size){

            if(chars[x] < CHAR_MAX){
                chars[x]++;
                break;
            } else {
                x++;
            }
            // realloc something slightly bigger
            if(chars[init_size-1] == CHAR_MAX){
                init_size+=2;
                chars = realloc(chars, init_size * sizeof(char));
                chars[init_size-1] = 0;
                chars[init_size-2] = 0;
            }
        }

        EVP_MD_CTX* ctx = EVP_MD_CTX_create(); // create context for hashing
        if (ctx == NULL)
            return 2;

        auto context_initialized = EVP_DigestInit_ex(ctx, type, NULL);
        if (! context_initialized)
            return 3;

        auto message_processed = EVP_DigestUpdate(ctx, chars, strlen(chars));
        if (! message_processed)
            return 4;

        unsigned char hash[EVP_MAX_MD_SIZE]; // char array for hash - 64 bytes (max for sha 512)
        unsigned int length;  // total len of hash

        auto message_hashed = EVP_DigestFinal_ex(ctx, hash, &length);
        if (! message_hashed)
            return 5;

        EVP_MD_CTX_destroy(ctx); // destroy the context

        int flag = 1;

        int index = 0; // first bits counter
        // cycle through char array
        for (int i = 0; i < init_size; i++){
            char b = hash[i];
            // bits
            for(int bit = 7; 0 <= bit; bit --)
                if(index <= number_of_zeros)
                {
                    if((char) ((b >> bit) & 0x01) == 1){
                        flag = 0;
                        break;
                }
                index++;
            }
        }

        if(flag)
        {
            // output: first line: hex content of the message
            for (unsigned int i = 0; i < init_size; i++)
                printf("%02x", chars[i]);
            printf("\n");
            // second line: hash
            for (unsigned int i = 0; i < length; i++)
                printf("%02x", hash[i]);
            printf("\n");

            // free alocated memory
            free(chars);
            return 0;
        }
    }
}


