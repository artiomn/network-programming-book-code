#include <mr4c/algo_dev_api.h>

#include <iostream>


using namespace MR4C;


class HelloWorld : public Algorithm
{
public:
    void executeAlgorithm(AlgorithmData& data, AlgorithmContext& context)
    {
        std::string nativeHdr = "\n**************NATIVE_OUTPUT**************\n";
        std::cout << nativeHdr << std::endl;

        std::cout << "  Hello World!!" << std::endl;

        std::cout << nativeHdr << std::endl;
    }

    static Algorithm* create()
    {
        HelloWorld* algo = new HelloWorld();
        return algo;
    }
};


MR4C_REGISTER_ALGORITHM(HelloWorld, HelloWorld::create());
