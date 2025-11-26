#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

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
    rect.x = static_cast<Sint16>(x);
    rect.y = static_cast<Sint16>(y);
    rect.w = static_cast<Uint16>(size);
    rect.h = static_cast<Uint16>(size);

    Uint32 color = SDL_MapRGB(screen->format, r, g, b);
    SDL_FillRect(screen, &rect, color);
}

int sdl_app(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    const int winW = 800;
    const int winH = 600;

    SDL_Surface* screen = SDL_SetVideoMode(winW, winH, 32, SDL_SWSURFACE);
    if (!screen)
    {
        std::cerr << "Failed to set video mode: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_WM_SetCaption("Simple SDL 1.2 Game (software)", nullptr);

    Player player{};
    player.size = 40.0f;
    player.x = (winW - player.size) * 0.5f;
    player.y = (winH - player.size) * 0.5f;
    player.vx = 0.0f;
    player.vy = 0.0f;

    bool running = true;
    bool keyLeft  = false;
    bool keyRight = false;
    bool keyUp    = false;
    bool keyDown  = false;

    const float speed = 200.0f; // pixels per second

    Uint32 lastTicks = SDL_GetTicks();

    while (running)
    {
        // --- Input ---
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
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

        // --- Timing ---
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTicks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = now;

        // --- Update velocity from input ---
        player.vx = 0.0f;
        player.vy = 0.0f;

        if (keyLeft)  player.vx -= speed;
        if (keyRight) player.vx += speed;
        if (keyUp)    player.vy -= speed;
        if (keyDown)  player.vy += speed;

        // --- Integrate position ---
        player.x += player.vx * dt;
        player.y += player.vy * dt;

        // --- Clamp to window ---
        if (player.x < 0.0f) player.x = 0.0f;
        if (player.x + player.size > winW) player.x = winW - player.size;
        if (player.y < 0.0f) player.y = 0.0f;
        if (player.y + player.size > winH) player.y = winH - player.size;

        // --- Render ---
        // Clear screen (dark background)
        Uint32 bgColor = SDL_MapRGB(screen->format, 25, 25, 40);
        SDL_FillRect(screen, nullptr, bgColor);

        // Draw player as a green square
        drawRect(screen, player.x, player.y, player.size, 25, 230, 50);

        // Present
        SDL_Flip(screen);
    }

    SDL_Quit();
    return 0;
}

int main(int argc, char* argv[])
{
    return sdl_app(argc, argv);
}
