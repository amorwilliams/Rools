#pragma once

#include <cstddef>
#include <cstdint>

namespace rools {

/**
 * FX App 共享静态 RAM 池（Delay / Reverb 等）。
 *
 * 切换 App 时 AppShell 调用 Reset() 清零，避免尾音泄漏。
 * TODO(M3): 从 SDRAM 分配 kPoolBytes；实现 bump AllocateFloats()。
 */
class DspMemoryPool {
public:
    /** 最大 delay line：~1 s @ 48 kHz mono float（可按 M3 实测调整） */
    static constexpr size_t kMaxDelaySamples = 48000;
    static constexpr size_t kPoolBytes       = kMaxDelaySamples * sizeof(float);

    void Reset();

    /** 从池顶 bump 分配；不足时返回 nullptr。切换后需重新分配。 */
    float* AllocateFloats(size_t count);

    size_t bytes_used() const { return used_bytes_; }
    size_t bytes_capacity() const { return kPoolBytes; }

private:
    // TODO(M3): alignas(32) static float pool_[kMaxDelaySamples]; 或 SDRAM 指针
    size_t used_bytes_ = 0;
};

} // namespace rools
