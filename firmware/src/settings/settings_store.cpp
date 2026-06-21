#include "settings/settings_store.h"

#include "board/pwr_fail.h"

#include "per/qspi.h"
#include "sys/system.h"

namespace rools {

using daisy::QSPIHandle;
using daisy::System;

namespace {

constexpr uint32_t SlotAddress(uint32_t slot_index)
{
    return kSettingsBaseOffset + slot_index * kSectorSize;
}

// libDaisy PersistentStorage 旧单槽格式(用于一次性迁移)
enum class LegacyState : uint32_t {
    Unknown = 0,
    Factory = 1,
    User    = 2,
};

struct LegacySave {
    LegacyState    storage_state;
    GlobalSettings user_data;
};

} // namespace

bool GlobalSettings::operator==(const GlobalSettings& o) const
{
    if(magic != o.magic || last_app_index != o.last_app_index)
        return false;
    for(size_t i = 0; i < kCvChannelCount; ++i)
    {
        if(cv[i].center_volts != o.cv[i].center_volts
           || cv[i].attenuation != o.cv[i].attenuation)
            return false;
    }
    return true;
}

GlobalSettings MakeDefaultSettings()
{
    GlobalSettings s{};
    s.magic          = kSettingsMagic;
    s.last_app_index = 0;
    for(size_t i = 0; i < kCvChannelCount; ++i)
        s.cv[i] = kCvCalibrationDefault;
    return s;
}

SettingsStore& SettingsStore::Instance()
{
    static SettingsStore inst;
    return inst;
}

const SettingsSlot* SettingsStore::SlotAt(uint32_t slot_index) const
{
    if(!qspi_ || slot_index >= kRingSlots)
        return nullptr;
    return reinterpret_cast<const SettingsSlot*>(qspi_->GetData(SlotAddress(slot_index)));
}

bool SettingsStore::ScanRing()
{
    bool     found  = false;
    uint32_t best_i = 0;
    uint32_t best_s = 0;

    for(uint32_t i = 0; i < kRingSlots; ++i)
    {
        const SettingsSlot* slot = SlotAt(i);
        if(!slot)
            continue;
        if(slot->slot_magic != kSlotMagic || slot->state != kSlotStateValid)
            continue;
        if(!found || slot->sequence >= best_s)
        {
            found  = true;
            best_i = i;
            best_s = slot->sequence;
        }
    }

    if(!found)
        return false;

    const SettingsSlot* slot = SlotAt(best_i);
    settings_                = slot->data;
    persisted_               = slot->data;
    active_slot_             = best_i;
    active_sequence_         = best_s;
    return true;
}

bool SettingsStore::TryLegacyMigrate()
{
    if(!qspi_)
        return false;

    const auto* legacy = reinterpret_cast<const LegacySave*>(qspi_->GetData(kSettingsBaseOffset));
    const auto  state  = legacy->storage_state;
    if(state != LegacyState::Factory && state != LegacyState::User)
        return false;

    settings_  = legacy->user_data;
    persisted_ = settings_;
    if(settings_.magic != kSettingsMagic)
    {
        settings_  = MakeDefaultSettings();
        persisted_ = settings_;
    }
    return true;
}

bool SettingsStore::WriteSlot(uint32_t slot_index, uint32_t sequence)
{
    if(!qspi_ || slot_index >= kRingSlots)
        return false;

    SettingsSlot slot{};
    slot.slot_magic = kSlotMagic;
    slot.sequence   = sequence;
    slot.state      = kSlotStateValid;
    slot.data       = settings_;

    const uint32_t addr = SlotAddress(slot_index);
    if(qspi_->EraseSector(addr) != QSPIHandle::OK)
        return false;
    return qspi_->Write(addr, sizeof(slot), reinterpret_cast<uint8_t*>(&slot)) == QSPIHandle::OK;
}

void SettingsStore::Init(QSPIHandle& qspi)
{
    qspi_ = &qspi;

    const GlobalSettings defaults = MakeDefaultSettings();
    bool                 loaded   = ScanRing();

    if(!loaded)
    {
        if(TryLegacyMigrate())
            loaded = false; // 迁移成功但需写入环
        else
        {
            settings_  = defaults;
            persisted_ = defaults;
        }
        active_slot_     = 0;
        active_sequence_ = 0;
        WriteSlot(0, 1);
        active_slot_     = 0;
        active_sequence_ = 1;
        persisted_       = settings_;
    }

    if(settings_.magic != kSettingsMagic)
    {
        settings_  = defaults;
        persisted_ = settings_;
        dirty_     = true;
        Flush();
    }

    inited_         = true;
    dirty_          = false;
    last_change_ms_ = System::GetNow();
}

GlobalSettings& SettingsStore::Get()
{
    return settings_;
}

void SettingsStore::MarkDirty()
{
    dirty_          = true;
    last_change_ms_ = System::GetNow();
}

void SettingsStore::Tick(uint32_t now)
{
    if(!inited_ || !dirty_)
        return;
    if(now - last_change_ms_ >= kFlushDebounceMs)
        Flush();
}

void SettingsStore::Flush()
{
    if(!inited_ || !qspi_)
        return;

    if(settings_ == persisted_)
    {
        dirty_ = false;
        return;
    }

    const uint32_t next_slot = active_sequence_ == 0 ? 0 : (active_slot_ + 1) % kRingSlots;
    const uint32_t next_seq  = active_sequence_ == 0 ? 1 : active_sequence_ + 1;

    if(!WriteSlot(next_slot, next_seq))
        return;

    active_slot_     = next_slot;
    active_sequence_ = next_seq;
    persisted_       = settings_;
    dirty_           = false;
}

bool SettingsStore::HasPowerFailHardware()
{
    return true;
}

void SettingsStore::InitPowerFailDetection()
{
    PwrFail::Init();
}

void SettingsStore::OnPowerFail()
{
    Instance().Flush();
}

} // namespace rools
