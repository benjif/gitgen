#ifndef REPO_H
#define REPO_H

#include <string>
#include <vector>
#include <git2.h>
#include <git2/global.h>

#include "fmt/format.h"

class RepoHtmlGen {
public:
    static const size_t MAX_COMMIT_COUNT = 100;
    static const size_t MAX_DIFF_LINE_COUNT = 1024;
    static const size_t MAX_VIEW_FILESIZE = 0x400 * 256; // 256 KiB

    struct Options {
        std::string repo_path;
        size_t max_commits { MAX_COMMIT_COUNT };
        size_t max_diff_lines { MAX_DIFF_LINE_COUNT };
        size_t max_view_filesize { MAX_VIEW_FILESIZE };
    };

    RepoHtmlGen(const Options &opt);
    ~RepoHtmlGen();

    void generate();

private:
    Options m_options;

    RepoHtmlGen(RepoHtmlGen &&) = delete;
    RepoHtmlGen(const RepoHtmlGen &) = delete;

    void cleanup();
    void error(const char *msg);

    git_commit *last_commit();
    //git_commit *file_last_modified(git_object *obj);

    void generate_file_page_code(const std::string &file_path, std::string &html);
    void generate_file_page(const git_index_entry *entry);
    void generate_files();
    void generate_index(git_tree *tree, std::string root = "");

    struct Delta {
        git_patch *patch;
        size_t gain, loss;
    };

    struct CommitInfo {
        ~CommitInfo();

        git_commit *commit;
        git_commit *parent;
        git_diff *diff;
        git_tree *tree;
        git_tree *parent_tree;
        git_time_t time;
        const char *summary;
        const char *message;
        const git_oid *id;
        const git_oid *parent_id;
        const git_signature *author;
        const git_signature *committer;
        char id_str[GIT_OID_HEXSZ + 1];
        char parent_id_str[GIT_OID_HEXSZ + 1];
        size_t files = 0, hunks = 0, gain = 0, loss = 0;

        std::vector<Delta> deltas;
    };

    void get_commit_info(git_commit *commit, CommitInfo &info);
    void generate_commit_page(const CommitInfo &commit);
    void generate_commits();

    std::string m_repo_path;
    std::string m_repo_name;
    std::string m_header_content;
    std::string m_description;

    git_commit *m_head_commit { nullptr};
    git_repository *m_repo { nullptr };
    git_index *m_index { nullptr };
    git_tree *m_tree { nullptr };
    int m_err { 0 };

    std::string m_readme_content;
};

#endif
