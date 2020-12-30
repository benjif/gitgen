#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "index.h"

namespace fs = std::filesystem;

static void usage(char *name)
{
    fmt::print(stderr, "usage: {} repo <path> [--max-commits <max>] [--max-filesize <max>] [--max-diff-lines <max>]\n", name);
    fmt::print(stderr, "       {} index <repo path>...\n", name);
    exit(1);
}

struct Args {
    Args(size_t argc, char **argv);

    enum class CmdType {
        None,
        Repo,
        Index
    } cmd_type { CmdType::None };

    RepoHtmlGen::Options repo_options;
    IndexHtmlGen::Options index_options;

    bool touched_repo_options { false };
    bool touched_index_options { false };
};

int main(int argc, char **argv)
{
    if (argc == 1)
        usage(argv[0]);

    Args args(argc, argv);

    if (args.cmd_type == Args::CmdType::Repo) {
        RepoHtmlGen gen(args.repo_options);
        gen.generate();
    } else if (args.cmd_type == Args::CmdType::Index) {
        IndexHtmlGen gen(args.index_options);
        gen.generate();
    }

    return 0;
}

Args::Args(size_t argc, char **argv)
{
    if (argc == 1)
        usage(argv[0]);

    for (size_t i = 1; i < argc; i++) {
        const std::string arg(argv[i]);

        if (arg == "repo") {
            if (++i >= argc)
                usage(argv[0]);
            repo_options.repo_path = argv[i];
            cmd_type = CmdType::Repo;
            touched_repo_options = true;
        } else if (arg == "index") {
            cmd_type = CmdType::Index;
            touched_index_options = true;
        } else if (arg == "--max-commits") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            repo_options.max_commits = std::stoi(arg1);
            touched_repo_options = true;
        } else if (arg == "--max-filesize") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            repo_options.max_view_filesize = std::stoi(arg1);
            touched_repo_options = true;
        } else if (arg == "--max-diff-lines") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            repo_options.max_diff_lines = std::stoi(arg1);
            touched_repo_options = true;
        } else if (cmd_type == CmdType::Index) {
            index_options.repo_paths.push_back(argv[i]);
        } else {
            usage(argv[0]);
        }
    }

    if (cmd_type == CmdType::None)
        usage(argv[0]);
    if ((cmd_type == CmdType::Repo && touched_index_options) ||
            (cmd_type == CmdType::Index && touched_repo_options))
        usage(argv[0]);
    if (cmd_type == CmdType::Repo && repo_options.repo_path == "")
        usage(argv[0]);
    if (cmd_type == CmdType::Index && index_options.repo_paths.size() == 0)
        usage(argv[0]);
}
