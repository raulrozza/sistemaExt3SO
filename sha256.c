#include "sha256.h"


void printSha256(const char *path){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    unsigned int fileSize = 0;
    BIO* fileBio = BIO_new_file(path, "r");
    char data;
    
    while(BIO_read(fileBio, &data, 1) > 0){
        fileSize++;
        SHA256_Update(&sha256, &data, 1);
    }
    
    SHA256_Final(hash, &sha256);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        printf("%.2x", hash[i]);
    }
    
    BIO_free(fileBio);
}
