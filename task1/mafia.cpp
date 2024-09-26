#include <iostream>
#include <map>
#include <memory>

namespace mafia {
    template<typename T>
    class shared_ptr;
}

template<typename T>
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
    shared_ptr& operator=(const shared_ptr& obj) noexcept;
    shared_ptr& operator=(shared_ptr&& obj) noexcept;
    T* get() const noexcept;
};

template<typename T>
mafia::shared_ptr<T>::shared_ptr(std::nullptr_t) noexcept : ptr(nullptr), count(nullptr) {}

template<typename T>
mafia::shared_ptr<T>::shared_ptr(const shared_ptr &obj) noexcept
    : ptr(obj.ptr)
    , count(obj.count)
{
    one_more();
}

template<typename T>
mafia::shared_ptr<T>::shared_ptr(shared_ptr &&obj) noexcept
    : ptr(obj.ptr)
    , count(obj.count)
{
    obj.ptr = nullptr;
    obj.count = nullptr;
}

template<typename T>
void mafia::shared_ptr<T>::reset() {
    if (count && --(*count) == 0) {
        delete count;
        delete ptr;
    }
    ptr = nullptr;
    count = nullptr;
}

template<typename T>
mafia::shared_ptr<T>::shared_ptr(T *p)
    : ptr(p)
    , count(new long(1))
{}

template<typename T>
mafia::shared_ptr<T>::~shared_ptr() {
    reset();
}

template<typename T>
long mafia::shared_ptr<T>::use_count() const noexcept {
    return count ? *count : 0;
}

template<typename T>
T* mafia::shared_ptr<T>::get() const noexcept { 
    return ptr;
}

template<typename T>
void mafia::shared_ptr<T>::one_more() {
    if (ptr) {
        if (!count) {
            count = new long(1);
        } else {
            ++(*count);
        }
    }
}

template<typename T>
void mafia::shared_ptr<T>::swap(shared_ptr &obj) noexcept {
    std::swap(ptr, obj.ptr);
    std::swap(count, obj.count);
}

template<typename T>
bool mafia::shared_ptr<T>::operator==(const shared_ptr &obj) const noexcept {
    return ptr == obj.ptr;
}

template<typename T>
bool mafia::shared_ptr<T>::operator==(std::nullptr_t) const noexcept {
    return !ptr;
}

template<typename T>
mafia::shared_ptr<T>& mafia::shared_ptr<T>::operator=(const shared_ptr &obj) noexcept {
    if (this != &obj) {
        if (--(*count) == 0) {
            delete ptr;
            delete count;
        }
        ptr = obj.ptr;
        count = obj.count;
        ++(*count);
    }
    return *this;
}

template<typename T>
mafia::shared_ptr<T>& mafia::shared_ptr<T>::operator=(shared_ptr &&obj) noexcept {
    if (this != &obj) {
        reset();
        ptr = obj.ptr;
        count = obj.count;
        obj.ptr = nullptr;
        obj.count = nullptr;
    }
    return *this;
}


