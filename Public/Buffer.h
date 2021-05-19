//
// Created by Yaozy on 2021/1/7.
//

#ifndef BASE_BUFFER_H
#define BASE_BUFFER_H

#include <vector>
#include <algorithm>
#include <string>

namespace Base {
    static const size_t InitSize = 1024;

    class Buffer {
    public:
        explicit Buffer(size_t initSize = InitSize);

        ~Buffer();

        void Append(const std::string &newData);

        std::string GetPackage();

        std::string GetAll();

        void SetHead(const std::string &head);

    private:
        [[nodiscard]] size_t WriteableBytes() const;

        void GetHeadIndex();

        void MakeSpace(int len);

        std::vector<char> data;
        std::vector<char> head;
        size_t headIndex;
        size_t nextHeadIndex;

        size_t readIndex;
        size_t writeIndex;
    };
}

#endif //BASE_BUFFER_H
