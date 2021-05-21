//
// Created by Yaozy on 2021/5/12.
//

#ifndef BASE_EPOLLER_H
#define BASE_EPOLLER_H

#include <sys/epoll.h>
#include <vector>
#include <memory>
#include "../Thread/IndependentThread.h"

namespace Base {
    namespace Net {
        namespace Tcp {
            namespace Sockets {

                class Epoll {
                public:
                    Epoll();

                    ~Epoll();

                    int AddEvent(int fd);

                    int DELEvent(int fd);

                    int MODEvent(int fd, int opt) const;

                    int Wait(int size, std::vector<struct epoll_event> &events ,const int &time) const;

                    int GetSize();

                    void SetIndependentThreadVoid(const  std::shared_ptr<IndependentThreadVoid> &independentThreadVoid);

                    std::shared_ptr<IndependentThreadVoid> GetLoop(){
                        return independentThreadVoid_;
                    }

                    bool Looping() {return loop;}
                private:
                    std::shared_ptr<IndependentThreadVoid> independentThreadVoid_;
                    std::atomic<bool> loop;
                    std::atomic<int> size_;
                    int fd_;
                };
            }
        }
    }
}
#endif //BASE_EPOLLER_H
