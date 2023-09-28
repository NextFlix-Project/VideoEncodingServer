#include "VideoEncodingRoutes.h"
#include <fstream>
#include <iostream>
#include <string>
#include <Utilities/http.h>
#include <Utilities/files.h>
#include "../../../VideoEncoder/VideoEncoder.h"
#include "../../../Models/Models.h"
#include "../RestAPI.h"
#include <vector>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pugixml.hpp>
#include <filesystem>

using namespace NextFlix;

VideoEncodingRoutes::VideoEncodingRoutes(RestAPI *restAPI)
{
    this->restAPI = restAPI;
    this->encoder = new VideoEncoder();

    this->initRoutes();
}

VideoEncodingRoutes::~VideoEncodingRoutes()
{
    delete this->encoder;
}

void VideoEncodingRoutes::initRoutes()
{
    CROW_ROUTE(this->restAPI->getApp(), "/codecs")
    ([this]()
     { return this->encoder->getAvailableCodecs(); });

    CROW_ROUTE(this->restAPI->getApp(), "/encode")
        .methods("POST"_method)([this](const crow::request &req, crow::response &res)
                                {

                                    std::string id = req.url_params.get("id");
       if (!isMultipartFormData(req.get_header_value("Content-Type")))
        {
            res.code = 400;
            res.write("Invalid request");
            res.end();
            return;
        }
        
        size_t boundaryPos = 0;
        std::string boundary = getBoundaryFromHeader(req.get_header_value("Content-Type"), boundaryPos);

        std::vector<std::string> parts;
        std::string body = req.body;
        
        while (boundaryPos != std::string::npos) 
        {      
            size_t nextBoundaryPos = body.find(boundary, boundaryPos + boundary.size());
        
            if (nextBoundaryPos != std::string::npos)
            {
                std::string part = body.substr(boundaryPos + boundary.size(), nextBoundaryPos - (boundaryPos + boundary.size()));
                parts.push_back(part);
            }
            boundaryPos = nextBoundaryPos;
        }
        
        std::string outputFile = "";
        std::string inputFile = "";

        for (const auto& part : parts) 
        {
            size_t filenamePos = part.find("filename=\"");
        
            if (filenamePos != std::string::npos)
            {
                size_t filenameEndPos = part.find("\"", filenamePos + 10);
                
                if (filenameEndPos != std::string::npos)
                {
                    std::string filename = part.substr(filenamePos + 10, filenameEndPos - (filenamePos + 10));
                    size_t contentPos = part.find("\r\n\r\n") + 4;
                    std::string content = part.substr(contentPos);

                    if (!doesDirectoryExist("./queue/"+  id) )
                        createDirectory("./queue/"+ id );


                    if (!doesDirectoryExist("./output/"+  id) )
                        createDirectory("./output/"+ id );

                    std::string savePath = "./queue/" + id + "/" + filename;
                    inputFile = savePath;
                    outputFile =  "./output/" + id + "/manifest.mpd";
                    std::ofstream outfile(savePath, std::ofstream::binary);
                    outfile.write(content.data(), content.size());
                    outfile.close();

                   
                }
            }
        }
        int ret = this->encoder->encodeVideoWithCmd(inputFile, outputFile);
        
        uploadToStreamingServer("./output/" + id, id);

        if (ret == 0)
            res.write("File uploaded successfully");
        else
            res.write("Error uploading file");

        res.end(); });
}
 

void VideoEncodingRoutes::uploadToStreamingServer(std::string path, std::string id)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 1024 * 1024 * 1024L);

    if (!curl)
    {
        std::cerr << "Failed to initialize curl" << std::endl;
        return; 
    }
    std::string uri = "http://127.0.0.1:8888/uploadfile?id=" + id;
    const char *url = uri.c_str();
    curl_easy_setopt(curl, CURLOPT_URL, url);

    struct dirent *entry;
    struct stat fileInfo;

    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
    {
        std::cout << "Could not open directory " << path << std::endl;
        curl_easy_cleanup(curl);
        return;
    }

    std::vector<std::string> files;

    while ((entry = readdir(dir)))
    {
        std::string filePath = path + "/" + entry->d_name;

        if (stat(filePath.c_str(), &fileInfo) == 0 && S_ISREG(fileInfo.st_mode))
        {
            files.push_back(filePath);
        }
    }

    closedir(dir);

    for (const auto &file : files)
    {
        curl_httppost *post = NULL;
        curl_httppost *last = NULL;

        CURLFORMcode resForm = curl_formadd(&post, &last, CURLFORM_COPYNAME, "file",
                                            CURLFORM_FILE, file.c_str(), CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        CURLcode res = curl_easy_perform(curl);

        curl_formfree(post);

 
    }
    curl_easy_cleanup(curl);
}
