#include "awcotn/awcotn.h"
#include "awcotn/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "awcotn/timer.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();

int sock = 0;

void test_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //awcotn::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.10", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        AWCOTN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        awcotn::IOManager::GetThis()->addEvent(sock, awcotn::IOManager::READ, [](){
            AWCOTN_LOG_INFO(g_logger) << "read callback";
        });
        awcotn::IOManager::GetThis()->addEvent(sock, awcotn::IOManager::WRITE, [](){
            AWCOTN_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            awcotn::IOManager::GetThis()->cancelEvent(sock, awcotn::IOManager::READ);
            close(sock);
        });
    } else {
        AWCOTN_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    awcotn::IOManager iom(1, false);
    iom.schedule(&test_fiber);

    AWCOTN_LOG_INFO(g_logger) << "test1";
}

awcotn::Timer::ptr s_timer;
void test_timer() {
    awcotn::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        AWCOTN_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            //s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, true);
}


int main(int argc, char** argv) {
    test_timer();
    return 0;
}