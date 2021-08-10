// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#ifndef BACKOFFXX_POLICIES_H_
#define BACKOFFXX_POLICIES_H_

#include <chrono>
#include <random>

namespace backoffxx {

using duration_type = std::chrono::milliseconds;

namespace details {

[[nodiscard]]inline unsigned seed() {
    return std::random_device{}();
}

} // namespace details

namespace policy {

class constant {
public:
    duration_type apply(const duration_type& base, uint32_t /*done_retries*/) const {
        return base;
    }
};

class linear {
public:
    explicit linear(const duration_type& incr)
        : increment_(incr),
          max_dur_(duration_type::max()) {}

    linear(const duration_type& incr, const duration_type& max_dur)
        : increment_(incr),
          max_dur_(max_dur) {}

    duration_type apply(const duration_type& base, uint32_t done_retries) const {
        return std::min(base + done_retries * increment_, max_dur_);
    }

private:
    duration_type increment_;
    duration_type max_dur_;
};

class exponential {
public:
    exponential()
        : max_dur_(duration_type::max()) {}

    explicit exponential(const duration_type& max_dur)
        : max_dur_(max_dur) {}

    duration_type apply(const duration_type& base, uint32_t done_retries) const {
        return std::min(base * (1 << done_retries), max_dur_);
    }

private:
    duration_type max_dur_;
};

// --- jitters ---

// rand(a, b) would generate an integer x, such that a <= x <= b, i.e. both sides included.
// For jitter algorithms, see
// https://github.com/aws-samples/aws-arch-backoff-simulator/blob/master/src/backoff_simulator.py

// rand(0, min(cap, base * 2^t))
class full_jitter {
public:
    full_jitter()
        : full_jitter(duration_type::max()) {}

    explicit full_jitter(const duration_type& max_dur)
        : engine_(details::seed()),
          max_dur_(max_dur) {}

    auto apply(const duration_type& base, uint32_t done_retries) {
        auto bound = std::min(base * (1 << done_retries), max_dur_);
        decltype(rnd_)::param_type range(duration_type{}.count(), bound.count());
        return duration_type(rnd_(engine_, range));
    }

private:
    std::default_random_engine engine_;
    std::uniform_int_distribution<duration_type::rep> rnd_;
    duration_type max_dur_;
};

// duration = min(cap, rand(base, duration * 3))
class decorrelated_jitter {
public:
    decorrelated_jitter()
        : decorrelated_jitter(duration_type::max()) {}

    explicit decorrelated_jitter(const duration_type& max_dur)
        : engine_(details::seed()),
          max_dur_(max_dur),
          last_dur_{} {}

    auto apply(const duration_type& base, uint32_t /*done_retries*/) {
        if (last_dur_.count() == 0) {
            last_dur_ = base;
        }

        auto bound = last_dur_ * 3;
        decltype(rnd_)::param_type range(base.count(), bound.count());
        duration_type final = std::min(duration_type(rnd_(engine_, range)), max_dur_);

        last_dur_ = final;
        return final;
    }

private:
    std::default_random_engine engine_;
    std::uniform_int_distribution<duration_type::rep> rnd_;
    duration_type max_dur_;
    duration_type last_dur_;
};

} // namespace policy
} // namespace backoffxx

#endif // BACKOFFXX_POLICIES_H_
