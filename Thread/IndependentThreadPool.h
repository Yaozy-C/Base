//
// Created by Yaozy on 2021/5/14.
//

#ifndef BASE_INDEPENDENTTHREADPOOL_H
#define BASE_INDEPENDENTTHREADPOOL_H

#include "IndependentThread.h"

namespace Base {
    class IndependentThreadPool {
    public:

        explicit IndependentThreadPool(int size) : size_(size) {

            for (int i = 0; i < size_; ++i) {
                std::shared_ptr<EventLoop> independentThreadVoid(new EventLoop);
                independentVoids[i] = independentThreadVoid;
            }
        };

        ~IndependentThreadPool() {

            for (auto &independentVoid : independentVoids) {
                independentVoid.second->Shutdown();
            }
        };

        std::shared_ptr<EventLoop> GetIndependentThreadVoid(const int &index) {
            return independentVoids[index % size_];
        }


        [[nodiscard]] int GetSize() const {
            return size_;
        }

    private:
        std::map<int, std::shared_ptr<EventLoop>> independentVoids;
        int size_;
    };
}


#endif //BASE_INDEPENDENTTHREADPOOL_H
