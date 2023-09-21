#pragma once
#include <filesystem>
#include <iostream>
#include <string>

static inline bool createDirectory(const std::string &directoryPath)
{
    if (!std::filesystem::exists(directoryPath))
    {
        try
        {
            std::filesystem::create_directories(directoryPath);
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to create directory: " << ex.what() << std::endl;
            return false;
        }
    }

    return true;
}

static inline bool doesDirectoryExist(const std::string &directoryPath)
{
    return std::filesystem::exists(directoryPath);
}