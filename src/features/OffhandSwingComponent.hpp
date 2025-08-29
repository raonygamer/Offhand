#pragma once

// note: this is not an MC component, this just seemed like the easiest way to add variables to player without 
// having to do some weird shit like global variables. 
// if anyones reading this, thoughts? am I missing something more obvious?

struct OffhandSwingComponent {
public:
    bool mOffhandSwinging = false;
    int mOffhandSwingTime = false;

    float mOffhandAttackAnim = 0.0f;
    float mOffhandOAttackAnim = 0.0f; 
};