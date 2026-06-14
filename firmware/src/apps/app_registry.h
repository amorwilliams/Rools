#pragma once

#include "app_shell.h"

namespace rools {

class Gfx;

class AppRegistry {
public:
    static size_t      Count();
    static App*        Get(size_t index);
    static const char* Name(size_t index);
    static App*        DefaultApp();
    static void        BindUi(Gfx* gfx);
};

} // namespace rools
