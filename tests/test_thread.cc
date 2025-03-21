#include "awcotn/awcotn.h"
#include <vector>
#include <unistd.h>
awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();
awcotn::RWMutex s_mutex;
int count = 0;
void fun1() {
    AWCOTN_LOG_INFO(g_logger) << "name: " << awcotn::Thread::GetName()
        << " this.name: " << awcotn::Thread::GetThis()->getName()
        << " id: " << awcotn::GetThreadId()
        << " this.id: " << awcotn::Thread::GetThis()->getId();
    //sleep(100);
    for(int i =0; i < 100000; i++) {
        awcotn::RWMutex::WriteLock lock(s_mutex);
        count++;
    }
}

void fun2() {

}
int main(int argc,char** argv) {
    AWCOTN_LOG_INFO(g_logger) << "thread test begin";
    std::vector<awcotn::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        awcotn::Thread::ptr thr(new awcotn::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }
    
    for(int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    AWCOTN_LOG_INFO(g_logger) << "thread test end";
    printf("count=%d\n", count);
    return 0;
} 