#include <string>
#include <variant>
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "index.h"

static void usage(char *name)
{
    fmt::print(stderr, "usage: {} repo <path> [--max-commits <max>] [--max-filesize <max>] [--max-diff-lines <max>]\n", name);
    fmt::print(stderr, "       {} index\n", name);
    exit(1);
}

struct Args {
    Args(size_t argc, char **argv);
    ~Args();

    enum class CmdType {
        None,
        Repo,
        Index
    } cmd_type { CmdType::None };

    std::variant<RepoHtmlGen::Options, IndexHtmlGen::Options> options;
    bool touched_repo_options { false };
    bool touched_index_options { false };
};

int main(int argc, char **argv)
{
    if (argc == 1)
        usage(argv[0]);

    Args args(argc, argv);

    if (args.cmd_type == Args::CmdType::Repo) {
        RepoHtmlGen gen(std::get<RepoHtmlGen::Options>(args.options));
        gen.generate();
    } else if (args.cmd_type == Args::CmdType::Index) {
        IndexHtmlGen gen(std::get<IndexHtmlGen::Options>(args.options));
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
            std::get<RepoHtmlGen::Options>(options).repo_path = argv[i];
            cmd_type = CmdType::Repo;
            touched_repo_options = true;
        } else if (arg == "index") {
            cmd_type = CmdType::Index;
            touched_index_options = true;
        } else if (arg == "--max-commits") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            std::get<RepoHtmlGen::Options>(options).max_commits = std::stoi(arg1);
            touched_repo_options = true;
        } else if (arg == "--max-filesize") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            std::get<RepoHtmlGen::Options>(options).max_view_filesize = std::stoi(arg1);
            touched_repo_options = true;
        } else if (arg == "--max-diff-lines") {
            if (++i >= argc)
                usage(argv[0]);
            const std::string arg1(argv[i]);
            std::get<RepoHtmlGen::Options>(options).max_diff_lines = std::stoi(arg1);
            touched_repo_options = true;
        } else {
            usage(argv[0]);
        }
    }

    if (cmd_type == CmdType::None)
        usage(argv[0]);
    if ((cmd_type == CmdType::Repo && touched_index_options) ||
            (cmd_type == CmdType::Index && touched_repo_options))
        usage(argv[0]);
    if (cmd_type == CmdType::Repo && std::get<RepoHtmlGen::Options>(options).repo_path == "")
        usage(argv[0]);
}

Args::~Args()
{
}
