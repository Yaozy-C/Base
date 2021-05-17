//
// Created by Yaozy on 2021/1/26.
//

#ifndef BASE_SAFEQUE_H
#define BASE_SAFEQUE_H

#include <queue>
#include <mutex>

namespace Base {

    template<typename T>
    class Queue {
    private:
        std::mutex queLock;
        std::queue<T> myQue;
    public:
        bool Empty() {
            std::unique_lock<std::mutex> lock(queLock);
            bool stat = myQue.empty();
            return stat;
        }

        Queue()=default;

        Queue( std::queue<T> &&que){
            myQue = que;
        };

        int Size() {
            std::unique_lock<std::mutex> lock(queLock);
            int size = myQue.size();
            return size;
        }

        std::queue<T> &Get(){
            std::unique_lock<std::mutex> lock(queLock);
            return myQue;
        }

        void Swap(std::queue<T> &temp) {
            std::unique_lock<std::mutex> lock(queLock);
            myQue.swap(temp);
            return ;
        }

        void Enque(T &t) {
            std::unique_lock<std::mutex> lock(queLock);
            myQue.push(t);
        }

        bool Deque(T &t) {
            std::unique_lock<std::mutex> lock(queLock);
            if (myQue.empty()) {
                lock.unlock();
                return false;
            }
            t = move(myQue.front());
            myQue.pop();
            return true;
        }
    };
}

#endif //BASE_SAFEQUE_H
