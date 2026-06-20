#include "board/cv_reference.h"

namespace rools {

const CvReferenceConfig kCvReferenceDefault{
    kCvAdcRefVolts,
    kCvHwCenterVolts,
    kCvGainNorm, // 3.3f, ±10V full scale
    true,
};

float ClampBipolar(float v)
{
    if(v < -1.0f)
        return -1.0f;
    if(v > 1.0f)
        return 1.0f;
    return v;
}

float CvAdcUniToEurorackVolts(float adc_uni)
{
    const float v_adc = adc_uni * kCvAdcRefVolts;
    return (kCvHwCenterVolts - v_adc) / kCvHwAttenuation;
}

float CvAdcToNormalizedWithCenterNorm(float adc_uni, float center_norm, float gain_norm, bool invert)
{
    float norm = (adc_uni - center_norm) * gain_norm;
    if(invert)
        norm = -norm;
    return ClampBipolar(norm);
}

float CvAdcToNormalized(float adc_uni, const CvReferenceConfig& cfg)
{
    const float center_norm = cfg.center_volts / cfg.adc_ref_volts;
    return CvAdcToNormalizedWithCenterNorm(adc_uni, center_norm, cfg.gain_norm, cfg.invert);
}

float CvAdcToNormalized(float adc_uni)
{
    return CvAdcToNormalized(adc_uni, kCvReferenceDefault);
}

float CvNormalizedToVolts(float normalized, float full_scale_volts)
{
    return normalized * full_scale_volts;
}

float ColumnSumNormalized(float cv_norm, float knob_uni)
{
    const float knob_bip = (knob_uni - 0.5f) * 2.f;
    return ClampBipolar(cv_norm + knob_bip);
}

} // namespace rools
