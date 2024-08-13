#pragma once
#include "EnginePCH.h"
#include "Core/App.h"

extern D2D::App* CreateApplication();

int main()
{
    D2D::App* app = CreateApplication();
    int returnCode = app->Bootup();
    delete app;
    return returnCode;
}
