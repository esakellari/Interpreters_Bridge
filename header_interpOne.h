namespace mynamespace{
  int a;
  namespace mynamespace_embedded{
    int c;
  }
  int b;
}
namespace mynamespace_embedded{
  int c;
}
namespace mynamespace2{}

int evaluateMe = mynamespace::a;
