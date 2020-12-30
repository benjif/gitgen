#include <fstream>
#include <sstream>
#include <md4c-html.h>
#include "markdown.h"

MarkdownResult render_markdown(const std::string &file_path, std::string &html)
{
    std::ifstream in_stream(file_path, std::ios::in);
    if (!in_stream.is_open())
        return MarkdownResult::FAILURE;

    std::ostringstream ss;
    ss << in_stream.rdbuf();

    auto process_out = +[](const MD_CHAR *text, MD_SIZE size, void *user) {
        std::string *out = (std::string *)user;
        out->append(text, size);
    };

    int ret = md_html(ss.str().data(), ss.str().size(), process_out, (void *)(&html),
            MD_FLAG_NOHTML, MD_HTML_FLAG_SKIP_UTF8_BOM);
    if (ret != 0)
        return MarkdownResult::FAILURE;

    return MarkdownResult::SUCCESS;
}
