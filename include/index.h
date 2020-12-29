#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include <filesystem>

class IndexHtmlGen {
public:
    struct Options {
    };

    IndexHtmlGen(const Options &opt);
    ~IndexHtmlGen();

    void generate();

private:
    Options m_options;

    IndexHtmlGen(IndexHtmlGen &&) = delete;
    IndexHtmlGen(const IndexHtmlGen &) = delete;

    std::vector<std::filesystem::path> m_repo_paths;
};

#endif
