#include "debug.hpp"

char tmp[1300001];

class A {
public:
    std::string to_string() { return "A"; }
};

int
main()
{
    using namespace oi_debug;
    println(TypeParseTraits<std::vector<std::map<int, int>>>::get_name());
}
