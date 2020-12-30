#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <string>

enum class MarkdownResult {
    SUCCESS,
    FAILURE,
};

MarkdownResult render_markdown(const char *raw, size_t raw_size, std::string &html);

#endif
