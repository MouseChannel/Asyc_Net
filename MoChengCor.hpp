#pragma once
#include <coroutine>
#include <cstdio>
#include <iostream>
#include <utility>
#include <vector>

template <typename T>

struct promise_type_base {

  T get_return_object() {
    {
      return T{
          std::coroutine_handle<promise_type_base<T>>::from_promise(*this)};
    }
  }

  std::suspend_never initial_suspend() noexcept { return {}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  void unhandled_exception() { throw; }

 
};

struct MoChengCoroutine {

  using Handle = std::coroutine_handle<promise_type_base<MoChengCoroutine>>;
  using promise_type = promise_type_base<MoChengCoroutine>;
  MoChengCoroutine(Handle handle) : m_handle(handle) { printf("actually here2\n");}
  Handle m_handle;
  MoChengCoroutine(MoChengCoroutine &&other):m_handle(std::move(other.m_handle)) {other.m_handle = nullptr; }
 
  MoChengCoroutine() = delete;
  // ~MoChengCoroutine(){
  //  std::cout<<"delete"<<std::endl;
  // }
};

// 以下是task例子
// using TaskHandle = std::coroutine_handle<promise_type_base<Task>>;
struct Test_Task {
 int a;
  bool await_ready() { return false; }
  int await_resume() { return 0; };
  void await_suspend(
      std::coroutine_handle<promise_type_base<MoChengCoroutine>> handle) {
    std::cout << handle.address() << " |  " << a<<std::endl;
    std::cout << handle.address() << " |  " << a<<std::endl;
    std::cout << handle.address() << " |  " << a<<std::endl;
    
  };
  // auto operator co_await() { return AwaiterBase{}; }
};

//--
