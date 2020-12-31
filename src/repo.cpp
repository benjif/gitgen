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
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "extra.h"
#include "templates.h"

#ifdef HIGHLIGHT
#include "color.h"
#endif

#ifdef MARKDOWN
#include "markdown.h"
#endif

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

RepoHtmlGen::~RepoHtmlGen()
{
    cleanup();
}

void RepoHtmlGen::error(const char *msg)
{
    fmt::print(stderr, "Error occurred (code: {}): {}\n", m_err, msg);
    cleanup();
    exit(1);
}

RepoHtmlGen::RepoHtmlGen(const Options &opt)
    : m_options(opt),
      m_repo_path(fs::absolute(opt.repo_path))
{
    if ((m_err = git_libgit2_init()) < 0)
        error("failed to initialize libgit2");
    if ((m_err = git_repository_open_ext(&m_repo, m_repo_path.c_str(),
                GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr)) != 0)
        error("failed to open repository");
    if ((m_err = git_repository_index(&m_index, m_repo)) < 0)
        error("failed to retrieve repository index");

    m_head = head();
    if (!m_head)
        error("failed to retrieve HEAD object id");
    if ((m_err = git_commit_lookup(&m_head_commit, m_repo, m_head)) < 0)
        error("failed to lookup HEAD commit");
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

    if (m_repo_name.ends_with(".git"))
        m_repo_name.resize(m_repo_name.size() - 4);

    to_lowercase(m_repo_name);

    std::ifstream in_stream;
    std::ostringstream ss;
    if (fs::exists(m_repo_path + "/description"))
        in_stream.open(m_repo_path + "/description");
    else if (fs::exists(m_repo_path + "/.git/description"))
        in_stream.open(m_repo_path + "/.git/description");

    ss << in_stream.rdbuf();
    m_description = escape_string(ss.str());

    m_header_content = fmt::format(
        header_template,
        fmt::arg("repo_name", m_repo_name),
        fmt::arg("repo_desc", m_description),
        fmt::arg("files_path", '/' + m_repo_name + "/index.html"),
        fmt::arg("commits_path", '/' + m_repo_name + "/commits.html")
    );

    if (fs::exists("public/" + m_repo_name))
        fs::remove_all("public/" + m_repo_name);
}

const git_oid *RepoHtmlGen::head() const
{
    git_object *head_obj = nullptr;
    const git_oid *head_oid = nullptr;
    if (!git_revparse_single(&head_obj, m_repo, "HEAD"))
        head_oid = git_object_id(head_obj);

    git_object_free(head_obj);
    return head_oid;
}

static const std::string README_FILENAMES[] = {
    "README.md",
    "readme.md",
    "README.txt",
    "readme.txt",
    "README",
};

void RepoHtmlGen::find_readme()
{
    git_object *readme_obj;
    for (auto &readme_filename : README_FILENAMES) {
        if (!git_revparse_single(&readme_obj, m_repo, ("HEAD:" + readme_filename).c_str())) {
#ifndef MARKDOWN
            std::string readme_content;
            generate_file_code_page(readme_filename, (git_blob *)readme_obj, readme_content);
            m_readme_content = fmt::format(
                 file_view_template,
                 fmt::arg("filename", readme_filename),
                 fmt::arg("file_content", readme_content),
                 fmt::arg("file_size", ""),
                 fmt::arg("file_size_unit", "")
            );
#else
            const char *raw_readme_content =
                (const char *)git_blob_rawcontent((git_blob *)readme_obj);
            size_t raw_readme_size = git_blob_rawsize((git_blob *)readme_obj);

            m_readme_content.reserve(raw_readme_size);
            m_readme_content.append(markdown_pre);

            auto result = render_markdown(raw_readme_content, raw_readme_size, m_readme_content);
            if (result == MarkdownResult::FAILURE)
                error("failed to render markdown");

            m_readme_content.append(markdown_post);
#endif
            git_object_free(readme_obj);
            break;
        }
    }
}

void RepoHtmlGen::generate()
{
    find_readme();
    generate_tree_pages(m_tree);
    generate_commit_pages();
}

struct SingleUseBuf : public std::streambuf {
    SingleUseBuf(char *start, size_t len)
    {
        setg(start, start, start + len - 1);
    }
};

static const size_t LINE_SIZE_EST = 50;

void RepoHtmlGen::generate_file_code_page(const std::string &filename, git_blob *blob, std::string &html)
{
    char *raw_content = (char *)git_blob_rawcontent(blob);
    size_t filesize = git_blob_rawsize(blob);
    if (filesize >= m_options.max_view_filesize) {
        html += "File is too large to view.";
        return;
    }

    SingleUseBuf buf(raw_content, filesize);
    std::istream in_stream(&buf);
#ifndef HIGHLIGHT
    html.reserve(filesize + (filesize / LINE_SIZE_EST) * sizeof(file_line_template));

    std::string line;
    size_t line_no = 0;
    while (std::getline(in_stream, line)) {
        line_no++;
        html += fmt::format(
            file_line_template,
            fmt::arg("line", escape_string(line))
        );
    }
#else
    std::stringbuf obuf;
    std::ostream out_stream(&obuf);
 
    highlight(filename, in_stream, out_stream);
    html = std::move(obuf.str());
#endif
}

void RepoHtmlGen::generate_file_page(const fs::path &file_path, const git_tree_entry *entry)
{
    const char *entry_name = git_tree_entry_name(entry);

    fs::path html_path = "public/" + m_repo_name + "/files/" + std::string(file_path) + ".html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    git_object *obj;
    if ((m_err = git_object_lookup(&obj, m_repo, git_tree_entry_id(entry), GIT_OBJ_ANY)) < 0)
        error("failed to lookup git object from index entry");

    std::string html_file_content;
    if (git_blob_is_binary((git_blob *)obj))
        html_file_content = "This is a binary file.";
    else
        generate_file_code_page(entry_name, (git_blob *)obj, html_file_content);

    std::ofstream out_stream(html_path, std::ios::out);
    if (!out_stream.is_open())
        error("failed to open output file.");

    auto size_info = format_filesize(git_blob_rawsize((git_blob *)obj));
    out_stream << fmt::format(
        file_page_template,
        fmt::arg("header_content", m_header_content),
        fmt::arg("repo_name", m_repo_name),
        fmt::arg("filename", entry_name),
        fmt::arg("fileview_content",
            fmt::format(
                file_view_template,
                fmt::arg("filename", entry_name),
                fmt::arg("file_content", html_file_content),
                fmt::arg("file_size", size_info.first),
                fmt::arg("file_size_unit", size_info.second)
            )
        )
    );

    git_object_free(obj);
}

void RepoHtmlGen::generate_tree_pages(git_tree *tree, std::string root)
{
    fs::path html_path =
        root == "" ? "public/" + m_repo_name + "/index.html"
                   : "public/" + m_repo_name + "/tree/" + root + "index.html";
    if (!fs::exists(html_path.parent_path()))
        fs::create_directories(html_path.parent_path());

    std::ofstream out_stream(html_path, std::ios::out);
    if (!out_stream.is_open())
        error("failed to open output file.");

    size_t tree_entry_count = git_tree_entrycount(tree);

    std::string tree_html;
    tree_html.reserve(tree_entry_count * sizeof(file_tree_line_template));
    for (size_t i = 0; i < tree_entry_count; i++) {
        const char *entry_name;
        const git_tree_entry *entry = nullptr;
        git_object *obj;

        if (!(entry = git_tree_entry_byindex(tree, i)))
            error("failed to retrieve tree entry");
        if (!(entry_name = git_tree_entry_name(entry)))
            error("failed to retrieve tree entry name");
        if ((m_err = git_tree_entry_to_object(&obj, m_repo, entry)) < 0)
            error("failed to convert tree entry to object");

        if (git_tree_entry_type(entry) == GIT_OBJ_TREE) {
            generate_tree_pages((git_tree *)obj, root + entry_name + '/');
            tree_html += fmt::format(
                file_tree_line_dir_template,
                fmt::arg("file_tree_name", entry_name),
                fmt::arg("file_tree_link", '/' + m_repo_name + "/tree/" + root + entry_name)
            );
            git_object_free(obj);
            continue;
        }

        generate_file_page(root + entry_name, entry);

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

    out_stream << fmt::format(
        file_index_template,
        fmt::arg("repo_name", m_repo_name),
        fmt::arg("header_content", m_header_content),
        fmt::arg("readme_content", root == "" ? m_readme_content : ""),
        fmt::arg("tree_content", tree_html),
        fmt::arg("tree_path", root)
    );
}

struct diff_printer_passthrough {
    diff_printer_passthrough(size_t max)
        : max_line_no(max)
    {
    }

    std::string html;
    const size_t max_line_no;
    size_t line_hunk_hdr_no { 0 };
    size_t line_no { 0 };
};

static int diff_printer(const git_diff_delta *, const git_diff_hunk *,
        const git_diff_line *line, void *void_pass)
{
    diff_printer_passthrough *passthrough = (diff_printer_passthrough *)void_pass;
    if (++passthrough->line_no >= passthrough->max_line_no) {
        passthrough->html += diff_max_line_count;
        return 1;
    }

    const char *diff_template;
    switch (line->origin) {
    case GIT_DIFF_LINE_ADDITION:
        diff_template = diff_add_template;
        break;
    case GIT_DIFF_LINE_DELETION:
        diff_template = diff_del_template;
        break;
    case GIT_DIFF_LINE_ADD_EOFNL:
        diff_template = diff_add_eofnl_template;
        break;
    case GIT_DIFF_LINE_DEL_EOFNL:
        diff_template = diff_del_eofnl_template;
        break;
    case GIT_DIFF_LINE_FILE_HDR:
        diff_template = diff_file_hdr_template;
        break;
    case GIT_DIFF_LINE_HUNK_HDR:
        passthrough->html += fmt::format(
            diff_hunk_hdr_template,
            passthrough->line_hunk_hdr_no,
            passthrough->line_hunk_hdr_no,
            line->content,
            line->content_len
        );
        passthrough->line_hunk_hdr_no++;
        return 0;
    default:
        diff_template = diff_line_template;
        break;
    }

    passthrough->html += fmt::format(
        diff_template,
        escape_string(std::string(line->content, line->content_len))
    );

    return 0;
}

void RepoHtmlGen::get_commit_info(git_commit *commit, CommitInfo &info)
{
    info.commit = commit;
    info.time = git_commit_time(commit);
    info.summary = git_commit_summary(commit);
    info.message = git_commit_message(commit);
    info.author = git_commit_author(commit);
    info.committer = git_commit_committer(commit);
    info.id = git_commit_id(commit);
    git_oid_tostr(info.id_str, sizeof(info.id_str), info.id);

    if ((m_err = git_tree_lookup(&info.tree, m_repo, git_commit_tree_id(commit))) < 0)
        error("failed to lookup commit tree");
    if ((m_err = git_commit_parent(&info.parent, commit, 0)) == 0) {
        info.parent_id = git_commit_id(info.parent);
        git_oid_tostr(info.parent_id_str, sizeof(info.parent_id_str), info.parent_id);
        if ((m_err = git_tree_lookup(&info.parent_tree, m_repo, git_commit_tree_id(info.parent))) < 0) {
            info.parent = nullptr;
            info.parent_tree = nullptr;
        }
    } else {
        info.parent = nullptr;
        info.parent_id = nullptr;
        info.parent_tree = nullptr;
    }

    git_diff_options diff_options;
    git_diff_init_options(&diff_options, GIT_DIFF_OPTIONS_VERSION);
    diff_options.flags |= GIT_DIFF_IGNORE_SUBMODULES | GIT_DIFF_INCLUDE_TYPECHANGE | GIT_DIFF_DISABLE_PATHSPEC_MATCH;

    git_diff_find_options diff_find_options;
    diff_find_options.flags |= GIT_DIFF_FIND_RENAMES | GIT_DIFF_FIND_COPIES | GIT_DIFF_FIND_EXACT_MATCH_ONLY;

    if ((m_err = git_diff_find_init_options(&diff_find_options, GIT_DIFF_FIND_OPTIONS_VERSION)) < 0)
        error("failed to find init options");
    if ((m_err = git_diff_tree_to_tree(&info.diff, m_repo, info.parent_tree, info.tree, &diff_options)) < 0)
        error("failed to collect difference from parent tree");
    if ((m_err = git_diff_find_similar(info.diff, &diff_find_options)) < 0)
        error("failed to transform diff");

    size_t delta_count = git_diff_num_deltas(info.diff);

    info.files = delta_count;
    info.deltas.reserve(delta_count);

    for (size_t i = 0; i < delta_count; i++) {
        git_patch *current_patch;
        if (git_patch_from_diff(&current_patch, info.diff, i) < 0)
            error("failed to get patch from diff");
        auto &delta_obj = info.deltas.emplace_back(current_patch);
        const git_diff_delta *delta = git_patch_get_delta(current_patch);

        if (delta->flags & GIT_DIFF_FLAG_BINARY)
            continue;

        size_t hunks_count = git_patch_num_hunks(delta_obj.patch);
        for (size_t j = 0; j < hunks_count; j++) {
            const git_diff_hunk *current_hunk;
            const git_diff_line *current_line;
            size_t current_hunk_lines;

            if (git_patch_get_hunk(&current_hunk, &current_hunk_lines, delta_obj.patch, j) < 0)
                error("failed to get hunk from patch");
            for (size_t k = 0; !git_patch_get_line_in_hunk(&current_line, delta_obj.patch, j, k); k++) {
                if (current_line->old_lineno == -1) {
                    delta_obj.gain++;
                    info.gain++;
                } else if (current_line->new_lineno == -1) {
                    delta_obj.loss++;
                    info.loss++;
                }
            }
        }

        info.hunks += hunks_count;
    }
}

void RepoHtmlGen::generate_commit_page(const CommitInfo &info)
{
    std::ofstream out_stream("public/" + m_repo_name + "/commits/" + info.id_str + ".html", std::ios::out);
    if (!out_stream.is_open())
        error("failed to open output file.");

    size_t diff_size_est =
        std::min(info.gain + info.loss + info.files * 4 + info.hunks, m_options.max_diff_lines) * LINE_SIZE_EST;

    diff_printer_passthrough passthrough(m_options.max_diff_lines);
    passthrough.html.reserve(diff_size_est);
    git_diff_print(info.diff, GIT_DIFF_FORMAT_PATCH, &diff_printer, &passthrough);

    out_stream << fmt::format(
        commit_page_template,
        fmt::arg("repo_name", m_repo_name),
        fmt::arg("header_content", m_header_content),
        fmt::arg("date", to_string(info.time)),
        fmt::arg("message", escape_string(info.message)),
        fmt::arg("author", escape_string(info.author->name)),
        fmt::arg("email", escape_string(info.author->email)),
        fmt::arg("commit", info.id_str),
        fmt::arg("parent", info.parent_id_str),
        fmt::arg("diff_content", passthrough.html)
    );
}

RepoHtmlGen::CommitInfo::~CommitInfo()
{
    git_commit_free(commit);
    git_commit_free(parent);
    git_tree_free(tree);
    git_tree_free(parent_tree);
    git_diff_free(diff);
    for (auto &d : deltas)
        git_patch_free(d.patch);
}

void RepoHtmlGen::generate_commit_pages()
{
    fs::path commits_dir = "public/" + m_repo_name + "/commits/";
    if (!fs::exists(commits_dir))
        fs::create_directories(commits_dir);

    git_revwalk *walk;
    git_oid oid;

    if ((m_err = git_revwalk_new(&walk, m_repo)) < 0)
        error("failed to create git revwalk");
    if ((m_err = git_revwalk_push_head(walk)) < 0)
        error("failed to push repository head to revision walker");

    git_commit *nth_commit;
    if ((m_err = git_commit_nth_gen_ancestor(&nth_commit, m_head_commit, m_options.max_commits)) == 0) {
        // this should be slightly faster than manually counting and breaking past m_options.max_commits
        // (see https://github.com/libgit2/libgit2/issues/4428#issuecomment-465959268)
        git_revwalk_hide(walk, git_commit_id(nth_commit));
        git_commit_free(nth_commit);
    }

    git_revwalk_sorting(walk, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_simplify_first_parent(walk);

    std::string commits_html;
    commits_html.reserve(m_options.max_commits * sizeof(commits_line_template));

    while (git_revwalk_next(&oid, walk) == 0) {
        git_commit *commit;
        if ((m_err = git_commit_lookup(&commit, m_repo, &oid)) < 0)
            error("failed to lookup commit");

        CommitInfo commit_info;
        get_commit_info(commit, commit_info);
        generate_commit_page(commit_info);

        commits_html += fmt::format(
            commits_line_template,
            fmt::arg("files", commit_info.files),
            fmt::arg("gain", commit_info.gain),
            fmt::arg("loss", commit_info.loss),
            fmt::arg("commit_link", '/' + m_repo_name + "/commits/" + commit_info.id_str + ".html"),
            fmt::arg("date", to_string(commit_info.time)),
            fmt::arg("author", escape_string(commit_info.author->name)),
            fmt::arg("summary", escape_string(commit_info.summary))
        );
    }

    git_revwalk_free(walk);

    std::ofstream out_stream("public/" + m_repo_name + "/commits.html", std::ios::out);
    if (!out_stream.is_open())
        error("failed to open output file.");

    out_stream << fmt::format(
        commits_page_template,
        fmt::arg("repo_name", m_repo_name),
        fmt::arg("header_content", m_header_content),
        fmt::arg("commits_content", commits_html)
    );
}
