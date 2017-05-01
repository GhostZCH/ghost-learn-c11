
#include <set>
#include <iostream>
#include <string.h>
#include <unordered_map>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define MAX_BUF_LEN (1024)

const int port = 8000;
const char* host = "127.0.0.1";

const char *head = "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: 10\r\n\r\n0123456789";
const int head_len = strlen(head);


struct Client {
    int fd;
    bool read;

    int recv;
    int send;
    char buf[MAX_BUF_LEN];

    Client(int fd=-1)
        :fd(fd),read(true),recv(0),send(0){}
};


class Server {
public:
    Server(const char* host, const int port) {
        struct sockaddr_in svr;
        socklen_t len = sizeof(svr);
        svr.sin_family = AF_INET;
        svr.sin_port = htons(port);
        svr.sin_addr.s_addr = inet_addr(host);

        svr_= socket(AF_INET, SOCK_STREAM, 0);
        fcntl(svr_, F_SETFL, O_NONBLOCK);

        bind(svr_, (struct sockaddr *)&svr, sizeof(struct sockaddr));
        listen(svr_, 1024);                                                                                                                                         

        epoll_ = epoll_create(4096);
        EpollAdd(svr_);
    }

    void EpollAdd(int fd, int events = EPOLLIN|EPOLLET) {
        struct epoll_event event; 
        event.data.fd = fd;
        event.events = events;
        epoll_ctl(epoll_, EPOLL_CTL_ADD, fd, &event);
    }

    void Read(int fd) {
        int n;
        Client *client = &(clients_[fd]);

        while(true) {
            n = read(fd, &(client->buf[client->recv]), MAX_BUF_LEN - client->recv);
            if (n <= 0) {
                break;
            }
            client->recv += n;
        }

        if (strstr(client->buf, "\r\n\r\n") != nullptr) {
            client->buf[client->recv] = '\0';

            client->read = false;
            client->recv = 0;
            client->send = 0;

            Write(fd);
        }
    }

    void Write(int fd) {

        int n;
        Client *client = &(clients_[fd]);
        
        while (client->send < head_len) {
            n = write(fd, head + client->send, head_len - client->send);
            if (n <= 0) {
                break;
            }

            client->send += n;
        }
    }

    bool AddClient(int fd) {
        if (fd <= 0) {
            return false;
        }

        fcntl(fd, F_SETFL, O_NONBLOCK);
        clients_[fd] = Client(fd);
        EpollAdd(fd);
        return true;
    }

    void Start() {
        struct epoll_event event;
        struct epoll_event events[1024];

        while (true) {    
            int n = epoll_wait(epoll_, events, 1024, 1);

            for (int i = 0; i < n; i++) {
                if (events[i].data.fd != svr_) {
                    Read(events[i].data.fd);
                    continue;
                }

                while (AddClient(accept(svr_, NULL, NULL)));
            }
        }
    }

    ~Server() {
        close(svr_);
        
        auto i = clients_.begin();
        for (; i != clients_.end(); i++) {
            close(i->first);
        }
    }

private:
    
    int epoll_;
    int svr_;
    unordered_map<int, Client> clients_;
};


int main(int argn, const char* argv[])
{
    Server svr(host, port);

    svr.Start();
   
    return 0;
}
