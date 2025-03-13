#include <SDL3/SDL.h>
#include <SDL3_Image/SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_SPEED = 5;
const int BULLET_SPEED = 10;
const int ENEMY_SPEED = 2;
const Uint32 SHOOT_COOLDOWN = 200; // 200ms cooldown between shots
const char *PLAYER_PATH = "assets/Ships/ship_0001.png";
const char *ENEMY_PATH = "assets/Ships/ship_0012.png";
const char *BULLET_PATH = "assets/Ships/ship_0011.png";

struct Sprite
{
    SDL_Texture *texture;
    int x, y;
    int w, h;
};

struct Bullet
{
    int x, y;
    bool active;
};

struct Enemy
{
    int x, y;
    bool active;
};

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Space Shooter",
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (!window || !renderer)
    {
        std::cerr << "Window/Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    Sprite player;
    player.texture = IMG_LoadTexture(renderer, PLAYER_PATH);
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT - 100;
    player.w = 64;
    player.h = 64;

    Sprite bulletSprite;
    bulletSprite.texture = IMG_LoadTexture(renderer, BULLET_PATH);
    bulletSprite.w = 16;
    bulletSprite.h = 32;

    Sprite enemySprite;
    enemySprite.texture = IMG_LoadTexture(renderer, ENEMY_PATH);
    enemySprite.w = 48;
    enemySprite.h = 48;

    if (!player.texture || !bulletSprite.texture || !enemySprite.texture)
    {
        std::cerr << "Texture loading failed" << std::endl;
        return 1;
    }

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    bool quit = false;
    SDL_Event event;
    const bool *keyboardState = SDL_GetKeyboardState(NULL);
    srand(static_cast<unsigned>(time(0)));

    // Shooting cooldown variables
    Uint32 lastShotTime = 0;
    bool canShoot = true;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
        }

        // Get current time for cooldown
        Uint32 currentTime = SDL_GetTicks();

        // Player movement and shooting
        if (keyboardState[SDL_SCANCODE_LEFT] && player.x > 0)
        {
            player.x -= PLAYER_SPEED;
        }
        if (keyboardState[SDL_SCANCODE_RIGHT] && player.x < WINDOW_WIDTH - player.w)
        {
            player.x += PLAYER_SPEED;
        }
        if (keyboardState[SDL_SCANCODE_SPACE] && canShoot)
        {
            Bullet bullet = {player.x + player.w / 2 - 4, player.y, true};
            bullets.push_back(bullet);
            lastShotTime = currentTime;
            canShoot = false;
        }

        // Update shooting cooldown
        if (!canShoot && (currentTime - lastShotTime >= SHOOT_COOLDOWN))
        {
            canShoot = true;
        }

        // Spawn enemies
        if (rand() % 60 == 0)
        {
            Enemy enemy = {rand() % (WINDOW_WIDTH - enemySprite.w), -enemySprite.h, true};
            enemies.push_back(enemy);
        }

        // Update bullets
        for (auto &bullet : bullets)
        {
            if (bullet.active)
            {
                bullet.y -= BULLET_SPEED;
                if (bullet.y < 0)
                    bullet.active = false;
            }
        }

        // Update enemies
        for (auto &enemy : enemies)
        {
            if (enemy.active)
            {
                enemy.y += ENEMY_SPEED;
                if (enemy.y > WINDOW_HEIGHT)
                    enemy.active = false;
            }
        }

        // Collision detection
        for (auto &bullet : bullets)
        {
            if (!bullet.active)
                continue;
            for (auto &enemy : enemies)
            {
                if (!enemy.active)
                    continue;
                if (bullet.x < enemy.x + enemySprite.w &&
                    bullet.x + bulletSprite.w > enemy.x &&
                    bullet.y < enemy.y + enemySprite.h &&
                    bullet.y + bulletSprite.h > enemy.y)
                {
                    bullet.active = false;
                    enemy.active = false;
                }
            }
        }

        // Clean up inactive objects
        for (size_t i = bullets.size(); i > 0; --i)
        {
            if (!bullets[i - 1].active)
            {
                bullets.erase(bullets.begin() + (i - 1));
            }
        }
        for (size_t i = enemies.size(); i > 0; --i)
        {
            if (!enemies[i - 1].active)
            {
                enemies.erase(enemies.begin() + (i - 1));
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_FRect playerRect = {float(player.x), float(player.y), float(player.w), float(player.h)};
        SDL_RenderTexture(renderer, player.texture, NULL, &playerRect);

        for (const auto &bullet : bullets)
        {
            if (bullet.active)
            {
                SDL_FRect bulletRect = {float(bullet.x), float(bullet.y),
                                        float(bulletSprite.w), float(bulletSprite.h)};
                SDL_RenderTexture(renderer, bulletSprite.texture, NULL, &bulletRect);
            }
        }

        for (const auto &enemy : enemies)
        {
            if (enemy.active)
            {
                SDL_FRect enemyRect = {float(enemy.x), float(enemy.y),
                                       float(enemySprite.w), float(enemySprite.h)};
                SDL_RenderTexture(renderer, enemySprite.texture, NULL, &enemyRect);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(player.texture);
    SDL_DestroyTexture(bulletSprite.texture);
    SDL_DestroyTexture(enemySprite.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}