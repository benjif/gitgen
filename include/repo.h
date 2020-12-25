#ifndef REPO_H
#define REPO_H

#include <string>
#include <git2.h>
#include <git2/global.h>

class RepoHtmlGen {
public:
    RepoHtmlGen(const std::string &repo_path);
    ~RepoHtmlGen();

    void generate();

private:
    RepoHtmlGen(RepoHtmlGen &&) = delete;
    RepoHtmlGen(const RepoHtmlGen &) = delete;

    void cleanup();
    void error(const char *msg);

    git_commit *last_commit();
    //git_commit *file_last_modified(git_object *obj);

    void render_header_content();

    void generate_file_page_code(const std::string &file_path, std::string &html);
    void generate_file_page(const git_index_entry *entry);
    void generate_files();
    void generate_index(git_tree *tree, std::string root = "");
    void generate_commits();

    std::string m_repo_path;
    std::string m_repo_name;

    std::string m_header_content;

    int m_err { 0 };
    git_repository *m_repo { nullptr };
    git_index *m_index { nullptr };
    git_tree *m_tree { nullptr };

    const git_index_entry *m_readme { nullptr };
};

#endif
