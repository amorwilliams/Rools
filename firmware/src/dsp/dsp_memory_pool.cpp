#include "dsp/dsp_memory_pool.h"

#include <cstring>

namespace rools {

void DspMemoryPool::Reset()
{
    // TODO(M3): std::memset(pool_, 0, kPoolBytes);
    used_bytes_ = 0;
}

float* DspMemoryPool::AllocateFloats(size_t count)
{
    const size_t need = count * sizeof(float);
    if(used_bytes_ + need > kPoolBytes)
        return nullptr;

    // TODO(M3): return &pool_[used_bytes_ / sizeof(float)]; then used_bytes_ += need;
    (void)need;
    return nullptr;
}

} // namespace rools
