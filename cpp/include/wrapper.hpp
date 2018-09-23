
#pragma once

#include <node.h>
#include <string>

namespace Wrapper
{

class Array;

class Object
{
    friend class Array;

    v8::Isolate * iso;

    v8::Local<v8::Object> obj;

public:

    Object(v8::Isolate *);
    Object(v8::Isolate *, const v8::Local<v8::Object>&);

    Object& operator=(const Object&);

    const v8::Local<v8::Object>& raw() const;

    void set(const std::string&, const std::string&);
    void set(const std::string&, double);
    void set(const std::string&, const Object&);
    void set(const std::string&, const Array&);

    void get(const std::string&, std::string&) const;
    void get(const std::string&, double&) const;
    void get(const std::string&, Object&) const;
    void get(const std::string&, Array&) const;
};

class Array
{
    friend class Object;

    v8::Isolate * iso;

    v8::Local<v8::Array> arr;

public:

    Array(v8::Isolate *, std::size_t size = 0UL);
    Array(v8::Isolate *, const v8::Local<v8::Array>&);

    Array& operator=(const Array&);

    const v8::Local<v8::Array>& raw() const;

    void set(std::size_t, const std::string&);
    void set(std::size_t, double);
    void set(std::size_t, const Object&);
    void set(std::size_t, const Array&);

    void get(std::size_t, std::string&) const;
    void get(std::size_t, double&) const;
    void get(std::size_t, Object&) const;
    void get(std::size_t, Array&) const;

    std::size_t size() const;
};

}
