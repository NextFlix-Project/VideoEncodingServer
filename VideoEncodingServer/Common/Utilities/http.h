#pragma once
#include <string>
#include <algorithm>

static inline bool isMultipartFormData(const std::string contentType)
{
    std::string lowerContentType = contentType;
    std::transform(lowerContentType.begin(), lowerContentType.end(), lowerContentType.begin(), ::tolower);

    return lowerContentType.find("multipart/form-data") != std::string::npos;
}

static inline std::string getBoundaryFromHeader(std::string header, size_t &boundaryPos)
{
    std::string boundary = header;

    boundaryPos = boundary.find("boundary=");

    if (boundaryPos != std::string::npos)
    {
        boundary = header.substr(boundaryPos + 9);

        size_t startPos = boundary.find_first_not_of(" \t");
        size_t endPos = boundary.find_last_not_of(" \t");
        boundary = boundary.substr(startPos, endPos - startPos + 1);
    }

    return boundary;
}