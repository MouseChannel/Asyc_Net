#include "MoChengCor.hpp"
#include "MoChengIO.hpp"

#include "Singleton.hpp"
#include "ikcp.h"
#include "liburing.h"
#include "liburing/io_uring.h"
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

class MoChengSession;
class MoChengNet : public Singleton<MoChengNet> {
  static const int BUF_SIZE = 1024;

private:
  int socketFd;

  std::unique_ptr<MoChengIO> io;

  sockaddr_in receive_addr;

  std ::vector<char> recv_buf = std::vector<char>(BUF_SIZE);

  std::map<int, MoChengSession> sessions;

public:
  MoChengNet() {
    InitUDP();
    io = std::make_unique<MoChengIO>(socketFd);
  }
  MoChengNet(const MoChengNet &) = delete;

  MoChengNet(int socketFd, std::unique_ptr<MoChengIO> io,
             sockaddr_in receive_addr, std::map<int, MoChengSession> sessions)
      : socketFd(socketFd), io(std::move(io)), receive_addr(receive_addr),
        sessions(std::move(sessions)) {}
  void InitUDP() {

    socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketFd < 0) {
      printf("socket init fail: %d\n", socketFd);
    }
    printf("socketFD  : %d\n", socketFd);
    sockaddr_in serverAddr{AF_INET, 8888, inet_addr("127.0.0.1")};

    auto err = ::bind(socketFd, (sockaddr *)&serverAddr, sizeof(serverAddr));
    if (err < 0) {
      printf("bind fail: %d\n", err);
    }

    printf("Init UDP done\n");
  }

  MoChengCoroutine udp_recv();

  MoChengCoroutine udp_send(sockaddr_in addr, const char *buf, int len) {
    co_await io->udp_send((char *)buf, len, addr);
  }
  void SpawnNewSession(sockaddr_in addr);
};

class MoChengSession {
private:
  static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {

    auto userData = (MoChengSession *)user;
    auto endpoint = userData->addr;
    MoChengNet::Get_instance().udp_send(endpoint, buf, len);

    std::cout << "In output" << std::endl;

    return 0;
  }

  sockaddr_in addr;

  std::vector<char> buf;
  char *recv_buf;
  size_t recv_len;
  char *send_buf;
  std::function<void(char *)> onReceive;
  IKCPCB *kcp;
  int id;

public:
  MoChengSession() = delete;
  MoChengSession(sockaddr_in addr, int id) : addr(addr), id(id), buf(1024) {
    InitKCP();
  }
  MoChengSession(int id) {}

  void InitKCP() {
    kcp = ikcp_create(id, (void *)this);

    ikcp_nodelay(kcp, 2, 10, 2, 1);
    ikcp_wndsize(kcp, 128, 128);
    kcp->rx_minrto = 10;
    kcp->fastresend = 1;

    kcp->output = udp_output;
  }
  void Kcp_recv(char *buf, int nbytes) {
    // memcpy(recv_buf, buf, nbytes);
    // recv_len = nbytes;
    ikcp_input(kcp, buf, nbytes);
    ikcp_flush(kcp);
    auto res = ikcp_recv(kcp, buf, 1024);
    if (res > 0) {
      // todo
      std::cout << "kcp receive: " << buf << std::endl;
    }
  }

  void Kcp_send(char *buf, int len) { ikcp_send(kcp, buf,len); }
};

inline MoChengCoroutine MoChengNet::udp_recv() {
  {
    auto udp_receive = io->udp_recv(recv_buf.data(), 1024, receive_addr);
    while (true) {
      auto nbytes = co_await udp_receive;
      if (nbytes < 0)
        continue;
      auto sessionId = ikcp_getconv(recv_buf.data());
      if (sessions.contains(sessionId)) [[likely]] {
        sessions[sessionId].Kcp_recv(recv_buf.data(), nbytes);
      } else {
        SpawnNewSession(receive_addr);
      }
    }
  }
}

inline void MoChengNet::SpawnNewSession(sockaddr_in addr) {

  int id = 1;
  auto session = std::make_shared<MoChengSession>(addr, id);
  sessions.emplace(id, session);
}
