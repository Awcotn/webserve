#include "awcotn/hook.h"
#include "awcotn/iomanager.h"
#include "awcotn/log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("root");
void test_sleep() {
    awcotn::IOManager iom(1);

    iom.schedule([]() {
        sleep(2);
        AWCOTN_LOG_INFO(g_logger) << "sleep 2 second end";
    });

    iom.schedule([]() {
        sleep(3);
        AWCOTN_LOG_INFO(g_logger) << "sleep 3 second end";
    });

    AWCOTN_LOG_INFO(g_logger) << "test_sleep begin";
    
}

void test_socket() {
    //awcotn::set_hook_enable(false);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    AWCOTN_LOG_INFO(g_logger) << "sock=" << sock;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.66", &addr.sin_addr.s_addr);
    //awcotn::set_hook_enable(true);
    AWCOTN_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    AWCOTN_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    AWCOTN_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    AWCOTN_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    AWCOTN_LOG_INFO(g_logger) << buff;
}

int main() {
    awcotn::IOManager iom;
    //test_sleep();
    test_socket();
    return 0;
}