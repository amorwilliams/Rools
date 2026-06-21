#pragma once

#include <cstddef>
#include <cstdint>

#include "board/cv_calibration.h"

namespace daisy {
class QSPIHandle;
}

namespace rools {

// schema 版本;结构变更时递增,旧数据自动回落默认
constexpr uint32_t kSettingsMagic        = 0x524F4C31; // 'ROL1'
constexpr size_t   kCvChannelCount       = 4;
constexpr uint32_t kFlushDebounceMs      = 2000;

// QSPI 环形缓冲:10 槽 x 4KB ≈ 百万次级擦写寿命
constexpr uint32_t kSectorSize           = 4096;
constexpr uint32_t kRingSlots            = 10;
constexpr uint32_t kSettingsBaseOffset   = 0;
constexpr uint32_t kSlotMagic            = 0x524F4C53; // 'ROLS'
constexpr uint32_t kSlotStateEmpty       = 0;
constexpr uint32_t kSlotStateValid       = 1;

/** 全局持久化设置聚合体。必须紧凑、无指针、可平凡比较。 */
struct GlobalSettings {
    uint32_t      magic;
    uint32_t      last_app_index;
    CvCalibration cv[kCvChannelCount];
    // 预留: 各 app/菜单参数块作为强类型成员挂载于此

    bool operator==(const GlobalSettings& o) const;
    bool operator!=(const GlobalSettings& o) const { return !(*this == o); }
};

/** 单槽 Flash 记录(写在每 4KB 扇区首) */
struct SettingsSlot {
    uint32_t      slot_magic;
    uint32_t      sequence;
    uint32_t      state;
    uint32_t      reserved;
    GlobalSettings data;
};

GlobalSettings MakeDefaultSettings();

/**
 * 统一设置层:QSPI 环形缓冲 + dirty 节流落盘。
 * Erase/Write 仅可在主循环或掉电 EXTI 上下文调用,绝不在 audio ISR。
 */
class SettingsStore {
public:
    static SettingsStore& Instance();

    void            Init(daisy::QSPIHandle& qspi);
    GlobalSettings& Get();

    void MarkDirty();
    void Tick(uint32_t now);
    void Flush();

    // 掉电检测：D28 1 kHz debounce → EmergencyBacklightOff → OnPowerFail
    static bool HasPowerFailHardware();
    static void InitPowerFailDetection();
    static void OnPowerFail(); // debounce 确认后 Flush；禁止 SPI

private:
    SettingsStore() = default;

    bool ScanRing();
    bool TryLegacyMigrate();
    bool WriteSlot(uint32_t slot_index, uint32_t sequence);
    const SettingsSlot* SlotAt(uint32_t slot_index) const;

    daisy::QSPIHandle* qspi_            = nullptr;
    GlobalSettings     settings_{};
    GlobalSettings     persisted_{};
    uint32_t           active_slot_     = 0;
    uint32_t           active_sequence_ = 0;
    bool               inited_          = false;
    bool               dirty_           = false;
    uint32_t           last_change_ms_  = 0;
};

} // namespace rools
