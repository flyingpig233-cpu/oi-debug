#include "debug.hpp"

char tmp[1300001];

int main()
{
    using namespace oi_debug;
    std::vector<std::map<int, int>> v {
      std::map<int, int>{{1, 2}},
      std::map<int, int>{{3, 5}},
    };
    debug_with_type(v);
}
