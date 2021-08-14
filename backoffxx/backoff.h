// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#ifndef BACKOFFXX_BACKOFF_H_
#define BACKOFFXX_BACKOFF_H_

#include <cstdint>
#include <optional>
#include <type_traits>

#include "backoffxx/policies.h"

namespace backoffxx {

template<typename Policy, typename = void>
struct is_valid_policy : std::false_type {};

template<typename Policy>
struct is_valid_policy<
    Policy,
    std::void_t<
        std::enable_if_t<
            std::is_convertible_v<
                decltype(std::declval<Policy>().apply(duration_type{}, uint32_t{})),
                duration_type>>>>
    : std::true_type {};

template<typename Policy>
class backoff : Policy {
public:
    static_assert(is_valid_policy<Policy>::value,
                  "Policy should have a method apply whose signature meets requirements");

    backoff(duration_type base, uint32_t max_retries)
        : base_(base),
          max_retries_(max_retries) {}

    backoff(duration_type base, uint32_t max_retries, const Policy& policy)
        : Policy(policy),
          base_(base),
          max_retries_(max_retries) {}

    backoff(const backoff&) = default;

    backoff(backoff&&) = default;

    backoff& operator=(const backoff&) = default;

    backoff& operator=(backoff&&) = default;

    ~backoff() = default;

    std::optional<duration_type> next_delay() {
        if (done_retries_ == max_retries_) {
            return std::nullopt;
        }

        return std::optional<duration_type>(this->apply(base_, done_retries_++));
    }

    void reset() noexcept {
        done_retries_ = 0;
    }

private:
    duration_type base_;
    uint32_t max_retries_;
    uint32_t done_retries_ = 0u;
};

inline auto make_constant(const duration_type& delay, uint32_t max_retries) {
    return backoff<policy::constant>(delay, max_retries);
}

inline auto make_linear(const duration_type& base,
                        uint32_t max_retries,
                        const duration_type& increment) {
    return backoff<policy::linear>(base, max_retries, policy::linear(increment));
}

inline auto make_linear(const duration_type& base,
                        uint32_t max_retires,
                        const duration_type& increment,
                        const duration_type& max_delay) {
    return backoff<policy::linear>(base, max_retires, policy::linear(increment, max_delay));
}

inline auto make_exponential(const duration_type& base, uint32_t max_retries) {
    return backoff<policy::exponential>(base, max_retries);
}

inline auto make_exponential(const duration_type& base,
                             uint32_t max_retries,
                             const duration_type& max_delay) {
    return backoff<policy::exponential>(base, max_retries, policy::exponential(max_delay));
}

inline auto make_full_jitter(const duration_type& base, uint32_t max_retries) {
    return backoff<policy::full_jitter>(base, max_retries);
}

inline auto make_full_jitter(const duration_type& base,
                             uint32_t max_retries,
                             const duration_type& max_delay) {
    return backoff<policy::full_jitter>(base, max_retries, policy::full_jitter(max_delay));
}

inline auto make_decorrelated_jitter(const duration_type& base, uint32_t max_retries) {
    return backoff<policy::decorrelated_jitter>(base, max_retries);
}

inline auto make_decorrelated_jitter(const duration_type& base,
                                     uint32_t max_retries,
                                     const duration_type& max_delay) {
    return backoff<policy::decorrelated_jitter>(base,
                                                max_retries,
                                                policy::decorrelated_jitter(max_delay));
}

} // namespace backoffxx

#endif // BACKOFFXX_BACKOFF_H_
