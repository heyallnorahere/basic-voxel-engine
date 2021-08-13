#pragma once
#include "script.h"
class player : public bve::script {
public:
    virtual void on_attach() override;
    virtual void update() override;
private:
    void update_camera_direction();
    void take_input();
};