#include "bve_pch.h"
#include "world.h"
namespace bve {
    world::world(glm::ivec3 size) {
        this->m_size = size;
        for (int32_t x = 0; x < this->m_size.x; x++) {
            for (int32_t y = 0; y < this->m_size.y; y++) {
                for (int32_t z = 0; z < this->m_size.z; z++) {
                    this->m_voxel_types[glm::ivec3(x, y, z)] = 0;
                }
            }
        }
    }
}