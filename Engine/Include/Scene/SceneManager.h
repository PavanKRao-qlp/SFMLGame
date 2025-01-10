#pragma once
#include "Umbra.h"

class SceneManager : Singleton<SceneManager> {

private:
    friend class Singleton<SceneManager>;
    SceneManager(/* args */);
    ~SceneManager();
};
