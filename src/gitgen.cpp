#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include "repo.h"
#include "index.h"

static void usage(char *name)
{
    fmt::print(stderr, "usage: {} repo <path>\n", name);
    fmt::print(stderr, "       {} index\n", name);
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

        RepoHtmlGen gen(argv[2]);
        gen.generate();
    } else if (cmd == "index") {
        if (argc != 2)
            usage(argv[0]);

        IndexHtmlGen gen;
        gen.generate();
    } else {
        usage(argv[0]);
    }

    return 0;
}
