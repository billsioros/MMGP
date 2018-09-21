
#pragma once

#include <node.h>
#include <string>

class WArray;

class WObject
{
    friend class WArray;

    v8::Isolate * iso;

    v8::Local<v8::Object> obj;

public:

    WObject(v8::Isolate *);
    WObject(v8::Isolate *, const v8::Local<v8::Object>&);

    WObject& operator=(const WObject&);

    const v8::Local<v8::Object>& raw() const;

    void set(const std::string&, const std::string&);
    void set(const std::string&, double);
    void set(const std::string&, const WObject&);
    void set(const std::string&, const WArray&);

    void get(const std::string&, std::string&) const;
    void get(const std::string&, double&) const;
    void get(const std::string&, WObject&) const;
    void get(const std::string&, WArray&) const;
};

class WArray
{
    friend class WObject;

    v8::Isolate * iso;

    v8::Local<v8::Array> arr;

public:

    WArray(v8::Isolate *, std::size_t size = 0UL);
    WArray(v8::Isolate *, const v8::Local<v8::Array>&);

    WArray& operator=(const WArray&);

    const v8::Local<v8::Array>& raw() const;

    void set(std::size_t, const std::string&);
    void set(std::size_t, double);
    void set(std::size_t, const WObject&);
    void set(std::size_t, const WArray&);

    void get(std::size_t, std::string&) const;
    void get(std::size_t, double&) const;
    void get(std::size_t, WObject&) const;
    void get(std::size_t, WArray&) const;

    std::size_t size() const;
};
