#include <string>
#include <iostream>
//using namespace std;

int t = 9;
int p = 10;

template<typename T>
void f(T s)
{
    std::cout << s << '\n';
}

int function(){
  int x = 8;
  int y = 9;
  std::cout << "function() " << std::endl;
  return x+y;
}

template <class a_type> 
struct templ_class {
  a_type a;
};

template <class A_Type> class calc
{
  public:
    A_Type multiply(A_Type x, A_Type y);
    A_Type add(A_Type x, A_Type y);
};
template <class A_Type> A_Type calc<A_Type>::multiply(A_Type x,A_Type y)
{
  return x*y;
}
template <class A_Type> A_Type calc<A_Type>::add(A_Type x, A_Type y)
{
  return x+y;
}

struct SA{
private:
 int sa;
};

class A{

  int private_member;
  public:
    int a;
    static int bb; 
    int c;
};

class AA : public A{

};

namespace B{
 int t;
}
namespace C{
int t;
 class D{
   public:
    int a;
 };
 int u;

 namespace E{
  int nnC;
  int b;
 }
}

namespace F{
  int b;
  int bb;
}

extern "C"{
 void foo(){

  std::cout << "Interp1::foo " << std::endl;
  
  static int i = 0;
  std::cout << ++i << std::endl;
  
  //A a;
  //a.a = 9;
 }
}

