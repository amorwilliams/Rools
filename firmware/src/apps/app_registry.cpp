#include "apps/app_registry.h"

#include "apps/spectrum_app.h"
#include "display/gfx.h"

namespace rools {

// 每个 App 一个 static 实例，生命周期贯穿固件运行期
static SpectrumApp g_spectrum;

void AppRegistry::BindUi(Gfx* gfx)
{
    // 需要绘图的 App 在此绑定 Gfx；无 UI 的 App 可省略
    g_spectrum.Bind(gfx);
}

App* AppRegistry::Get(size_t index)
{
    switch(index)
    {
    case 0: return &g_spectrum;
    default: return nullptr;
    }
}

size_t AppRegistry::Count()
{
    return 1;
}

const char* AppRegistry::Name(size_t index)
{
    App* app = Get(index);
    return app ? app->name() : "";
}

App* AppRegistry::DefaultApp()
{
    return &g_spectrum;
}

} // namespace rools
