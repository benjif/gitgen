#include <ctime>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <utility>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <fmt/core.h>
#include <fmt/format.h>
#include "html.h"
#include "repo.h"
#include "templates.h"

namespace fs = std::filesystem;

void RepoHtmlGen::cleanup()
{
    if (m_repo)
        git_repository_free(m_repo);
    if (m_index)
        git_index_free(m_index);
    if (m_tree)
        git_tree_free(m_tree);
    if (m_head_commit)
        git_commit_free(m_head_commit);
    git_libgit2_shutdown();
}

void RepoHtmlGen::error(const char *msg)
{
    fmt::print(stderr, "Error occurred (code: {}): {}\n", m_err, msg);
    cleanup();
    exit(1);
}

git_commit *RepoHtmlGen::last_commit()
{
    git_commit *commit = nullptr;
    git_oid oid_commit;

    if ((m_err = git_reference_name_to_id(&oid_commit, m_repo, "HEAD")) < 0)
        error("failed to retrieve HEAD commit");
    if ((m_err = git_commit_lookup(&commit, m_repo, &oid_commit)) < 0)
        error("failed to retrieve HEAD commit");

    return commit;
}

RepoHtmlGen::RepoHtmlGen(const std::string &repo_path)
    : m_repo_path(fs::absolute(repo_path))
{
    if ((m_err = git_libgit2_init()) < 0)
        error("failed to initialize libgit2");
    if ((m_err = git_repository_open(&m_repo, repo_path.c_str())) < 0)
        error("failed to open repository");
    if ((m_err = git_repository_index(&m_index, m_repo)) < 0)
        error("failed to retrieve repository index");

    m_head_commit = last_commit();
    if ((m_err = git_commit_tree(&m_tree, m_head_commit)) < 0)
        error("failed to retrieve tree for HEAD commit");

    if (m_repo_path.back() == '.' && m_repo_path.length() > 2 && *(m_repo_path.end() - 2) == '/')
        m_repo_path.pop_back();
    if (m_repo_path.back() == '/')
        m_repo_path.pop_back();

    size_t path_split_pos = m_repo_path.find_last_of('/');
    if (path_split_pos == std::string::npos)
        m_repo_name = m_repo_path;
    else
        m_repo_name = m_repo_path.substr(path_split_pos + 1);

    m_header_content = fmt::format(
        header_template,
        fmt::arg("reponame", m_repo_name),
        fmt::arg("repodesc", ""),
        fmt::arg("filespath", '/' + m_repo_name + "/index.html"),
        fmt::arg("commitspath", '/' + m_repo_name + "/commits.html")
    );

    if (fs::exists("public/" + m_repo_name))
        fs::remove_all("public/" + m_repo_name);
}

RepoHtmlGen::~RepoHtmlGen()
{
    cleanup();
}

void RepoHtmlGen::generate()
{
    generate_files();
    generate_index(m_tree);
    generate_commits();
}

void RepoHtmlGen::generate_file_page_code(const std::string &file_path, std::string &html)
{
    std::ifstream in_stream(file_path, std::ios::in);

    // TODO: optimize this
    std::string line;
    while (std::getline(in_stream, line)) {
        html += fmt::format(
            file_line_template,
            fmt::arg("line", escape_string(line))
        );
    }
}

void RepoHtmlGen::generate_file_page(const git_index_entry *entry)
{
    const std::string file_path = m_repo_path + '/' + entry->path;

    fs::path html_path = "public/" + m_repo_name + "/files/" + std::string(entry->path) + ".html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    std::string html_file_content;
    generate_file_page_code(file_path, html_file_content);

    std::ofstream out_stream("public/" + m_repo_name + "/files/" + std::string(entry->path) + ".html",
            std::ios::out);
    out_stream << fmt::format(
        file_page_template,
        fmt::arg("headercontent", m_header_content),
        fmt::arg("reponame", m_repo_name),
        fmt::arg("filename", entry->path),
        fmt::arg("fileviewcontent",
            fmt::format(
                file_view_template,
                fmt::arg("filename", entry->path),
                fmt::arg("filecontent", html_file_content)
            )
        )
    );

}

static char lowercase(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - ('Z' - 'z');
    return c;
}

static bool is_readme(std::string &filename)
{
    std::transform(filename.begin(), filename.end(),
            filename.begin(), lowercase);
    if (filename == "readme.md" || filename == "readme.txt")
        return true;
    return false;
}

void RepoHtmlGen::generate_files()
{
    size_t file_count = git_index_entrycount(m_index);
    for (size_t i = 0; i < file_count; i++) {
        auto *entry = git_index_get_byindex(m_index, i);
        generate_file_page(entry);

        std::string filename = std::string(entry->path);
        if (is_readme(filename))
           m_readme = entry; 
    }
}

/*
git_commit *RepoHtmlGen::file_last_modified(git_object *obj)
{
    git_revwalk *walk = nullptr;
    if ((m_err = git_revwalk_new(&walk, m_repo)) < 0)
        error("Failed to create git revwalk");

    const git_oid *obj_oid = git_object_id(obj);
    git_oid oid;

    git_revwalk_push_head(walk);
    git_revwalk_sorting(walk, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    //git_revwalk_simplify_first_parent(walk);

    while (git_revwalk_next(&oid, walk) == 0) {
        if (git_oid_cmp(obj_oid, &oid) == 0)
            break;
    }

    git_commit *ret = nullptr;
    if ((m_err = git_commit_lookup(&ret, m_repo, &oid)) < 0)
        error("failed to lookup commit");
    git_revwalk_free(walk);

    return ret;
}
*/

template <typename T>
std::string to_string_precision(const T value, size_t precision)
{
    std::ostringstream ret;
    ret.precision(precision);
    ret << std::fixed << value;
    return ret.str();
}

#define GiB 0x40000000
#define MiB 0x100000
#define KiB 0x400

std::pair<std::string, std::string> format_filesize(size_t rawsize)
{
    if (rawsize >= GiB)
        return std::make_pair(to_string_precision((double)rawsize / GiB, 2), "GiB");
    else if (rawsize >= MiB)
        return std::make_pair(to_string_precision((double)rawsize / MiB, 2), "MiB");
    else if (rawsize >= KiB)
        return std::make_pair(to_string_precision((double)rawsize / KiB, 2), "KiB");

    return std::make_pair(std::to_string(rawsize), "B");
}

void RepoHtmlGen::generate_index(git_tree *tree, std::string root)
{
    fs::path html_path =
        root == "" ? "public/" + m_repo_name + "/index.html"
                   : "public/" + m_repo_name + "/tree/" + root + "index.html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    std::ofstream out_stream(html_path, std::ios::out);
    std::string tree_html;

    const git_tree_entry *entry = nullptr;
    const char *entry_name;

    git_object *obj = nullptr;

    size_t tree_entry_count = git_tree_entrycount(tree);
    for (size_t i = 0; i < tree_entry_count; i++) {
        if (!(entry = git_tree_entry_byindex(tree, i)))
            error("failed to retrieve tree entry");
        if (!(entry_name = git_tree_entry_name(entry)))
            error("failed to retrieve tree entry name");
        if ((m_err = git_tree_entry_to_object(&obj, m_repo, entry)) < 0)
            error("failed to convert tree entry to object");

        switch (git_tree_entry_type(entry)) {
        case GIT_OBJ_BLOB:
            break;
        case GIT_OBJ_TREE:
            generate_index((git_tree *)obj, root + entry_name + '/');
            tree_html += fmt::format(
                file_tree_line_dir_template,
                fmt::arg("file_tree_name", entry_name),
                fmt::arg("file_tree_link", '/' + m_repo_name + "/tree/" + root + entry_name)
            );
            git_object_free(obj);
            continue;
        default:
            break;
        }

        auto size_info = format_filesize(git_blob_rawsize((git_blob *)obj));
        tree_html += fmt::format(
            file_tree_line_template,
            fmt::arg("file_tree_name", entry_name),
            fmt::arg("file_tree_size", size_info.first),
            fmt::arg("file_tree_size_unit", size_info.second),
            fmt::arg("file_tree_link", '/' + m_repo_name + "/files/" + root + entry_name + ".html")
        );

        git_object_free(obj);
    }

    std::string readme_code_html;
    if (m_readme != nullptr)
        generate_file_page_code(m_repo_path + '/' + m_readme->path, readme_code_html);

    out_stream << fmt::format(
        file_index_template,
        fmt::arg("headercontent", m_header_content),
        fmt::arg("reponame", m_repo_name),
        fmt::arg("repodesc", ""), // TODO
        fmt::arg("treecontent", tree_html),
        fmt::arg("readmecontent",
            m_readme == nullptr ? "" :
                 fmt::format(
                     file_view_template,
                     fmt::arg("filename", m_readme->path),
                     fmt::arg("filecontent", readme_code_html)
                 )
        )
    );
}

//const char *commits_line_template =
    //"<tr><td>{date}</td><td>{message}</td><td>{author}</td><td>{files}</td><td>{gain}</td><td>{loss}</td></tr>";

RepoHtmlGen::CommitInfo::~CommitInfo()
{
    git_commit_free(commit);
}

std::string to_string(git_time_t time)
{
    std::stringstream sstream;
    sstream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M");
    return sstream.str();
}

void RepoHtmlGen::generate_commit_page(const CommitInfo &info)
{
    std::ofstream out_stream("public/" + m_repo_name + "/commits/" + info.id_str + ".html", std::ios::out);

    out_stream << fmt::format(
        commit_page_template,
        fmt::arg("reponame", m_repo_name),
        fmt::arg("headercontent", m_header_content),
        fmt::arg("date", to_string(info.time)),
        fmt::arg("message", info.summary),
        fmt::arg("author", info.author->name),
        fmt::arg("email", info.author->email)
    );
}

void RepoHtmlGen::generate_commits()
{
    fs::path commits_dir = "public/" + m_repo_name + "/commits/";
    if (!fs::exists(commits_dir))
        fs::create_directories(commits_dir);

    git_revwalk *walk = nullptr;
    git_oid oid;

    if ((m_err = git_revwalk_new(&walk, m_repo)) < 0)
        error("Failed to create git revwalk");

    git_revwalk_push_head(walk);
    git_revwalk_sorting(walk, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    //git_revwalk_simplify_first_parent(walk);

    std::string commits_html;
    while (git_revwalk_next(&oid, walk) == 0) {
        git_commit *commit = nullptr;
        if ((m_err = git_commit_lookup(&commit, m_repo, &oid)) < 0)
            error("failed to lookup commit");

        CommitInfo commit_info = {
            commit,
            git_commit_time(commit),
            git_commit_summary(commit),
            git_commit_message(commit),
            git_commit_author(commit),
            git_commit_committer(commit),
            "",
            0,
            0,
            0,
        };

        git_oid_fmt(commit_info.id_str, git_commit_id(commit));
        generate_commit_page(commit_info);

        std::cout << commit_info.summary << '\n';

        commits_html += fmt::format(
            commits_line_template,
            fmt::arg("date", to_string(commit_info.time)),
            fmt::arg("message", escape_string(commit_info.summary)),
            fmt::arg("author", escape_string(commit_info.author->name)),
            fmt::arg("files", ""),
            fmt::arg("gain", ""),
            fmt::arg("loss", ""),
            fmt::arg("commit_link", '/' + m_repo_name + "/commits/" + commit_info.id_str + ".html")
        );
    }

    git_revwalk_free(walk);

    std::ofstream out_stream("public/" + m_repo_name + "/commits.html", std::ios::out);
    out_stream << fmt::format(
        commits_page_template,
        fmt::arg("reponame", m_repo_name),
        fmt::arg("headercontent", m_header_content),
        fmt::arg("commitscontent", commits_html)
    );
}
