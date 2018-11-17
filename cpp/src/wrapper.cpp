
#include <wrapper.hpp>
#include <node.h>
#include <string>
#include <cstdint>

// Object Implementation:
Wrapper::Object::Object(v8::Isolate * iso)
:
iso(iso), obj(v8::Object::New(iso))
{
}

Wrapper::Object::Object(v8::Isolate * iso, const v8::Local<v8::Object>& obj)
:
iso(iso), obj(obj)
{
}

Wrapper::Object& Wrapper::Object::operator=(const Object& wobject)
{
    iso = wobject.iso;
    obj = wobject.obj;

    return *this;
}

const v8::Local<v8::Object>& Wrapper::Object::raw() const
{
    return obj;
}

void Wrapper::Object::set(const std::string& field, const std::string& str)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        v8::String::NewFromUtf8(iso, str.c_str())
    );
}

void Wrapper::Object::set(const std::string& field, double val)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        v8::Number::New(iso, val)
    );
}

void Wrapper::Object::set(const std::string& field, int32_t val)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        v8::Integer::New(iso, val)
    );
}

void Wrapper::Object::set(const std::string& field, const Object& wobject)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        wobject.obj
    );
}

void Wrapper::Object::set(const std::string& field, const Array& warray)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        warray.arr
    );
}

void Wrapper::Object::get(const std::string& field, std::string& str) const
{
    str = *v8::String::Utf8Value
    (
        obj->Get(v8::String::NewFromUtf8(iso, field.c_str())).As<v8::String>()
    );
}

void Wrapper::Object::get(const std::string& field, double& val) const
{
    val = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Number>()->NumberValue();
}

void Wrapper::Object::get(const std::string& field, int32_t& val) const
{
    val = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Number>()->Int32Value();
}

void Wrapper::Object::get(const std::string& field, Object& wobject) const
{
    wobject.iso = iso;
    wobject.obj = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Object>();
}

void Wrapper::Object::get(const std::string& field, Array& warray) const
{
    warray.iso = iso;
    warray.arr = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Array>();
}

// Array Implementation:
Wrapper::Array::Array(v8::Isolate * iso, std::size_t size)
:
iso(iso), arr(v8::Array::New(iso, size))
{
}

Wrapper::Array::Array(v8::Isolate * iso, const v8::Local<v8::Array>& arr)
:
iso(iso), arr(arr)
{
}

Wrapper::Array& Wrapper::Array::operator=(const Array& warray)
{
    iso = warray.iso;
    arr = warray.arr;

    return *this;
}

const v8::Local<v8::Array>& Wrapper::Array::raw() const
{
    return arr;
}

void Wrapper::Array::set(std::size_t index, const std::string& str)
{
    arr->Set(index, v8::String::NewFromUtf8(iso, str.c_str()));
}

void Wrapper::Array::set(std::size_t index, double val)
{
    arr->Set(index, v8::Number::New(iso, val));
}

void Wrapper::Array::set(std::size_t index, int32_t val)
{
    arr->Set(index, v8::Integer::New(iso, val));
}

void Wrapper::Array::set(std::size_t index, const Object& wobject)
{
    arr->Set(index, wobject.obj);
}

void Wrapper::Array::set(std::size_t index, const Array& warray)
{
    arr->Set(index, warray.arr);
}

void Wrapper::Array::get(std::size_t index, std::string& str) const
{
    str = *v8::String::Utf8Value
    (
        arr->Get(index).As<v8::String>()
    );
}

void Wrapper::Array::get(std::size_t index, double& val) const
{
    val = arr->Get(index).As<v8::Number>()->NumberValue();
}

void Wrapper::Array::get(std::size_t index, int32_t& val) const
{
    val = arr->Get(index).As<v8::Number>()->Int32Value();
}

void Wrapper::Array::get(std::size_t index, Object& wobject) const
{
    wobject.iso = iso;
    wobject.obj = arr->Get(index).As<v8::Object>();
}

void Wrapper::Array::get(std::size_t index, Array& warray) const
{
    warray.iso = iso;
    warray.arr = arr->Get(index).As<v8::Array>();
}

std::size_t Wrapper::Array::size() const
{
    return static_cast<std::size_t>(arr->Length());
}
