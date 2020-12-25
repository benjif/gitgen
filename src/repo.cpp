#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <fmt/core.h>
#include <fmt/format.h>
#include "html.h"
#include "repo.h"
#include "templates.h"

namespace fs = std::filesystem;

static const std::string css_path_string = "public/css/style.css";

void RepoHtmlGen::cleanup()
{
    if (m_repo)
        git_repository_free(m_repo);
    if (m_index)
        git_index_free(m_index);
    if (m_tree)
        git_tree_free(m_tree);
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
    : m_repo_path(fs::absolute(repo_path)),
      m_css_path(fs::absolute(css_path_string))
{
    if ((m_err = git_libgit2_init()) < 0)
        error("failed to initialize libgit2");
    if ((m_err = git_repository_open(&m_repo, repo_path.c_str())) < 0)
        error("failed to open repository");
    if ((m_err = git_repository_index(&m_index, m_repo)) < 0)
        error("failed to retrieve repository index");

    git_commit *head_commit = last_commit();
    if ((m_err = git_commit_tree(&m_tree, head_commit)) < 0)
        error("failed to retrieve tree for HEAD commit");

    if (m_repo_path.back() == '.' && m_repo_path.length() > 2 && *(m_repo_path.end() - 2) == '/')
        m_repo_path.pop_back();
    if (m_repo_path.back() == '/')
        m_repo_path.pop_back();

    fmt::print("repo path: {}\n", m_repo_path);

    size_t path_split_pos = m_repo_path.find_last_of('/');
    if (path_split_pos == std::string::npos)
        m_repo_name = m_repo_path;
    else
        m_repo_name = m_repo_path.substr(path_split_pos + 1);

    render_header_content();
}

RepoHtmlGen::~RepoHtmlGen()
{
    cleanup();
}

void RepoHtmlGen::render_header_content()
{
    m_header_content = fmt::format(
        header_template,
        fmt::arg("reponame", m_repo_name),
        fmt::arg("repodesc", ""),
        fmt::arg("filespath", "/index.html")
    );
}

void RepoHtmlGen::generate()
{
    generate_files();
    generate_index("", m_tree);
}

void RepoHtmlGen::generate_file_page(const git_index_entry *entry)
{
    const std::string file_path = m_repo_path + '/' + entry->path;
    std::ifstream in_stream(file_path, std::ios::in);

    fs::path html_path = "public/files/" + std::string(entry->path) + ".html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    // TODO: optimize this
    std::string line, html_file_content;
    while (std::getline(in_stream, line)) {
        escape_string(line);
        html_file_content += fmt::format(
            file_line_template,
            fmt::arg("line", line)
        );
    }

    std::string relative_css_path = fs::relative(m_css_path, html_path.parent_path());

    std::ofstream out_stream("public/files/" + std::string(entry->path) + ".html",
            std::ios::out);
    out_stream << fmt::format(
        file_page_template,
        fmt::arg("headercontent", m_header_content),
        fmt::arg("filecontent", html_file_content),
        fmt::arg("reponame", m_repo_name),
        fmt::arg("filename", entry->path),
        fmt::arg("csspath", relative_css_path)
    );

}

void RepoHtmlGen::generate_files()
{
    size_t file_count = git_index_entrycount(m_index);
    for (size_t i = 0; i < file_count; i++) {
        auto *entry = git_index_get_byindex(m_index, i);
        generate_file_page(entry);
    }
}

void RepoHtmlGen::generate_index(std::string root, git_tree *tree)
{
    fs::path html_path = "public/" + root + "index.html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    std::ofstream out_stream(html_path, std::ios::out);
    std::string relative_css_path = fs::relative(m_css_path, html_path.parent_path());
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
            if (root != "")
                root += '/';
            generate_index(root + entry_name + '/', (git_tree *)obj);
            tree_html += fmt::format(
                file_tree_line_dir,
                fmt::arg("file_tree_name", entry_name),
                fmt::arg("file_tree_link", entry_name)
            );
            git_object_free(obj);
            continue;
        default:
            break;
        }

        tree_html += fmt::format(
            file_tree_line,
            fmt::arg("file_tree_name", entry_name),
            fmt::arg("file_tree_size", git_blob_rawsize((git_blob *)obj)),
            fmt::arg("file_tree_link", "/files/" + root + entry_name + ".html")
        );

        git_object_free(obj);
    }

    out_stream << fmt::format(
        file_index_template,
        fmt::arg("headercontent", m_header_content),
        fmt::arg("reponame", m_repo_name),
        fmt::arg("repodesc", ""), // TODO
        fmt::arg("filespath", "/index.html"),
        fmt::arg("treecontent", tree_html),
        fmt::arg("csspath", relative_css_path)
    );
}
