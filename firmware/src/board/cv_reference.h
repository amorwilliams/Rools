#pragma once

namespace rools {

struct CvReferenceConfig {
    float adc_ref_volts;
    float center_volts;
    float gain_norm;
    bool  invert;
};

// TL072: Vin -> Vadc = center - atten*Vin (invert=true in norm domain)
// Measured (grounds tied): 0V->uni 0.332, +-5V fit -> center 1.096V, atten 0.1043
constexpr float kCvAdcRefVolts     = 3.3f;
constexpr float kCvHwCenterVolts   = 1.096f;
constexpr float kCvHwAttenuation   = 0.1043f;
constexpr float kCvFullScaleVolts  = 10.0f;
constexpr float kCvGainNorm
    = 1.0f / ((kCvHwAttenuation * kCvFullScaleVolts) / kCvAdcRefVolts); // 3.3f, norm 1.0 = ±10V
constexpr float kAudioFullScaleVolts = 1.8f; // Seed line level; jack ±10V via TL074 ×0.1/×10

extern const CvReferenceConfig kCvReferenceDefault;

float ClampBipolar(float v);
float CvAdcToNormalizedWithCenterNorm(float adc_uni, float center_norm, float gain_norm, bool invert);
float CvAdcToNormalized(float adc_uni);
float CvAdcToNormalized(float adc_uni, const CvReferenceConfig& cfg);
float CvNormalizedToVolts(float normalized, float full_scale_volts);
float CvAdcUniToEurorackVolts(float adc_uni);
float ColumnSumNormalized(float cv_norm, float knob_uni);

} // namespace rools
