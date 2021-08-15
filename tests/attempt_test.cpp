// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#include <chrono>

#include "gtest/gtest.h"

#include "backoffxx/attempt.h"

using namespace std::chrono_literals;

using attempt_rc = backoffxx::attempt_rc;

namespace {

TEST(Attempt, SucceedWithoutRetry) {
    constexpr auto delay = 4s;
    constexpr auto max_retries = 3u;
    int attempt = 0;
    auto result = backoffxx::attempt(backoffxx::make_constant(delay, max_retries), [&attempt] {
        ++attempt;
        return attempt_rc::success;
    });

    EXPECT_TRUE(result.ok());
    EXPECT_EQ(attempt, 1);
}

TEST(Attempt, SucceedAfterRetry) {
    constexpr auto delay = 200ms;
    constexpr auto max_retries = 5u;
    int attempt = 0;
    auto result = backoffxx::attempt(backoffxx::make_exponential(delay, max_retries), [&attempt] {
        return ++attempt == 3 ? attempt_rc::success
                              : attempt_rc::failure;
    });

    EXPECT_TRUE(result.ok());
    EXPECT_EQ(attempt, 3);
}

TEST(Attempt, RetryExhausted) {
    auto result = backoffxx::attempt(backoffxx::make_decorrelated_jitter(1s, 3, 5s),
                                     [] {
                                         return attempt_rc::failure;
                                     });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), attempt_rc::failure);
}

TEST(Attempt, AbortDueToHardError) {
    int attempt = 0;
    auto result = backoffxx::attempt(
        backoffxx::make_decorrelated_jitter(100ms, 3, 1s),
        [&attempt] {
            return attempt++ == 0 ? attempt_rc::failure
                                  : attempt_rc::hard_error;
        });

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), attempt_rc::hard_error);
    EXPECT_EQ(attempt, 2);
}

TEST(Attempt, ReuseBackoff) {
    auto backoff = backoffxx::make_constant(500ms, 3u);
    auto result = backoffxx::attempt(backoff, [] {
        return attempt_rc::failure;
    });
    ASSERT_FALSE(result.ok());
    ASSERT_EQ(result.code(), attempt_rc::failure);
    ASSERT_FALSE(backoff.next_delay().has_value());

    backoff.reset();

    size_t count = 0;
    result = backoffxx::attempt(backoff, [&count] {
        ++count;
        return attempt_rc::failure;
    });

    // 1 execution + 3 retries
    EXPECT_EQ(count, 4);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), attempt_rc::failure);
}

} // namespace
