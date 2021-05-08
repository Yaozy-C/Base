//
// Created by Yaozy on 2021/4/21.
//

#ifndef BASE_TOOLS_H
#define BASE_TOOLS_H

#include <fstream>
#include <sstream>
#include <sys/stat.h>
std::string ReadFile(const std::string &file);

long long FileSize(const std::string &filepath);
#endif //BASE_TOOLS_H
