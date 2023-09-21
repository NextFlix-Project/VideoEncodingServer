#include "RestAPI.h"
#include <stdio.h>
#include <fstream>
#include <Utilities/http.h>
#include <Utilities/files.h>

using namespace NextFlix;

RestAPI::RestAPI()
{
    this->threadCount = 1;
    this->initRestAPI(8800);
}

RestAPI::RestAPI(int port)
{
    this->threadCount = 1;
    this->initRestAPI(port);
}

RestAPI::RestAPI(int port, int threadCount)
{
    this->threadCount = threadCount;
    this->initRestAPI(port);
}

RestAPI::~RestAPI()
{
    app.stop();
}

int RestAPI::initRestAPI(int port)
{

    videoEncodingRoutes = new VideoEncodingRoutes(this);
    CROW_ROUTE(app, "/heartbeat")
    ([](const crow::request &, crow::response &res)
     {
        res.code = 200;
        res.end(); });

    try
    {
        app.port(port).concurrency(this->threadCount).run();
    }
    catch (const std::exception &e)
    {
        std::cout << "REST server failed to run" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;

        return 1;
    }

    return 0;
}

crow::SimpleApp &RestAPI::getApp()
{
    return this->app;
}