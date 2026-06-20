#pragma once

#include <cstddef>

#include "board/cv_reference.h"

namespace rools {

/** 单通道 CV 前端线性标定:Vadc = center_volts - attenuation*Vin */
struct CvCalibration {
    float center_volts; // ADC 节点在 Vin=0 时的电压
    float attenuation;  // dVadc/dVin 的幅值
};

// 共地后实测默认值(4 路相同)
constexpr CvCalibration kCvCalibrationDefault{1.096f, 0.1043f};

/** 由标定推出运行时 CvReferenceConfig(invert 域增益) */
CvReferenceConfig CvCalToConfig(const CvCalibration& cal);

// 运行时 CV 通道接口(实现于 app_shell.cpp,依赖 hw / SettingsStore)

/** 校准用:读取指定 CV 通道的实时 ADC 原始值(0..1)。 */
float GetCvAdcRaw(size_t ch);

/** 写入指定 CV 通道标定(更新运行时配置 + 设置层并置 dirty)。 */
void SetCvChannelCal(size_t ch, const CvCalibration& cal);

/** 读取指定 CV 通道当前标定。 */
const CvCalibration& GetCvChannelCal(size_t ch);

} // namespace rools
