#include "CloudWrapperBase.h"
#include "../math_utils.h"

// template specialization for float CloudWrapperProperty to handle nan values
template<>
bool CloudWrapperProperty<float>::isDifferentFromCloud() {
    return arduino::math::ieee754_different(_primitive_value, _cloud_value, Property::_min_delta_property);
}
