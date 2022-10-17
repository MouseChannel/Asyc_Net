#pragma once
#include "MoChengCor.hpp"
#include "ikcp.h"
#include "liburing.h"

#include <coroutine>

struct IO_Task {
  bool need_resume;
  int res;
  std::coroutine_handle<> handle;
  io_uring_sqe *sqe;
  void resume(int _res) {
    res = _res;
    handle.resume();
  }
  bool await_ready() noexcept { return false; }
  auto await_resume() noexcept { return res; }
  void await_suspend(std::coroutine_handle<> handle) noexcept {
    handle = handle;
    io_uring_sqe_set_data(sqe, this);
  }

  IO_Task() = delete;
  IO_Task(io_uring_sqe *sqe, bool need_resume = false)
      : sqe(sqe), need_resume(need_resume) {}
};

struct Kcp_Receive_Task {
  char *buf;
  size_t nbytes;
  IKCPCB *kcp;
  int res;

  bool await_ready() noexcept { return false; }
  auto await_resume() noexcept {
    ikcp_input(kcp, buf, nbytes);
    ikcp_flush(kcp);
    res = ikcp_recv(kcp, buf, 1024);
    return res;
  }
  void await_suspend(std::coroutine_handle<> handle) noexcept {}
};
struct Kcp_Flush_Task {
  IKCPCB *kcp;
  bool await_ready() noexcept { return false; }
  auto await_resume() noexcept {
    ikcp_flush(kcp);
    return ikcp_peeksize(kcp);
  }
  void await_suspend(std::coroutine_handle<> handle) noexcept {}
};

struct Kcp_Send_Task {

  char *buf;
  size_t nbytes;
  IKCPCB *kcp;
  int res;
  bool await_ready() noexcept { return false; }
  auto await_resume() noexcept { return res; }
  void await_suspend(std::coroutine_handle<> handle) noexcept {
    res = ikcp_send(kcp, buf, nbytes);
    ikcp_flush(kcp);
    handle.resume();
  }
};

struct TTask {
  bool await_ready() noexcept { return false; }
  auto await_resume() noexcept { return 0; }
  void await_suspend(std::coroutine_handle<> handle) noexcept {}
};
