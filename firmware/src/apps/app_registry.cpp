#include "apps/app_registry.h"

#include "apps/display_test_app.h"
#include "apps/spectrum_app.h"
#include "display/gfx.h"

namespace rools {

static DisplayTestApp g_display_test;
static SpectrumApp    g_spectrum;

void AppRegistry::BindUi(Gfx* gfx)
{
    g_display_test.Bind(gfx);
    g_spectrum.Bind(gfx);
}

App* AppRegistry::Get(size_t index)
{
    switch(index)
    {
    case 0: return &g_display_test;
    case 1: return &g_spectrum;
    default: return nullptr;
    }
}

size_t AppRegistry::Count()
{
    return 2;
}

const char* AppRegistry::Name(size_t index)
{
    App* app = Get(index);
    return app ? app->name() : "";
}

App* AppRegistry::DefaultApp()
{
    return &g_display_test;
}

} // namespace rools
