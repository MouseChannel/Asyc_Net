

#include "MoChengCor.hpp"
#include"MoChengNet.hpp"
#include <coroutine>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

/*
void T(std::function<void()> d) { d(); }

MoChengCor Test() {
  // printf("here");
  auto b =
      std::bind([](int a, int b) { std::cout << a << b << std::endl; }, 2, 3);
  char aa[12];

  MoChengCor::Awaiter mo(b, aa);
  MoChengCor::TestAwait test{};
  std::string m = "hello";
  std::cout << m << std::endl;
  while (true) {
    std::cout << m << std::endl;
    // cout<<m<<endl;
    auto s = co_await test;
    std::cout << "s = " << s << std::endl;
  }
}
*/

MoChengCoroutine Test1() { co_await Test_Task{2}; }

struct A {

  A() { printf("  here\n"); }
  A(A &a) { printf("  here1"); }
  A(A &&a) { printf("  here12"); }
};

std::vector<char> v(1024);
int main(int, char **) {
  std::cout << "Hello, world!\n";
  //   main1();
  // T_Udp udp;
  // udp.Send();
   auto aaa =std::make_unique<MoChengCoroutine> (Test1());
  // std::unique_ptr<MoChengCoroutine> sendCor = & a ;
  // auto s = std::make_unique<A>(A{} );
  std::cout<<aaa->m_handle.address()<<std::endl;
  // Test1();
  return 1;

  v.push_back('1');
  auto a = v.data();
  printf("%s", v.data());
  // v.data();
  // udp.Receive();

  auto aa = Test1();
  // std::cout << "Hello, world2!\n";
  // auto bb = TestA();
  std::string mes;
  while (true) {
    // std::cin >> mes;
    // printf(aa)
    // aa.m_handle.promise().mes_len = 12;
    // aa.m_handle.resume();

    // bb.m_handle.resume();
  }

  return 0;
}
