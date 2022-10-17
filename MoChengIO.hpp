#pragma once
#include "MoChengIOTask.hpp"
#include "ikcp.h"
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <liburing.h>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>

class MoChengIO {
  static const int entries = 64;
  static const int GID = 12;
  static const int BUFFER_SIZE = 1024;
  static const int BUFFER_COUNT = 1024;
  enum io_type { M_ACCEPT, M_RECEIVE, M_SEND, M_PROVIDE };
  struct io_data {
    io_type type;
    int bid;
    sockaddr_in sockAddr;
  };

private:
  io_uring ring;
  io_uring_sqe *sqe;
  io_uring_cqe *cqe;
  int socketFD;
  int cqe_count;
  // std::map<int, char *> bufs;
  char bufs[BUFFER_COUNT][BUFFER_SIZE];

public:
  MoChengIO(int _socketFD) : socketFD(_socketFD) {
    auto err = io_uring_queue_init(entries, &ring, 0);
    if (err < 0)
      throw;
  }
  void Work() {
    while (true) {
      auto err = io_uring_submit_and_wait(&ring, 1);
      if (err < 0) {
        printf("wait cqe failed: %d\n ", err);
        return;
      }
      int a = 0;
      io_uring_for_each_cqe(&ring, a, cqe) {
        ++cqe_count;
        auto io_task = static_cast<IO_Task *>(io_uring_cqe_get_data(cqe));
        if (io_task->need_resume)
          io_task->resume(cqe->res);
      }
      io_uring_cq_advance(&ring, cqe_count);
      cqe_count = 0;
    }
  }

  /*
    void Multishot_accept(int socketFD) {
      sqe = io_uring_get_sqe(&ring);
      io_data *data = new io_data{
          .type = io_type::M_ACCEPT,
      };
      socklen_t size = sizeof(sockaddr_in);
      io_uring_prep_multishot_accept(sqe, socketFD, (sockaddr *)&data->sockAddr,
                                     &size, 0);

      io_uring_sqe_set_data(sqe, data);
    }
    void ProvideBufferInit() {
      sqe = io_uring_get_sqe(&ring);
      io_uring_prep_provide_buffers(sqe, bufs, BUFFER_SIZE, BUFFER_COUNT, GID,
    0); io_uring_submit(&ring); io_uring_wait_cqe(&ring, &cqe); if (cqe->res <
    0) { printf("cqe->res = %d\n", cqe->res); exit(1);
      }
      io_uring_cqe_seen(&ring, cqe);
      printf("provide buf init done\n");
    }
    void ProvideBuffer(int bid) {
      sqe = io_uring_get_sqe(&ring);

      io_uring_prep_provide_buffers(sqe, bufs[bid], BUFFER_SIZE, 1, GID, bid);
    }

    void Recv() {
      sqe = io_uring_get_sqe(&ring);
      io_uring_prep_recv(sqe, socketFD, nullptr, BUFFER_SIZE, 0);
      io_uring_sqe_set_flags(sqe, IOSQE_BUFFER_SELECT);
      sqe->buf_group = GID;
    }
    void Send(char *mes, size_t len, iovec iodata, sockaddr_in addr) {
      sqe = io_uring_get_sqe(&ring);

      msghdr msg{
          .msg_name = &addr,
          .msg_namelen = sizeof(addr),
          .msg_iov = &iodata,
          .msg_iovlen = 1,
      };

      io_uring_prep_sendmsg(sqe, socketFD, &msg, 0);
    }
    */

  io_uring_sqe *Get_Sqe() { return io_uring_get_sqe(&ring); }

  IO_Task udp_recv(void *buf, size_t nbytes, sockaddr_in addr);
  // Kcp_Receive_Task kcp_recv(void *buf, size_t nbytes);

  IO_Task udp_send(void *buf, size_t nbytes, sockaddr_in addr);

  //---
};

inline IO_Task MoChengIO::udp_recv(void *buf, size_t nbytes, sockaddr_in addr) {
  auto sqe = Get_Sqe();
  iovec data{.iov_base = buf, .iov_len = nbytes};
  msghdr msg{
      .msg_name = &addr,
      .msg_namelen = sizeof(addr),
      .msg_iov = &data,
      .msg_iovlen = 1,

  };
  io_uring_prep_recvmsg(sqe, socketFD, &msg, 0);
  return IO_Task{sqe, true};
}

inline IO_Task MoChengIO::udp_send(void *buf, size_t nbytes, sockaddr_in addr) {
  auto sqe = Get_Sqe();
  iovec data{.iov_base = buf, .iov_len = nbytes};
  msghdr msg{
      .msg_name = &addr,
      .msg_namelen = sizeof(addr),
      .msg_iov = &data,
      .msg_iovlen = 1,

  };
  io_uring_prep_sendmsg(sqe, socketFD, &msg, 0);
  return IO_Task{sqe};
}
