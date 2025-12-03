#include <stdio.h>
#include <stdbool.h>

#ifdef BUILD_FOR_10_4
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#endif

#include <SDL/SDL.h>

#ifndef USE_SDL_MAIN
#undef main
#endif

struct Player
{
    float x;
    float y;
    float size;
    float vx;
    float vy;
};

void drawRect(SDL_Surface* screen, float x, float y, float size, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Rect rect;
    rect.x = (Sint16)x;
    rect.y = (Sint16)y;
    rect.w = (Uint16)size;
    rect.h = (Uint16)size;

    Uint32 color = SDL_MapRGB(screen->format, r, g, b);
    SDL_FillRect(screen, &rect, color);
}

int sdl_app(int argc, char* argv[])
{
#ifdef BUILD_FOR_10_4
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp finishLaunching];
#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
#ifdef BUILD_FOR_10_4
        [pool release];
#endif
        return 1;
    }

    const int winW = 800;
    const int winH = 600;

    SDL_Surface *screen = SDL_SetVideoMode(winW, winH, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "Failed to set video mode: %s\n", SDL_GetError());
        SDL_Quit();
#ifdef BUILD_FOR_10_4
        [pool release];
#endif
        return 1;
    }

    SDL_WM_SetCaption("Simple SDL 1.2 Game (software)", NULL);

    struct Player player = {0};
    player.size = 40.0f;
    player.x = (winW - player.size) * 0.5f;
    player.y = (winH - player.size) * 0.5f;

    bool running = true;
    bool keyLeft = false, keyRight = false, keyUp = false, keyDown = false;
    const float speed = 200.0f;
    Uint32 lastTicks = SDL_GetTicks();

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_LEFT:   keyLeft  = true; break;
                    case SDLK_RIGHT:  keyRight = true; break;
                    case SDLK_UP:     keyUp    = true; break;
                    case SDLK_DOWN:   keyDown  = true; break;
                    default: break;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT:   keyLeft  = false; break;
                    case SDLK_RIGHT:  keyRight = false; break;
                    case SDLK_UP:     keyUp    = false; break;
                    case SDLK_DOWN:   keyDown  = false; break;
                    default: break;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTicks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = now;

        player.vx = 0.0f;
        player.vy = 0.0f;
        if (keyLeft)  player.vx -= speed;
        if (keyRight) player.vx += speed;
        if (keyUp)    player.vy -= speed;
        if (keyDown)  player.vy += speed;

        player.x += player.vx * dt;
        player.y += player.vy * dt;

        if (player.x < 0.0f) player.x = 0.0f;
        if (player.x + player.size > winW) player.x = winW - player.size;
        if (player.y < 0.0f) player.y = 0.0f;
        if (player.y + player.size > winH) player.y = winH - player.size;

        Uint32 bgColor = SDL_MapRGB(screen->format, 25, 25, 40);
        SDL_FillRect(screen, NULL, bgColor);

        drawRect(screen, player.x, player.y, player.size, 25, 230, 50);

        SDL_Flip(screen);
    }

    SDL_Quit();
#ifdef BUILD_FOR_10_4
    [pool release];
#endif
    return 0;
}

#ifdef USE_SDL_MAIN
int SDL_main(int argc, char* argv[])
{
    return sdl_app(argc, argv);
}
#else
int main(int argc, char* argv[])
{
    return sdl_app(argc, argv);
}
#endif
