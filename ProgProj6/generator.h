#pragma once

#include <coroutine>
#include <iterator>
#include <exception>
#include <cmath> 

template<typename T>
struct Generator {
    struct promise_type {
        T current_value;
        T previous_value;
        bool is_first_yield = true;
        bool stop_coroutine = false;

        auto yield_value(T value) {
            current_value = value;

            if (is_first_yield) {
                is_first_yield = false;
            }
            else {
                if (std::abs(current_value - previous_value) < 16) {
                    stop_coroutine = true;
                }
            }
            previous_value = current_value;

            struct Awaiter {
                promise_type& p;

                bool await_ready() {
                    return p.stop_coroutine;
                }
                void await_suspend(std::coroutine_handle<>) {}
                void await_resume() {}
            };

            return Awaiter{ *this };
        }

        bool should_stop() const {
            return stop_coroutine;
        }

        auto get_return_object() { return Generator{ handle_type::from_promise(*this) }; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}

        using handle_type = std::coroutine_handle<promise_type>;
    };

    struct iterator {
        using handle_type = std::coroutine_handle<promise_type>;
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        handle_type h_;

        iterator(handle_type h) : h_(h) {}

        iterator& operator++() {
            h_.resume();
            if (h_.done()) {
                h_ = nullptr;
            }
            return *this;
        }
        void operator++(int) { ++(*this); }
        const T& operator*() const { return h_.promise().current_value; }
        T& operator*() { return h_.promise().current_value; }
        bool operator==(std::default_sentinel_t) const { return !h_ || h_.done(); }

        using handle_type = std::coroutine_handle<promise_type>;
    };

    iterator begin() {
        if (h_) {
            h_.resume();
            if (h_.done()) {
                return { nullptr };
            }
        }
        return { h_ };
    }
    std::default_sentinel_t end() { return {}; }

    Generator(std::coroutine_handle<promise_type> h) : h_(h) {}
    ~Generator() { if (h_) h_.destroy(); }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) noexcept : h_(other.h_) { other.h_ = nullptr; }
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (h_) h_.destroy();
            h_ = other.h_;
            other.h_ = nullptr;
        }
        return *this;
    }

    struct PromiseAccessor {
        promise_type* p_promise = nullptr;
        bool await_ready() noexcept { return false; }
        bool await_suspend(std::coroutine_handle<promise_type> h) noexcept {
            p_promise = &h.promise();
            return false;
        }
        promise_type& await_resume() noexcept { return *p_promise; }
    };

private:
    std::coroutine_handle<promise_type> h_;
};