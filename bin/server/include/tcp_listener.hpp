#pragma once

#include <string>
#include <functional>

#ifdef _WIN32
    #include "tcp_listener_win32.hpp"
#elif defined(_POSIX_VERSION)
    #include "tcp_listener_posix.hpp"
#endif
