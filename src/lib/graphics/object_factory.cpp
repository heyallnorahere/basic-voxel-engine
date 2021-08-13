#include "bve_pch.h"
#include "graphics/object_factory.h"
#include "opengl/opengl_object_factory.h"
namespace bve {
    namespace graphics {
        ref<object_factory> object_factory::create(graphics_api api) {
            switch (api) {
            case graphics_api::OPENGL:
                return ref<opengl::opengl_object_factory>::create();
                break;
            default:
                return nullptr;
            }
        }
    }
}