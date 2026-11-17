#pragma once

#include <cstdint>

namespace nx::nvn {

class PresentCadence {
public:
    // Keep a full game period in reserve on startup. Preserve the deadline sequence across isolated misses.
    // Restart on a long stall. If the FIFO already has two periods of lead, do not extend its deadline.
    uint64_t next(uint64_t now, uint64_t refresh, uint64_t period, uint64_t last_shown) {
        if (period != period_ || refresh != refresh_ || !due_ ||
            (now > due_ && now - due_ >= 2 * period)) {
            period_ = period;
            refresh_ = refresh;
            due_ = now + period;
            if (last_shown && last_shown <= now && now - last_shown < 4 * period) {
                const uint64_t phase = (due_ - last_shown) % refresh;
                if (phase) due_ += refresh - phase;
            }
        }
        if (due_ > now && due_ - now > 2 * period) return due_ - period - refresh / 4;
        const uint64_t current = due_;
        due_ += period;
        return current - refresh / 4;
    }

    void reset() { due_ = 0; }

private:
    uint64_t due_ = 0;
    uint64_t period_ = 0;
    uint64_t refresh_ = 0;
};

}  // namespace nx::nvn
