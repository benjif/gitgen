#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
#include <filesystem>

class IndexHtmlGen {
public:
    struct Options {
        std::vector<std::string> repo_paths;
    };

    IndexHtmlGen(const Options &opt);
    ~IndexHtmlGen();

    void generate();

private:
    Options m_options;

    void cleanup();
    void error(const char *msg);

    struct RepoMeta {
        std::string path;
        std::string name;
        std::string description;
        git_repository *repo;
    };

    std::vector<RepoMeta> m_repos;

    int m_err { 0 };

    IndexHtmlGen(IndexHtmlGen &&) = delete;
    IndexHtmlGen(const IndexHtmlGen &) = delete;
};

#endif
