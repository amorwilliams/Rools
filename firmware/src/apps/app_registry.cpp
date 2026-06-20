#include "apps/app_registry.h"

#include "apps/calibration_app.h"
#include "apps/display_test_app.h"
#include "apps/oscilloscope_app.h"
#include "apps/spectrum_app.h"
#include "display/gfx.h"

namespace rools {

static DisplayTestApp g_display_test;
static OscilloscopeApp g_oscilloscope;
static SpectrumApp    g_spectrum;
static CalibrationApp g_calibration;

void AppRegistry::BindUi(Gfx* gfx)
{
    g_display_test.Bind(gfx);
    g_oscilloscope.Bind(gfx);
    g_spectrum.Bind(gfx);
    g_calibration.Bind(gfx);
}

App* AppRegistry::Get(size_t index)
{
    switch(index)
    {
    case 0: return &g_display_test;
    case 1: return &g_oscilloscope;
    case 2: return &g_spectrum;
    case 3: return &g_calibration;
    default: return nullptr;
    }
}

size_t AppRegistry::Count()
{
    return 4;
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
