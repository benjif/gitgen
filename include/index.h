#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include <string>

class IndexHtmlGen {
public:
    IndexHtmlGen(std::vector<std::string> &repo_paths);
    ~IndexHtmlGen();

    void generate();

private:
    IndexHtmlGen(IndexHtmlGen &&) = delete;
    IndexHtmlGen(const IndexHtmlGen &) = delete;
};

#endif
