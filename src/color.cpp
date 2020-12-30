#include <srchilite/sourcehighlight.h>
#include <srchilite/langmap.h>
#include "color.h"

#define SRCHILI_DIR "/usr/share/source-highlight"

void highlight(const std::string &filename, std::istream &in, std::ostream &out)
{
    srchilite::SourceHighlight src_hili("html_gitgen.outlang");
    src_hili.setCanUseStdOut(false);
    src_hili.setTabSpaces(4);
    src_hili.setDataDir(SRCHILI_DIR);

    srchilite::LangMap lang_map(SRCHILI_DIR, "lang.map");
    std::string lang = lang_map.getMappedFileNameFromFileName(filename);
    if (lang == "")
        lang = "nohilite.lang";

    src_hili.highlight(in, out, lang);
}
