#include "PropertyPrimitive.h"

#include "../math_utils.h"


// template specialization for float Properties to handle nan values
template<>
bool PropertyPrimitive<float>::isDifferentFromCloud() {
    return arduino::math::ieee754_different(_value, _cloud_value, Property::_min_delta_property);
}
