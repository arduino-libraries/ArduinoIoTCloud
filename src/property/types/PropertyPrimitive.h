#pragma once

#include "../Property.h"

template<typename T>
class PropertyPrimitive: public Property {
public:
    PropertyPrimitive(T v=0) : _value(v), _cloud_value(v) {}
    PropertyPrimitive(PropertyPrimitive<T> &&) = default;

    PropertyPrimitive(const PropertyPrimitive<T>&) = delete;

    bool isDifferentFromCloud() override {
        return _value != _cloud_value && (abs(float(_value - _cloud_value)) >= Property::_min_delta_property);
    }
    void fromCloudToLocal() override {
        _value = _cloud_value;
    }
    void fromLocalToCloud() override {
        _cloud_value = _value;
    }
    CborError appendAttributesToCloud(CborEncoder *encoder) override {
        return appendAttribute(_value, "", encoder);
    }
    void setAttributesFromCloud() override {
        setAttribute(_cloud_value, "");
    }

    inline PropertyPrimitive& operator=(const T& v) {
        updateValue(v);
        return *this;
    }

    inline PropertyPrimitive& operator=(const PropertyPrimitive &v) {
        updateValue(v._value);
        return *this;
    }

    PropertyPrimitive& operator=(PropertyPrimitive &&) = default;

    inline operator T() const {
        return _value;
    }

    inline PropertyPrimitive& operator+=(const T& v) {
        updateValue(_value + v);
        return *this;
    }

    inline PropertyPrimitive& operator-=(const T& v) {
        updateValue(_value - v);
        return *this;
    }

    inline PropertyPrimitive& operator/=(const T& v) {
        updateValue(_value / v);
        return *this;
    }

    inline PropertyPrimitive& operator*=(const T& v) {
        updateValue(_value * v);
        return *this;
    }

    inline PropertyPrimitive& operator++(int) {
        updateValue(_value + 1);
        return *this;
    }

    inline PropertyPrimitive& operator--(int) {
        updateValue(_value - 1);
        return *this;
    }

protected:
    inline void updateValue(const T& v) {
        _value = v;
        updateLocalTimestamp();
    }

    T   _value,
        _cloud_value;
};
