#include "../Property.h"

template<typename T>
class PropertyPrimitive: public Property {
public:
    PropertyPrimitive() {
        PropertyPrimitive(0);
    }
    PropertyPrimitive(T v) : _value(v), _cloud_value(v) {}
    operator T() const {
        return _value;
    }
    virtual bool isDifferentFromCloud() {
        return _value != _cloud_value && (abs(float(_value - _cloud_value)) >= Property::_min_delta_property);
    }
    virtual void fromCloudToLocal() {
        _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
        _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
        return appendAttribute(_value, "", encoder);
    }
    virtual void setAttributesFromCloud() {
        setAttribute(_cloud_value, "");
    }

    //modifiers
    PropertyPrimitive& operator=(T v) {
        _value = v;
        updateLocalTimestamp();
        return *this;
    }
    PropertyPrimitive& operator=(PropertyPrimitive v) {
        return operator=((T)v);
    }
    PropertyPrimitive& operator+=(T v) {
        return operator=(_value += v);
    }
    PropertyPrimitive& operator-=(T v) {
        return operator=(_value -= v);
    }
    PropertyPrimitive& operator*=(T v) {
        return operator=(_value *= v);
    }
    PropertyPrimitive& operator/=(T v) {
        return operator=(_value /= v);
    }
    PropertyPrimitive& operator%=(T v) {
        return operator=(_value %= v);
    }
    PropertyPrimitive& operator++() {
        return operator=(++_value);
    }
    PropertyPrimitive& operator--() {
        return operator=(--_value);
    }
    PropertyPrimitive operator++(int) {
        operator=(_value + 1);
        return PropertyPrimitive(_value);
    }
    PropertyPrimitive operator--(int) {
        operator=(_value - 1);
        return PropertyPrimitive(_value);
    }
    PropertyPrimitive& operator&=(T v) {
        return operator=(_value &= v);
    }
    PropertyPrimitive& operator|=(T v) {
        return operator=(_value |= v);
    }
    PropertyPrimitive& operator^=(T v) {
        return operator=(_value ^= v);
    }
    PropertyPrimitive& operator<<=(T v) {
        return operator=(_value <<= v);
    }
    PropertyPrimitive& operator>>=(T v) {
        return operator=(_value >>= v);
    }

    //accessors
    PropertyPrimitive operator+() const {
        return PropertyPrimitive(+_value);
    }
    PropertyPrimitive operator-() const {
        return PropertyPrimitive(-_value);
    }
    PropertyPrimitive operator!() const {
        return PropertyPrimitive(!_value);
    }
    PropertyPrimitive operator~() const {
        return PropertyPrimitive(~_value);
    }

    //friends
    // TODO second parameter should not be T, but another template?
    friend PropertyPrimitive operator+(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw += v; // FIXME this should return a new property primitive
    }
    friend PropertyPrimitive operator+(PropertyPrimitive iw, T v) { // TODO may not be required -> T can become property primitive
        return iw += v;
    }
    friend PropertyPrimitive operator+(T v, PropertyPrimitive iw) { // TODO may not be required -> T can become property primitive
        return PropertyPrimitive(v) += iw; // FIXME this doesn't seem to make sense
    }
    friend PropertyPrimitive operator-(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw -= v;
    }
    friend PropertyPrimitive operator-(PropertyPrimitive iw, T v) {
        return iw -= v;
    }
    friend PropertyPrimitive operator-(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) -= iw;
    }
    friend PropertyPrimitive operator*(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw *= v;
    }
    friend PropertyPrimitive operator*(PropertyPrimitive iw, T v) {
        return iw *= v;
    }
    friend PropertyPrimitive operator*(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) *= iw;
    }
    friend PropertyPrimitive operator/(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw /= v;
    }
    friend PropertyPrimitive operator/(PropertyPrimitive iw, T v) {
        return iw /= v;
    }
    friend PropertyPrimitive operator/(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) /= iw;
    }
    friend PropertyPrimitive operator%(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw %= v;
    }
    friend PropertyPrimitive operator%(PropertyPrimitive iw, T v) {
        return iw %= v;
    }
    friend PropertyPrimitive operator%(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) %= iw;
    }
    friend PropertyPrimitive operator&(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw &= v;
    }
    friend PropertyPrimitive operator&(PropertyPrimitive iw, T v) {
        return iw &= v;
    }
    friend PropertyPrimitive operator&(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) &= iw;
    }
    friend PropertyPrimitive operator|(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw |= v;
    }
    friend PropertyPrimitive operator|(PropertyPrimitive iw, T v) {
        return iw |= v;
    }
    friend PropertyPrimitive operator|(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) |= iw;
    }
    friend PropertyPrimitive operator^(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw ^= v;
    }
    friend PropertyPrimitive operator^(PropertyPrimitive iw, T v) {
        return iw ^= v;
    }
    friend PropertyPrimitive operator^(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) ^= iw;
    }
    friend PropertyPrimitive operator<<(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw <<= v;
    }
    friend PropertyPrimitive operator<<(PropertyPrimitive iw, T v) {
        return iw <<= v;
    }
    friend PropertyPrimitive operator<<(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) <<= iw;
    }
    friend PropertyPrimitive operator>>(PropertyPrimitive iw, PropertyPrimitive v) {
        return iw >>= v;
    }
    friend PropertyPrimitive operator>>(PropertyPrimitive iw, T v) {
        return iw >>= v;
    }
    friend PropertyPrimitive operator>>(T v, PropertyPrimitive iw) {
        return PropertyPrimitive(v) >>= iw;
    }

private:
    T   _value,
        _cloud_value;
};
