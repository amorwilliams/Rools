#include "board/cv_calibration.h"

namespace rools {

CvReferenceConfig CvCalToConfig(const CvCalibration& cal)
{
    const float gain = kCvAdcRefVolts / (cal.attenuation * kCvFullScaleVolts);
    return CvReferenceConfig{kCvAdcRefVolts, cal.center_volts, gain, true};
}

} // namespace rools
