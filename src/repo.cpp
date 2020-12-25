#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "templates.h"

void RepoHtmlGen::cleanup()
{
    if (m_repo)
        git_repository_free(m_repo);
    if (m_index)
        git_index_free(m_index);
    git_libgit2_shutdown();
}

void RepoHtmlGen::error(const char *msg)
{
    fmt::print(stderr, "Error occurred (code: {}): {}\n", m_err, msg);
    cleanup();
    exit(1);
}

RepoHtmlGen::RepoHtmlGen(const std::string &repo_path)
    : m_repo_path(repo_path)
{
    if ((m_err = git_libgit2_init()) < 0)
        error("failed to initialize libgit2");
    if ((m_err = git_repository_open(&m_repo, repo_path.c_str())) < 0)
        error("failed to open repository");
    if ((m_err = git_repository_index(&m_index, m_repo)) < 0)
        error("failed to retrieve repository index");

    if (m_repo_path.back() == '/')
        m_repo_path.pop_back();

    size_t path_split_pos = m_repo_path.find_last_of('/');
    if (path_split_pos == std::string::npos)
        m_repo_name = m_repo_path;
    else
        m_repo_name = m_repo_path.substr(path_split_pos + 1);
}

RepoHtmlGen::~RepoHtmlGen()
{
    cleanup();
}

void RepoHtmlGen::generate()
{
    generate_files();
    generate_readme();
}

void RepoHtmlGen::generate_file_page(const git_index_entry *entry)
{
    const std::string file_path = m_repo_path + '/' + entry->path;
    std::ifstream in_stream(file_path, std::ios::in);
    std::stringstream buffer;
    buffer << in_stream.rdbuf();

    std::cout << buffer.str() << '\n';
    std::string formatted = fmt::format(file_page_template, buffer.str());

    if (!std::filesystem::exists("public/files"))
        std::filesystem::create_directory("public/files");
    std::ofstream out_stream("public/files/" + std::string(entry->path) + ".html",
            std::ios::out);
    out_stream << formatted;
}

void RepoHtmlGen::generate_files()
{
    size_t file_count = git_index_entrycount(m_index);
    for (size_t i = 0; i < file_count; i++) {
        auto *entry = git_index_get_byindex(m_index, i);
        generate_file_page(entry);
    }
}

void RepoHtmlGen::generate_readme()
{
}
