#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "index.h"

static void usage(char *name)
{
    fmt::print(stderr, "usage: {} repo <path>\n", name);
    fmt::print(stderr, "       {} index <repo_links>...\n", name);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc == 1)
        usage(argv[0]);

    const std::string cmd = argv[1];
    if (cmd == "repo") {
        if (argc != 3)
            usage(argv[0]);

        // TODO: offer changing name?
        RepoHtmlGen gen(argv[2]);
        gen.generate();
    } else if (cmd == "index") {
        if (argc < 3)
            usage(argv[0]);

        std::vector<std::string> paths(&argv[2], &argv[argc]);
        IndexHtmlGen gen(paths);
    } else {
        usage(argv[0]);
    }

    return 0;
}
