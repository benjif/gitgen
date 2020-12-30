#include <fstream>
#include <iostream>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <git2.h>
#include <git2/global.h>
#include "extra.h"
#include "index.h"
#include "templates.h"

namespace fs = std::filesystem;

void IndexHtmlGen::cleanup()
{
    for (auto repo_meta : m_repos)
        git_repository_free(repo_meta.repo);
    git_libgit2_shutdown();
}

void IndexHtmlGen::error(const char *msg)
{
    fmt::print(stderr, "Error occurred (code: {}): {}\n", m_err, msg);
    cleanup();
    exit(1);
}

IndexHtmlGen::~IndexHtmlGen()
{
    cleanup();
}

IndexHtmlGen::IndexHtmlGen(const Options &opt)
    : m_options(opt)
{
    if ((m_err = git_libgit2_init()) < 0)
        error("failed to initialize libgit2");

    for (auto &repo_path : opt.repo_paths) {
        if (!fs::exists(repo_path))
            error("repo path does not exist");

        git_repository *repo;
        if ((m_err = git_repository_open(&repo, repo_path.c_str())) < 0)
            error("failed to open repository");

        RepoMeta meta;
        meta.repo = repo;
        meta.path = fs::absolute(repo_path);
        if (meta.path.back() == '.' && meta.path.length() > 2 && *(meta.path.end() - 2) == '/')
            meta.path.pop_back();
        if (meta.path.back() == '/')
            meta.path.pop_back();
 
        size_t path_split_pos = meta.path.find_last_of('/');
        if (path_split_pos == std::string::npos)
            meta.name = meta.path;
        else
            meta.name = meta.path.substr(path_split_pos + 1);

        to_lowercase(meta.name);
        escape_string(meta.name);

        if (fs::exists(meta.path + "/description")) {
            std::ifstream in_stream(meta.path + "/description");
            std::ostringstream in_sstream;
            in_sstream << in_stream.rdbuf();
            meta.description = escape_string(in_sstream.str());
        } else if (fs::exists(meta.path + "/.git/description")) {
            std::ifstream in_stream(meta.path + "/.git/description");
            std::ostringstream in_sstream;
            in_sstream << in_stream.rdbuf();
            if (in_sstream.str() != default_description)
                meta.description = escape_string(in_sstream.str());
        }

        m_repos.push_back(meta);
    }
}

static const size_t REPO_NAME_EST = 16;
static const size_t REPO_DESC_EST = 64;

void IndexHtmlGen::generate()
{
    std::string repos_html;
    repos_html.reserve((REPO_DESC_EST + REPO_NAME_EST + sizeof(index_line_template)) * m_repos.size());

    for (auto &repo_info : m_repos) {
        git_commit *head;
        git_oid oid_head;
 
        if ((m_err = git_reference_name_to_id(&oid_head, repo_info.repo, "HEAD")) < 0)
            error("failed to retrieve HEAD commit");
        if ((m_err = git_commit_lookup(&head, repo_info.repo, &oid_head)) < 0)
            error("failed to retrieve HEAD commit");

        repos_html += fmt::format(
            index_line_template,
            fmt::arg("name", repo_info.name),
            fmt::arg("desc", repo_info.description),
            fmt::arg("updated", to_string(git_commit_time(head)))
        );

        git_commit_free(head);
    }

    std::ofstream out_stream("public/index.html", std::ios::out);
    if (!out_stream.is_open())
        error("failed to open output file.");

    out_stream << fmt::format(
        index_page_template,
        fmt::arg("repos_content", repos_html)
    );
}
