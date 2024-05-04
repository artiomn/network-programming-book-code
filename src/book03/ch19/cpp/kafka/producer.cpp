#include <cppkafka/cppkafka.h>


int main()
{
    cppkafka::Configuration config = {{"metadata.broker.list", "127.0.0.1:9092"}};

    cppkafka::Producer producer(config);

    std::string message = "hey there!";
    producer.produce(cppkafka::MessageBuilder("mytopic").partition(0).payload(message));
    producer.flush();
}
