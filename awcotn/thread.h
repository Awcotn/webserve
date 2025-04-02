#ifndef __AWCOTN_THREAD_H__
#define __AWCOTN_THREAD_H__

//pthread_XXX
//std::thread, pthread

#include <thread>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include "mutex.h"

namespace awcotn {
class Thread {
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);

private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator = (const Thread&) = delete; 
private:
    static void* run(void* arg);

    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;

    Semaphore m_semaphore;

};

}


#endif