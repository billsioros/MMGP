
#pragma once

#include <fstream>  // std::ofstream
#include <string>   // std::string

class Log
{
    struct Info
    {
        const std::string file;
        bool dirty;
    } info;

    std::ofstream ofs;

public:

    static std::string timestamp(const std::string&);

    enum Code { Message, Warning, Error };

    Log(const std::string&);
    ~Log();

    void operator()(Code, const std::string&);
};
