#include <jwt-cpp/jwt.h>
#include <iostream>


int main()
{
    auto token = jwt::create()
        .set_issuer("auth0")
        .set_type("JWT")
        .set_payload_claim("sample", jwt::claim(std::string("test")))
        .sign(jwt::algorithm::hs256{"secret"});

    std::cout << token << std::endl;

    auto decoded = jwt::decode(token);

    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{ "secret" })
        .with_issuer("auth0");

    verifier.verify(decoded);

    for(auto& e : decoded.get_payload_claims())
        std::cout << e.first << " = " << e.second << std::endl;
}
