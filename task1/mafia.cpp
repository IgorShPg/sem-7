#include <compare>
#include <cstddef>
#include <type_traits>
#include <iostream>
#include <memory>
#include <cassert>

namespace mafia {
    template<class T> class shared_ptr;
}

template<class T>
class mafia::shared_ptr {
    T* ptr;        
    long* count;
public:
    void one_more();
    void reset();
    shared_ptr(std::nullptr_t = nullptr) noexcept;
    ~shared_ptr();
    shared_ptr(const shared_ptr&) noexcept;
    shared_ptr(shared_ptr&&) noexcept;
    explicit shared_ptr(T*);
    long use_count() const noexcept;
    bool operator==(const shared_ptr&) const noexcept;
    bool operator==(std::nullptr_t) const noexcept;
    bool operator!=(const shared_ptr& obj) const { return ptr != obj.ptr; }
    void swap(shared_ptr&) noexcept;
    T* get() const noexcept;
    
};

template<class T>mafia::shared_ptr<T>::shared_ptr(std::nullptr_t) noexcept: ptr(nullptr), count(nullptr) {}

template<class T>
mafia::shared_ptr<T>::shared_ptr(const shared_ptr &obj) noexcept
    : ptr(obj.ptr)
    , count(obj.count)
{
    one_more();
}


template<class T>
mafia::shared_ptr<T>::shared_ptr(shared_ptr &&obj) noexcept
    : ptr(obj.ptr)
    , count(obj.count)
{
    obj.ptr = nullptr;
    obj.count = nullptr;
}


template<class T>
void mafia::shared_ptr<T>::reset() {
    if ((count && --(*count))==0) {
        delete count;
        delete ptr;
    }
    ptr = nullptr;
    count = nullptr;
}

template<class T>
mafia::shared_ptr<T>::shared_ptr(T *p)
    : ptr(p)
    , count(nullptr)
{
    try {
        one_more();
    } catch (...) {
        reset();
        throw;
    }
}


template<class T>
mafia::shared_ptr<T>::~shared_ptr() {
    reset();
}

template<class T>
long mafia::shared_ptr<T>::use_count() const noexcept {
    return count ? *count : 0;
}

template <typename T>
T* mafia::shared_ptr<T>::get() const noexcept { 
    return ptr;
}

template<class T>
void mafia::shared_ptr<T>::one_more(void) {
    if (ptr) {
        if (!count) {
            count = new long();
        }
        ++*count;
    }
}

template<class T>
void mafia::shared_ptr<T>::swap(shared_ptr &obj) noexcept {
    T *ptr_tmp = ptr;
    ptr = obj.ptr;
    obj.ptr = ptr_tmp;
    long *count_tmp = count;
    count = obj.count;
    obj.count = count_tmp;
}

template<class T>
bool mafia::shared_ptr<T>::operator==(const shared_ptr &obj) const noexcept {
    return ptr == obj.ptr;
}

template<class T>
bool mafia::shared_ptr<T>::operator==(std::nullptr_t) const noexcept {
    return !ptr;
}












