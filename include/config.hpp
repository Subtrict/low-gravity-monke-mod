#pragma once

struct config_t {
    bool enabled = false;
    bool isTrigger = false;
    float gravityAmount = 0.5;
};

extern config_t config;

void SaveConfig();
bool LoadConfig();