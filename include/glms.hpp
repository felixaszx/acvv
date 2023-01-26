#ifndef GLMS_HPP
#define GLMS_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_INLINE
#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glms
{
    using namespace glm;

    template <typename T>
    GLM_FUNC_QUALIFIER mat<4, 4, T, defaultp> perspective(T fovy, T aspect, T zNear, T zFar)
    {
        mat<4, 4, T, defaultp> tmp = glm::perspective(fovy, aspect, zNear, zFar);
        tmp[1][1] *= -1;
        return tmp;
    }
} // namespace glms

#endif // GLMS_HPP
