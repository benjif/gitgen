#include "html.h"

std::string escape_string(const std::string &str)
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
