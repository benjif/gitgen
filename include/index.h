#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include <filesystem>

class IndexHtmlGen {
public:
    IndexHtmlGen();
    ~IndexHtmlGen();

    void generate();

private:
    IndexHtmlGen(IndexHtmlGen &&) = delete;
    IndexHtmlGen(const IndexHtmlGen &) = delete;

    std::vector<std::filesystem::path> m_repo_paths;
};

#endif
