
#include <string>       // std::string
#include <utility>      // std::forward
#include <algorithm>    // std::count
#include <sstream>      // std::stringstream
#include <regex>        // std::regex_replace

namespace utility
{
    namespace detail
    {
        template <typename Arg>
        std::string format(const std::string& str, Arg&& arg)
        {
            std::stringstream ss; ss << std::forward<Arg>(arg);

            return std::regex_replace
            (
                str,
                std::regex(R"(\?)"),
                ss.str(),
                std::regex_constants::format_first_only
            );
        }

        template <typename Arg, typename ...Args>
        std::string format(const std::string& str, Arg&& arg, Args&&... args)
        {
            return format(format(str, std::forward<Arg>(arg)), std::forward<Args>(args)...);
        }
    }

    template <typename ...Args>
    std::string format(const std::string& str, Args&&... args)
    {
        constexpr std::size_t count = sizeof...(args);
        if (std::count(str.begin(), str.end(), '?') != count)
            throw std::out_of_range("placeholder count is not equal to arguement count");

        return detail::format(str, std::forward<Args>(args)...);
    }
}
