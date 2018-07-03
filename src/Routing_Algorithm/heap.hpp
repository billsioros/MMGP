/* C++ Heap implementation by Vasileios Sioros */

#ifndef __HEAP__
#define __HEAP__

#include <functional>       // const std::function<bool(const T&, const T&)>

template <typename T>
class heap
{
    const std::function<bool(const T&, const T&)> priority;

    const size_t max; size_t size;

    T * items;

public:
    
    static bool less(const T&, const T&);
    static bool greater(const T&, const T&);

    heap(size_t, const std::function<bool(const T&, const T&)>& priority);
    ~heap();

    bool push(const T&);
    bool pop(T&);
};

// Heap Implementation:
template <typename T>
bool heap<T>::less(const T& a, const T& b)
{
    return a < b;
}

template <typename T>
bool heap<T>::greater(const T& a, const T& b)
{
    return a > b;
}

template <typename T>
heap<T>::heap(size_t max, const std::function<bool(const T&, const T&)>& priority)
:
priority(priority), max(max), size(0U), items(new T[this->max + 1U])
{
}

template <typename T>
heap<T>::~heap()
{
    delete[] items;
}

template <typename T>
bool heap<T>::push(const T& item)
{
    if(size == max)
        return false;

    size_t child = ++size, parent = child / 2;
    items[child] = item;

    while (parent)
    {
        if (!priority(items[child], items[parent]))
            break;

        T tmp = items[child];
        items[child] = items[parent];
        items[parent] = tmp;

        child = parent;
        parent = child / 2;
    }

    return true;
}

template <typename T>
bool heap<T>::pop(T& item)
{
    if(!size)
        return false;

    item = items[1];
    items[1] = items[size--];

    size_t current = 1, child = 2 * current;
    while (child <= size)
    {
        size_t next;
        if (child + 1 <= size && priority(items[child + 1], items[child]))
            next = child + 1;
        else
            next = child;

        if (priority(items[current], items[next]))
            break;

        T tmp = items[current];
        items[current] = items[next];
        items[next] = tmp;

        current = next;
        child = 2 * next;
    }

    return true;
}

#endif
