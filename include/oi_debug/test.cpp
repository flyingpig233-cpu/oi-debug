#include "debug.hpp"
#include <map>
char tmp[1300001];

constexpr std::string_view a = "a";
constexpr std::string_view b = "b";

int main()
{
    using namespace oi_debug;
    println(TypeParseTraits<std::tuple<int, std::time_t, std::vector<std::map<std::pair<int, int>, int>>>>::get_name());
}
