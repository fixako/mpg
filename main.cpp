#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <iostream>
#include "mpg_view.hpp"
#include "mpg_model.hpp"
#include "mpg_controller.hpp"
#include <Windows.h>

int main()
{
    MpgController mpgc;
    
    mpgc.startGame();
    return 0;
}