#ifndef Header_Utility
#define Header_Utility
#include <android/log.h>
#include <string>
#include <sstream>

#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "SOUNDPROCESS", __VA_ARGS__)

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

/*template <class T, class ...Args>
typename std::enable_if
<
    !std::is_array<T>::value,
    std::unique_ptr<T>
>::type
make_unique(Args&& ...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename std::enable_if
<
    std::is_array<T>::value,
    std::unique_ptr<T>
>::type
make_unique(std::size_t n)
{
    typedef typename std::remove_extent<T>::type RT;
    return std::unique_ptr<T>(new RT[n]);
}*/

#endif