#include "Core/App.h"
#include "Diag/Logger.h"
#include "Game/SimpleGameinstance.h"

Umbra::App *CreateApplication()
{
    return new Umbra::App(new SimpleGameInstance());
}