#include "debug.hpp"

char tmp[1300001];

int main()
{
    using namespace oi_debug;
    println(TypeParseTraits<std::tuple<int, std::time_t, std::vector<std::map<std::pair<int, int>, int>>>>::get_name());
}
