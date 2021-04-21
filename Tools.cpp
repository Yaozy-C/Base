//
// Created by Yaozy on 2021/4/21.
//

#include "Tools.h"


std::string ReadFile(const std::string &file){
    std::ifstream f(file);
    std::stringstream buf;
    buf<<f.rdbuf();
    return std::string(buf.str());
}

long long FileSize(const std::string &filepath){
    struct stat info{};
    stat(filepath.c_str(), &info);
    off_t size = info.st_size;
    return size;
}