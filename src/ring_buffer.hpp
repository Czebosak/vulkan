#pragma once

#include <span>

template <typename T>
class RingBuffer {
private:
    T* data_;
    size_t capacity;
    T* head_;
    T* tail_;

    constexpr size_t get_new_capacity() {
        return capacity * 2;
    }

    T* get_next(T* e) {
        T* end = data_ + capacity;

        T* next = (e + 1 != end) ? e + 1 : data_;

        return (next == tail_) ? nullptr : next;
    }

    T* get_next_or_grow() {
        T* next = get_next(head_);

        if (next == nullptr) {
            realloc(get_new_capacity());
            next = ++head_;
        }

        return next;
    }

    void realloc(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        
        std::copy(data_, data_ + capacity, new_data);

        head_ = head_ - data_ + new_data;
        tail_ = tail_ - data_ + new_data;

        delete[] data_;

        data_ = new_data;
        capacity = new_capacity;
    }
public:
    constexpr explicit RingBuffer() : data_(nullptr), head_(nullptr), tail_(nullptr) {}

    constexpr explicit RingBuffer(size_t capacity) : data_(new T[capacity]), capacity(capacity), head_(data_), tail_(data_) {}

    constexpr ~RingBuffer() {
        delete[] data_;
    }

    constexpr T& head() {
        return *head;
    }

    constexpr T& tail() {
        return *tail;
    }

    constexpr std::span<T> data() {
        return std::span(data_, capacity);
    }

    constexpr T& push_front(const T& value) {
        T* next = get_next_or_grow();
        *next = value;
        return *next;
    }

    constexpr T& push_front(T&& value) {
        T* next = get_next_or_grow();
        *next = std::move(value);
        return *next;
    }

    template<class... Args>
    constexpr T& emplace_back(Args&&... args) {
        T* next = get_next_or_grow();
        new (next) T(std::forward<Args>(args)...);
        return *next;
    }

    size_t size() const {
        return tail_ - data_;
    }

    void reserve(size_t size) {
        if (capacity) {}
    }
};
