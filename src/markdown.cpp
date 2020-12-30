#include <fstream>
#include <sstream>
#include <md4c-html.h>
#include "markdown.h"

MarkdownResult render_markdown(const char *raw, size_t raw_size, std::string &html)
{
    auto process_out = +[](const MD_CHAR *text, MD_SIZE size, void *user) {
        std::string *out = (std::string *)user;
        out->append(text, size);
    };

    int ret = md_html(raw, raw_size, process_out, (void *)(&html),
            MD_FLAG_NOHTML, MD_HTML_FLAG_SKIP_UTF8_BOM);
    if (ret != 0)
        return MarkdownResult::FAILURE;

    return MarkdownResult::SUCCESS;
}
