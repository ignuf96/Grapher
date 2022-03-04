#include "../include/platform.h"
#include <SDL2/SDL_platform.h>
#include <string.h>

#define OS_LENGTH 20

enum OPERATING_SYSTEM get_platform(void)
{
    enum OPERATING_SYSTEM OS;

    if((strncmp(SDL_GetPlatform(), "Linux", OS_LENGTH)) == 0)
    {
       OS = LINUX;
    } else
    if(strncmp(SDL_GetPlatform(), "Windows", OS_LENGTH) == 0)
    {
        OS = WINDOWS;
    } else
    if(strncmp(SDL_GetPlatform(), "Mac OS X", OS_LENGTH) == 0)
    {
        OS = MACOS;
    }
    else
    if(strncmp(SDL_GetPlatform(), "Android", OS_LENGTH) == 0)
    {
        OS = ANDROID;
    } else
    if(strncmp(SDL_GetPlatform(), "iOS", OS_LENGTH) == 0)
    {
        OS = IOS;
    }

    return OS;
}
