#include <iostream>
template <typename T> class Singleton {
public:
   static T &Get_instance() {
    static T instance;
    return instance;
  }
  virtual ~Singleton() { std::cout << "destructor called!" << std::endl; }
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

protected:
  Singleton() { std::cout << "constructor called!" << std::endl; }
};