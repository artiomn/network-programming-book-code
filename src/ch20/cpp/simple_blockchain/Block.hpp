// author: tko
#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.hh"
using json = nlohmann::json;

class Block
{
public:
    Block(int index, std::string prevHas, std::string hash, std::string nonce, std::vector<std::string> data);
    std::string getPreviousHash(void);
    std::string getHash(void);
    int getIndex(void);
    std::vector<std::string> getData(void);

    void toString(void);
    json toJSON(void);

private:
    int index;
    std::string previousHash;
    std::string blockHash;
    std::string nonce;
    std::vector<std::string> data;
    // string getMerkleRoot(const vector<string> &merkle);
};
// Constructor
Block::Block(int index, std::string prevHash, std::string hash, std::string nonce, std::vector<std::string> data)
{
    printf("\nInitializing Block: %d ---- Hash: %s \n", index, hash.c_str());
    this->previousHash = prevHash;
    this->data = data;
    this->index = index;
    this->nonce = nonce;
    this->blockHash = hash;
}

int Block::getIndex(void)
{
    return this->index;
}

std::string Block::getPreviousHash(void)
{
    return this->previousHash;
}

std::string Block::getHash(void)
{
    return this->blockHash;
}

std::vector<std::string> Block::getData(void)
{
    return this->data;
}

// Prints Block data
void Block::toString(void)
{
    std::string dataString;
    for (int i = 0; i < data.size(); i++) dataString += data[i] + ", ";
    printf("\n-------------------------------\n");
    printf(
        "Block %d\nHash: %s\nPrevious Hash: %s\nContents: %s", index, this->blockHash.c_str(),
        this->previousHash.c_str(), dataString.c_str());
    printf("\n-------------------------------\n");
}

json Block::toJSON(void)
{
    json j;
    j["index"] = this->index;
    j["hash"] = this->blockHash;
    j["previousHash"] = this->previousHash;
    j["nonce"] = this->nonce;
    j["data"] = this->data;
    return j;
}

#endif
