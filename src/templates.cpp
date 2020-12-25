#include "templates.h"

const char *header_template =
#include "templates/header.inc"
;

const char *file_page_template = 
#include "templates/file.inc"
;

const char *file_view_template =
#include "templates/file_view.inc"
;

const char *file_line_template =
    "<li><pre>{line}</pre></li>";

const char *file_index_template =
#include "templates/file_index.inc"
;

const char *index_page_template =
#include "templates/index.inc"
;

const char *file_tree_line_template =
    "<tr><td class=\"filename\"><a href=\"{file_tree_link}\">{file_tree_name}</a>"
    "</td><td class=\"filesize\">{file_tree_size}</td></tr>";

const char *file_tree_line_dir_template =
    "<tr><td class=\"filename\"><a href=\"{file_tree_link}\">{file_tree_name}</a>"
    "</td><td class=\"filesize\"></td></tr>";

const char *commits_page_template =
#include "templates/commits.inc"
;

const char *commits_line_template =
    "<tr><td>{date}</td><td>{message}</td><td>{author}</td><td>{files}</td><td>{gain}</td><td>{loss}</td></tr>";
