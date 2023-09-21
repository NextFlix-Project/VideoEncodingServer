#pragma once

#include <crow.h>
#include "../../Models/Models.h"
#include "Routes/VideoEncodingRoutes.h"

namespace NextFlix
{
    class RestAPI
    {
    public:
        RestAPI();
        RestAPI(int port);
        RestAPI(int port, int threadCount);
        ~RestAPI();
        crow::SimpleApp &getApp();

    private:
        crow::SimpleApp app;
        int threadCount;
        Models *models;
        VideoEncodingRoutes *videoEncodingRoutes;

    private:
        int initRestAPI(int port);
    };
}