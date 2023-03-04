#include "debug.hpp"
#include <map>
int tmp[13];

class A {
public:
    std::string to_string() const { return "A"; }
};

int main()
{
    using namespace oi_debug;
    std::list<int> l;
    l.push_back(1);
    l.push_back(2);
    debug(a);
}
