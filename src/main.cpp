#include "bve_pch.h"
#include "world.h"
#include "registry.h"
int main(int argc, const char** argv) {
    auto world = std::make_shared<bve::world>(glm::ivec3(1024));
    return 0;
}