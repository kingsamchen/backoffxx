// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#include <chrono>

#include "gtest/gtest.h"

#include "backoffxx/backoff.h"
#include "backoffxx/policies.h"

using namespace std::chrono_literals;

using duration_type = backoffxx::duration_type;

using constant_backoff = backoffxx::backoff<backoffxx::policy::constant>;
using linear_backoff = backoffxx::backoff<backoffxx::policy::linear>;

namespace {

TEST(Backoff, WhenMaximumRetriesReached) {
    constexpr uint32_t max_retries = 5;
    constant_backoff backoff(3s, max_retries);
    for (auto i = 0u; i < max_retries; ++i) {
        auto delay = backoff.next_delay();
        EXPECT_TRUE(delay.has_value());
    }

    auto delay = backoff.next_delay();
    EXPECT_FALSE(delay.has_value());
    delay = backoff.next_delay();
    EXPECT_FALSE(delay.has_value());
}

TEST(Backoff, ResetBackoff) {
    constexpr uint32_t max_retries = 5;
    constant_backoff backoff(3s, max_retries);
    for (auto i = 0u; i < max_retries; ++i) {
        auto delay = backoff.next_delay();
        ASSERT_TRUE(delay.has_value());
    }

    ASSERT_FALSE(backoff.next_delay().has_value());

    backoff.reset();
    for (auto i = 0u; i < max_retries; ++i) {
        auto delay = backoff.next_delay();
        ASSERT_TRUE(delay.has_value());
    }

    ASSERT_FALSE(backoff.next_delay().has_value());
}

TEST(Backoff, PolicyEBO) {
    constexpr size_t base_size = sizeof(backoffxx::duration_type) + sizeof(uint32_t) * 2;
    EXPECT_EQ(base_size, sizeof(constant_backoff));
    EXPECT_EQ(base_size + sizeof(backoffxx::policy::linear), sizeof(linear_backoff));
}

TEST(Backoff, PolicyTraitValidation) {
    {
        struct p {
            duration_type not_apply(const duration_type&, uint32_t) {
                return {};
            }
        };
        EXPECT_FALSE(backoffxx::is_valid_policy<p>::value);
    }

    {
        struct p {
            int apply(const duration_type&, uint32_t);
        };
        EXPECT_FALSE(backoffxx::is_valid_policy<p>::value);
    }

    {
        struct p {
            duration_type apply(const duration_type&) {
                return {};
            }
        };
        EXPECT_FALSE(backoffxx::is_valid_policy<p>::value);
    }

    {
        struct p {
            duration_type apply(const duration_type&, uint32_t, uint32_t);
        };
        EXPECT_FALSE(backoffxx::is_valid_policy<p>::value);
    }
}

} // namespace
