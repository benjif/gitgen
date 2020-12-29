#ifndef EXTRA_H
#define EXTRA_H

#include <utility>
#include <string>
#include <git2.h>
#include <git2/global.h>

#define GiB 0x40000000
#define MiB 0x100000
#define KiB 0x400

static const char *default_description =
    "Unnamed repository; edit this file 'description' to name the repository.\n";

inline std::string escape_string(const std::string &str)
{
    std::string out;
    out.reserve(str.length());

    for (size_t i = 0; i < str.size(); i++) {
        switch (str[i]) {
        case '&':   out.append("&amp;");    break;
        case '<':   out.append("&lt;");     break;
        case '>':   out.append("&gt;");     break;
        case '"':   out.append("&quot;");   break;
        case '\'':  out.append("&#39;");    break;
        default:    out.append(&str[i], 1); break;
        }
    }

    return out;
}

template <typename T>
inline std::string to_string_precision(const T value, size_t precision)
{
    std::ostringstream ret;
    ret.precision(precision);
    ret << std::fixed << value;
    return ret.str();
}

inline std::pair<std::string, std::string> format_filesize(size_t rawsize)
{
    if (rawsize >= GiB)
        return std::make_pair(to_string_precision((double)rawsize / GiB, 2), "GiB");
    else if (rawsize >= MiB)
        return std::make_pair(to_string_precision((double)rawsize / MiB, 2), "MiB");
    else if (rawsize >= KiB)
        return std::make_pair(to_string_precision((double)rawsize / KiB, 2), "KiB");

    return std::make_pair(std::to_string(rawsize), "B");
}

inline char lowercase(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - ('Z' - 'z');
    return c;
}

inline void to_lowercase(std::string &str)
{
    std::transform(str.begin(), str.end(),
            str.begin(), lowercase);
}

inline std::string to_string(git_time_t time)
{
    std::stringstream sstream;
    sstream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M");
    return sstream.str();
}

#endif
