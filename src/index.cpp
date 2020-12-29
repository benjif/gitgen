#include <fstream>
#include <iostream>
#include <filesystem>
#include "index.h"
#include "templates.h"

namespace fs = std::filesystem;

IndexHtmlGen::IndexHtmlGen(const Options &opt)
    : m_options(opt)
{
    for (auto &sub : fs::directory_iterator("public")) {
        if (sub.path() != "css")
            m_repo_paths.push_back(sub.path());
    }
}

IndexHtmlGen::~IndexHtmlGen()
{
}

void IndexHtmlGen::generate()
{
}
