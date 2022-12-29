#ifndef VKM_HPP
#define VKM_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_INLINE
#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>
#include <glm/gtc/bitfield.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_integer.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/reciprocal.hpp>
#include <glm/gtc/round.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/ulp.hpp>
#include <glm/gtc/vec1.hpp>

namespace vkm
{
    using namespace glm;

    template <typename T>
    GLM_FUNC_QUALIFIER mat<4, 4, T, defaultp> perspective(T fovy, T aspect, T zNear, T zFar)
    {
        mat<4, 4, T, defaultp> tmp = glm::perspective(fovy, aspect, zNear, zFar);
        tmp[1][1] *= -1;
        return tmp;
    }
} // namespace vkm

#endif // VKM_HPP
