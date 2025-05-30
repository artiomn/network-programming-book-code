// author: tko
#ifndef _HASH_H_
#define _HASH_H_
#include <openssl/ripemd.h>
#include <openssl/sha.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;


string sha256(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }

    return ss.str();
}

#endif
