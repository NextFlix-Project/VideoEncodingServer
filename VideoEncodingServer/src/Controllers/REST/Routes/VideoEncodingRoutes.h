#pragma once

#include <crow.h>

namespace NextFlix
{
    class VideoEncoder;
    class Models;
    class RestAPI;

    class VideoEncodingRoutes
    {
    private:
        RestAPI *restAPI;
        int threadCount;
        Models *models;
        VideoEncoder *encoder;

    public:
        VideoEncodingRoutes(RestAPI *restAPI);
        ~VideoEncodingRoutes();

    private:
        void initRoutes();
         void uploadToStreamingServer(std::string path, std::string id);
     };
}