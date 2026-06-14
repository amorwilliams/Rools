#pragma once

#include "app_shell.h"

namespace rools {

class Gfx;

/**
 * 静态 App 注册表。
 *
 * 新增 App 时在 app_registry.cpp 中：
 *   1. static 实例
 *   2. Get() switch 加 case
 *   3. Count() 递增
 *   4. 若需显示，在 BindUi() 注入 Gfx
 */
class AppRegistry {
public:
    static size_t      Count();
    static App*        Get(size_t index);
    static const char* Name(size_t index);
    static App*        DefaultApp();
    static void        BindUi(Gfx* gfx);
};

} // namespace rools
