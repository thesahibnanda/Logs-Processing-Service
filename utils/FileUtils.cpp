#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <filesystem>

class FileUtils
{
private:
    static inline std::shared_mutex fileMutex;

public:
    static std::string readFile(const std::string &filePath)
    {
        std::shared_lock<std::shared_mutex> lock(fileMutex);
        std::ifstream fileStream(filePath);
        if (!fileStream)
        {
            throw std::runtime_error("Error: Unable to open file for reading: " + filePath);
        }

        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

    static void writeFile(const std::string &filePath, const std::string &content)
    {
        std::unique_lock<std::shared_mutex> lock(fileMutex);
        std::ofstream fileStream(filePath);
        if (!fileStream)
        {
            throw std::runtime_error("Error: Unable to open file for writing: " + filePath);
        }

        fileStream << content;
    }

    static void createFileIfNotExists(const std::string &filePath)
    {
        if (std::filesystem::exists(filePath))
        {
            return;
        }

        std::unique_lock<std::shared_mutex> uniqueLock(fileMutex);
        std::ofstream newFile(filePath);
        if (!newFile)
        {
            throw std::runtime_error("Error: Unable to create file: " + filePath);
        }
    }

    static void deleteFile(const std::string &filePath)
    {
        std::unique_lock<std::shared_mutex> lock(fileMutex);

        if (!std::filesystem::exists(filePath))
        {
            throw std::runtime_error("Error: File does not exist: " + filePath);
        }

        if (!std::filesystem::remove(filePath))
        {
            throw std::runtime_error("Error: Unable to delete file: " + filePath);
        }
    }

    static void makeDirectoryIfNotExists(const std::string &directoryPath)
    {
        std::unique_lock<std::shared_mutex> uniqueLock(fileMutex);

        if (std::filesystem::exists(directoryPath)) {
            return;
        }

        try {
            if (!std::filesystem::create_directories(directoryPath)) {
                throw std::runtime_error("Error: Unable to create directory: " + directoryPath);
            }
        }
        catch (const std::exception &e) {
            throw std::runtime_error("Exception while creating directory: " + directoryPath + " - " + e.what());
        }
    }
};
