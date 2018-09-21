
#include "wrapper.hpp"
#include <node.h>
#include <string>

// WObject Implementation:
WObject::WObject(v8::Isolate * iso)
:
iso(iso), obj(v8::Object::New(iso))
{
}

WObject::WObject(v8::Isolate * iso, const v8::Local<v8::Object>& obj)
:
iso(iso), obj(obj)
{
}

WObject& WObject::operator=(const WObject& wobject)
{
    iso = wobject.iso;
    obj = wobject.obj;

    return *this;
}

const v8::Local<v8::Object>& WObject::raw() const
{
    return obj;
}

void WObject::set(const std::string& field, const std::string& str)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        v8::String::NewFromUtf8(iso, str.c_str())
    );
}

void WObject::set(const std::string& field, double val)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        v8::Number::New(iso, val)
    );
}

void WObject::set(const std::string& field, const WObject& wobject)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        wobject.obj
    );
}

void WObject::set(const std::string& field, const WArray& warray)
{
    obj->Set
    (
        v8::String::NewFromUtf8(iso, field.c_str()),
        warray.arr
    );
}

void WObject::get(const std::string& field, std::string& str) const
{
    str = *v8::String::Utf8Value
    (
        obj->Get(v8::String::NewFromUtf8(iso, field.c_str())).As<v8::String>()
    );
}

void WObject::get(const std::string& field, double& val) const
{
    val = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Number>()->NumberValue();
}

void WObject::get(const std::string& field, WObject& wobject) const
{
    wobject.iso = iso;
    wobject.obj = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Object>();
}

void WObject::get(const std::string& field, WArray& warray) const
{
    warray.iso = iso;
    warray.arr = obj->Get
    (
        v8::String::NewFromUtf8(iso, field.c_str())
    ).As<v8::Array>();
}

// WArray Implementation:
WArray::WArray(v8::Isolate * iso, std::size_t size)
:
iso(iso), arr(v8::Array::New(iso, size))
{
}

WArray::WArray(v8::Isolate * iso, const v8::Local<v8::Array>& arr)
:
iso(iso), arr(arr)
{
}

WArray& WArray::operator=(const WArray& warray)
{
    iso = warray.iso;
    arr = warray.arr;

    return *this;
}

const v8::Local<v8::Array>& WArray::raw() const
{
    return arr;
}

void WArray::set(std::size_t index, const std::string& str)
{
    arr->Set(index, v8::String::NewFromUtf8(iso, str.c_str()));
}

void WArray::set(std::size_t index, double val)
{
    arr->Set(index, v8::Number::New(iso, val));
}

void WArray::set(std::size_t index, const WObject& wobject)
{
    arr->Set(index, wobject.obj);
}

void WArray::set(std::size_t index, const WArray& warray)
{
    arr->Set(index, warray.arr);
}

void WArray::get(std::size_t index, std::string& str) const
{
    str = *v8::String::Utf8Value
    (
        arr->Get(index).As<v8::String>()
    );
}

void WArray::get(std::size_t index, double& val) const
{
    val = arr->Get(index).As<v8::Number>()->NumberValue();
}

void WArray::get(std::size_t index, WObject& wobject) const
{
    wobject.iso = iso;
    wobject.obj = arr->Get(index).As<v8::Object>();
}

void WArray::get(std::size_t index, WArray& warray) const
{
    warray.iso = iso;
    warray.arr = arr->Get(index).As<v8::Array>();
}

std::size_t WArray::size() const
{
    return static_cast<std::size_t>(arr->Length());
}
