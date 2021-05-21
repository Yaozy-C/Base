//
// Created by Yaozy on 2021/1/7.
//

#ifndef BASE_BUFFER_H
#define BASE_BUFFER_H

#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <cassert>

namespace Base {
//    static const size_t InitSize = 1024;
//
//    class Buffer {
//    public:
//        explicit Buffer(size_t initSize = InitSize);
//
//        ~Buffer();
//
//        void Append(const std::string &newData);
//
//        std::string GetPackage();
//
//        std::string GetAll();
//
//        void SetHead(const std::string &head);
//
//    private:
//        [[nodiscard]] size_t WriteableBytes() const;
//
//        void GetHeadIndex();
//
//        void MakeSpace(int len);
//
//        std::vector<char> data;
//        std::vector<char> head;
//        size_t headIndex;
//        size_t nextHeadIndex;
//
//        size_t readIndex;
//        size_t writeIndex;
//    };


    class Buffer {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
                : buffer_(kCheapPrepend + initialSize),
                  readerIndex_(kCheapPrepend),
                  writerIndex_(kCheapPrepend) {
            assert(readableBytes() == 0);
            assert(writableBytes() == initialSize);
            assert(prependableBytes() == kCheapPrepend);
        }

        bool empty() const {
            return readerIndex_ == writerIndex_;
        }

        void swap(Buffer &rhs) {
            buffer_.swap(rhs.buffer_);
            std::swap(readerIndex_, rhs.readerIndex_);
            std::swap(writerIndex_, rhs.writerIndex_);
        }

        size_t readableBytes() const { return writerIndex_ - readerIndex_; }

        size_t writableBytes() const { return buffer_.size() - writerIndex_; }

        size_t prependableBytes() const { return readerIndex_; }

        const char *peek() const { return begin() + readerIndex_; }

        const char *findCRLF() const {
            const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? nullptr : crlf;
        }

        const char *findCRLF(const char *start) const {
            assert(peek() <= start);
            assert(start <= beginWrite());
            const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? nullptr : crlf;
        }

        const char *findEOL() const {
            const void *eol = memchr(peek(), '\n', readableBytes());
            return static_cast<const char *>(eol);
        }

        const char *findEOL(const char *start) const {
            assert(peek() <= start);
            assert(start <= beginWrite());
            const void *eol = memchr(start, '\n', beginWrite() - start);
            return static_cast<const char *>(eol);
        }

        void retrieve(size_t len) {
            assert(len <= readableBytes());
            if (len < readableBytes()) {
                readerIndex_ += len;
            } else {
                retrieveAll();
            }
        }

        void retrieveUntil(const char *end) {
            assert(peek() <= end);
            assert(end <= beginWrite());
            retrieve(end - peek());
        }

        void retrieveInt64() {
            retrieve(sizeof(int64_t));
        }

        void retrieveInt32() {
            retrieve(sizeof(int32_t));
        }

        void retrieveInt16() {
            retrieve(sizeof(int16_t));
        }

        void retrieveInt8() {
            retrieve(sizeof(int8_t));
        }

        void retrieveAll() {
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend;
        }

        std::string retrieveAllAsString() {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveAsString(size_t len) {
            assert(len <= readableBytes());
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        void append(const char * /*restrict*/ data, size_t len) {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }

        void append(const void * /*restrict*/ data, size_t len) {
            append(static_cast<const char *>(data), len);
        }

        void ensureWritableBytes(size_t len) {
            if (writableBytes() < len) {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

        char *beginWrite() { return begin() + writerIndex_; }

        const char *beginWrite() const { return begin() + writerIndex_; }

        void hasWritten(size_t len) {
            assert(len <= writableBytes());
            writerIndex_ += len;
        }

        void unwrite(size_t len) {
            assert(len <= readableBytes());
            writerIndex_ -= len;
        }


        void appendInt8(int8_t x) {
            append(&x, sizeof x);
        }


        int8_t readInt8() {
            int8_t result = peekInt8();
            retrieveInt8();
            return result;
        }


        int8_t peekInt8() const {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t x = *peek();
            return x;
        }

        void prependInt8(int8_t x) {
            prepend(&x, sizeof x);
        }

        void prepend(const void * /*restrict*/ data, size_t len) {
            assert(len <= prependableBytes());
            readerIndex_ -= len;
            const char *d = static_cast<const char *>(data);
            std::copy(d, d + len, begin() + readerIndex_);
        }


        size_t internalCapacity() const {
            return buffer_.capacity();
        }


        ssize_t readFd(const char *data, size_t len);

    private:

        char *begin() { return &*buffer_.begin(); }

        const char *begin() const { return &*buffer_.begin(); }

        void makeSpace(size_t len) {
            if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
                // FIXME: move readable data
                buffer_.resize(writerIndex_ + len);
            } else {
                // move readable data to the front, make space inside buffer
                assert(kCheapPrepend < readerIndex_);
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                          begin() + writerIndex_,
                          begin() + kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
                assert(readable == readableBytes());
            }
        }

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;

        static const char kCRLF[];
    };
}

#endif //BASE_BUFFER_H
