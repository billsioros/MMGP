
#include <log.hpp>
#include <fstream>          // std::ofstream
#include <string>           // std::string
#include <iostream>         // std::cerr
#include <unordered_map>    // std::unordered_map
#include <chrono>           // std::chrono

std::string Log::timestamp(const std::string& format)
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
    (
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    auto sec = static_cast<std::time_t>(ms / 1000);

    std::tm time_info;
    localtime_r(&sec, &time_info);

    char buff[512];
    snprintf(buff, sizeof(buff), format.c_str(),
        1900 + time_info.tm_year, 1 + time_info.tm_mon, time_info.tm_mday,
        time_info.tm_hour, time_info.tm_min, time_info.tm_sec, ms % 1000);

    return std::string(buff);
}

Log::Log(const std::string& name)
:
info({ name + timestamp("%04d%02d%02d%02d%02d%02d%03lld") + ".log", false })
{
    try
    {
        ofs.open(this->info.file, std::ios::trunc);
    }
    catch (std::exception& e)
    {
        std::cerr << "FatalLoggerError: " << e.what() << std::endl;

        throw;
    }
}

Log::~Log()
{
    if (!info.dirty)
    {
        ofs.close(); std::remove(info.file.c_str());
    }
}

void Log::operator()(Code code, const std::string& message)
{
    const static std::unordered_map<Log::Code, std::string> prefixes =
    {
        { Log::Code::Message, "<MSG>" },
        { Log::Code::Warning, "<WRN>" },
        { Log::Code::Error,   "<ERR>" }
    };

    try
    {
        ofs
        << "[ " << timestamp("%04d-%02d-%02d %02d:%02d:%02d.%03lld") << " ]"
        << " : " << prefixes.at(code) << " : " << message << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "FatalLoggerError: " << e.what() << std::endl;

        throw;
    }

    info.dirty |= (code != Log::Code::Message);
}
