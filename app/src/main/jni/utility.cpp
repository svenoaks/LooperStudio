#include <string>
#include <sstream>
#include "utility.h"

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}