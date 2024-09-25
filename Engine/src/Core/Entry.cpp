#pragma once
#include "Core/App.h"
#include "Umbra.h"

extern Umbra::App* CreateApplication();

int main() {
    Umbra::App* app = CreateApplication();
    int returnCode  = app->Bootup();
    delete app;
    return returnCode;
}
