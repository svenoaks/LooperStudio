#include <string>
#include <string>
#include <sstream>

namespace std {
    template <typename T>
    std::string to_string(T value);
}

template <typename T>
std::string std::to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}
