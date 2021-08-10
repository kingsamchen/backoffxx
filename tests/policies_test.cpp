// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#include "gtest/gtest.h"

#include "backoffxx/policies.h"

using namespace std::chrono_literals;

namespace {

TEST(Policy, ConstantBackoff) {
    backoffxx::policy::constant policy;
    constexpr auto base_delay = 1s;
    constexpr auto max_retries = 10u;
    for (auto i = 0u; i < max_retries; ++i) {
        EXPECT_EQ(policy.apply(base_delay, i), base_delay);
    }
}

TEST(Policy, LinearBackoffDelaySequence) {
    auto increment = 5s;
    backoffxx::policy::linear policy(increment);

    constexpr auto base_delay = 10s;
    constexpr auto max_retries = 5u;

    std::vector<std::chrono::milliseconds> delay_seq;
    std::generate_n(std::back_inserter(delay_seq), max_retries, [&]() {
        return policy.apply(base_delay, static_cast<uint32_t>(delay_seq.size()));
    });

    std::vector<std::chrono::milliseconds> expected_seq{10s, 15s, 20s, 25s, 30s};

    EXPECT_EQ(delay_seq, expected_seq);
}

TEST(Policy, LinearBackoffMaximumDelay) {
    auto increment = 15s;
    auto max_delay = 50s;
    backoffxx::policy::linear policy(increment, max_delay);

    constexpr auto base_delay = 10s;
    constexpr auto max_retries = std::numeric_limits<uint32_t>::max();
    std::vector<std::chrono::milliseconds> delay_seq;
    std::generate_n(std::back_inserter(delay_seq), 7, [&]() {
        return policy.apply(base_delay, static_cast<uint32_t>(delay_seq.size()));
    });

    std::vector<std::chrono::milliseconds> expected_seq{10s, 25s, 40s, 50s, 50s, 50s, 50s};
    EXPECT_EQ(delay_seq, expected_seq);
}

TEST(Policy, ExponentialBackoffDelaySequence) {
    constexpr auto base_delay = 4s;
    constexpr auto max_retries = 5u;
    backoffxx::policy::exponential policy;

    std::vector<std::chrono::milliseconds> delay_seq;
    std::generate_n(std::back_inserter(delay_seq), max_retries, [&]() {
        return policy.apply(base_delay, static_cast<uint32_t>(delay_seq.size()));
    });

    // max delay cut-off.
    std::vector<std::chrono::milliseconds> expected_seq{4s, 8s, 16s, 32s, 64s};

    EXPECT_EQ(delay_seq, expected_seq);
}

TEST(Policy, ExponentialBackoffMaximumDelay) {
    constexpr auto max_delay = 60s;
    backoffxx::policy::exponential policy(max_delay);

    constexpr auto base_delay = 8s;
    constexpr auto max_retries = 5u;

    std::vector<std::chrono::milliseconds> delay_seq;
    std::generate_n(std::back_inserter(delay_seq), max_retries, [&]() {
        return policy.apply(base_delay, static_cast<uint32_t>(delay_seq.size()));
    });

    // max delay cut-off.
    std::vector<std::chrono::milliseconds> expected_seq{8s, 16s, 32s, 60s, 60s};

    EXPECT_EQ(delay_seq, expected_seq);
}

TEST(Policy, FullJitter) {
    constexpr auto max_delay = 50s;
    backoffxx::policy::full_jitter policy(max_delay);

    constexpr auto base_delay = 8s;
    constexpr auto max_retries = 5u;
    for (auto i = 0u; i < max_retries; ++i) {
        auto bound = std::min(max_delay, base_delay * (1 << i));
        auto delay = policy.apply(base_delay, i);
        EXPECT_TRUE(0s <= delay);
        EXPECT_TRUE(delay <= bound);
    }
}

TEST(Policy, DecorrelatedJitter) {
    constexpr backoffxx::duration_type max_delay = 50s;
    backoffxx::policy::decorrelated_jitter policy(max_delay);

    constexpr auto base_delay = 8s;
    constexpr auto max_retries = 5u;

    ASSERT_LE(base_delay, max_delay);

    backoffxx::duration_type last_delay = base_delay;
    for (auto i = 0u; i < max_retries; ++i) {
        auto delay = policy.apply(base_delay, i);
        auto bound = std::min(last_delay * 3, max_delay);
        EXPECT_TRUE(base_delay <= delay);
        EXPECT_TRUE(delay <= bound);
        last_delay = delay;
    }
}

} // namespace
