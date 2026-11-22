#ifndef NANO_EDR_RING_BUFFER_H
#define NANO_EDR_RING_BUFFER_H

#include <cstddef>
#include <array>

#include "event.h"

namespace nano_edr {

template <typename T, size_t N>
class RingBuffer {
 public:
    // Обход в логическом порядке — от самого старого к самому новому.
    //
    // Итератор минимальный: ровно то, что нужно range-based for. Как он
    // добирается до элементов буфера, решаете вы; приватной части здесь нет
    // намеренно.
    class Iterator {
     public:
        Iterator(const RingBuffer<T, N>& link, size_t index): link_(link), index_(index) {};

        const T& operator*() const{
            return link_[index_];
        }

        Iterator& operator++(){
            index_++;
            return *this;
        }

        bool operator!=(const Iterator& other) const{
            return other.index_ != this->index_;
        }

     private:
        const RingBuffer<T, N>& link_;
        size_t index_;
    };

    // Дописывает элемент. Если буфер полон, самый старый затирается — окно,
    // а не архив.
    void PushBack(const T& value){
        arr_[next_] = value;
        next_++;
        if (next_ == N){
            next_ = 0;
            is_full_ = true;
        }
    }

    void Clear(){
        next_ = 0;
        is_full_ = false;
        arr_ = {};
    }

    std::size_t size() const{
        if (!is_full_) return next_;
        return N;
    }

    static constexpr std::size_t capacity(){
        return N;
    }

    bool empty() const{
        return !is_full_ && next_ == 0;
    }

    bool full() const{
        return is_full_;
    }

    // По логическому индексу: 0 — самый старый.
    const T& operator[](std::size_t index) const{
        index %= N;

        if (!is_full_){
            return arr_[index];
        }

        return arr_[(next_+index) % N];
    }

    // Самый новый элемент. Вызывать на пустом буфере нельзя — это ошибка
    // в коде вызывающего, а не случай, который надо обрабатывать.
    const T& back() const{
        if (next_ == 0) return arr_[N-1];
        return arr_[next_-1];
    }

    Iterator begin() const{
        return Iterator(*this, 0);
    }
    
    Iterator end() const{
        return Iterator(*this, size());
    }
 private:
    std::array<T, N> arr_;
    bool is_full_ = false;
    size_t next_ = 0;

};

}  // namespace nano_edr

#endif  // NANO_EDR_RING_BUFFER_H
