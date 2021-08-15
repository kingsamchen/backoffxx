// Copyright (c) 2021 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#ifndef BACKOFFXX_ATTEMPT_H_
#define BACKOFFXX_ATTEMPT_H_

#include <thread>
#include <type_traits>

#include "backoffxx/backoff.h"

namespace backoffxx {

enum class attempt_rc {
    success,
    failure,
    hard_error // non-retryable
};

class attempt_result {
public:
    // implicit
    attempt_result(attempt_rc code)
        : code_(code) {}

    attempt_rc code() const noexcept {
        return code_;
    }

    bool ok() const noexcept {
        return code_ == attempt_rc::success;
    }

private:
    attempt_rc code_;
};

// `attempt_rc::success` indicates executing `fn` in success, maybe after retries.
// `attempt_rc::failure` indicates retries of backoff exhausted.
// `attempt_rc::hard_error` indicates executing `fn` ends with a non-retryable failure.

template<typename Policy, typename F>
attempt_result attempt(backoff<Policy>& backoff, F&& fn) {
    static_assert(std::is_convertible_v<std::invoke_result_t<F>, attempt_result>,
                  "The invoke result of F should be convertible to attempt_result");
    while (true) {
        // Either attempt_rc::success or attempt_rc::hard_error
        if (attempt_result result = fn(); result.code() != attempt_rc::failure) {
            return result;
        }

        auto delay = backoff.next_delay();
        // Retries exhausted.
        if (!delay) {
            return attempt_rc::failure;
        }

        std::this_thread::sleep_for(*delay);
    }
}

template<typename Policy, typename F>
attempt_result attempt(backoff<Policy>&& backoff, F&& fn) {
    return attempt(backoff, std::forward<F>(fn));
}

} // namespace backoffxx

#endif // BACKOFFXX_ATTEMPT_H_
