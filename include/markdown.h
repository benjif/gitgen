#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <string>

enum class MarkdownResult {
    SUCCESS,
    FAILURE,
};

MarkdownResult render_markdown(const std::string &file_path, std::string &html);

#endif
