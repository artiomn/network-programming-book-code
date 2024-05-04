// author: tko
#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "hash.hpp"

void print_hex(const char *label, const uint8_t *v, size_t len)
{
    size_t i;

    printf("%s: ", label);
    for (i = 0; i < len; ++i)
    {
        printf("%02x", v[i]);
    }
    printf("\n");
}

std::string getMerkleRoot(const std::vector<std::string> &merkle)
{
    printf("\nFinding Merkle Root.... \n");
    if (merkle.empty())
        return "";
    else if (merkle.size() == 1)
    {
        return sha256(merkle[0]);
    }

    std::vector<std::string> new_merkle = merkle;

    while (new_merkle.size() > 1)
    {
        if (new_merkle.size() % 2 == 1) new_merkle.push_back(merkle.back());

        std::vector<std::string> result;

        for (int i = 0; i < new_merkle.size(); i += 2)
        {
            std::string var1 = sha256(new_merkle[i]);
            std::string var2 = sha256(new_merkle[i + 1]);
            std::string hash = sha256(var1 + var2);
            // printf("---hash(hash(%s), hash(%s)) => %s\n",new_merkle[0].c_str(),new_merkle[1].c_str(),hash.c_str());
            result.push_back(hash);
        }
        new_merkle = result;
    }
    return new_merkle[0];
}
std::pair<std::string, std::string> findHash(int index, std::string prevHash, std::vector<std::string> &merkle)
{
    std::string header = std::to_string(index) + prevHash + getMerkleRoot(merkle);
    unsigned int nonce;
    for (nonce = 0; nonce < 100000; nonce++)
    {
        std::string blockHash = sha256(header + std::to_string(nonce));
        if (blockHash.substr(0, 2) == "00")
        {
            // cout << "nonce: " << nonce;
            // cout << "header: " << header;
            return std::make_pair(blockHash, std::to_string(nonce));
            break;
        }
    }
    return std::make_pair("fail", "fail");
}
// int addBlock(int index, std::string prevHash, std::vector<std::string> &merkle, std::vector<unique_ptr<Block> > &blockchain) {
//     std::string header = to_std::string(index) + prevHash + getMerkleRoot(merkle);
//     auto std::pair = findHash(header);

//     blockchain.push_back(std::make_unique<Block>(index,prevHash,std::pair.first,std::pair.second,merkle));
//     return 1;
// }
#endif
