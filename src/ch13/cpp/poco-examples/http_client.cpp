#include <iostream>

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/StreamCopier.h>


int main(int argc, const char* argv[])
{
    using namespace Poco::Net;

    HTTPClientSession s("www.artiomsoft.ru");
    // s.setProxy("localhost", srv.port());

    HTTPRequest request(HTTPRequest::HTTP_GET, "/large");
    HTMLForm form;

    form.add("entry1", "value1");
    form.prepareSubmit(request);

    s.sendRequest(request);

    HTTPResponse response;
    std::istream& rs = s.receiveResponse(response);
    Poco::StreamCopier::copyStream(rs, std::cout);
}
