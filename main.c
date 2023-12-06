#undef main
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#define PI 3.1415926535

// To use time library of C
#include <time.h>
 
void delay(float number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

#define screenx 1920
#define screeny 1080

Uint32 pixels[screenx * screeny];

void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xIncrement = (float)dx / steps;
    float yIncrement = (float)dy / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; ++i) {
        int pixelX = (int)x;
        int pixelY = (int)y;

        // Check if the pixel coordinates are within the image bounds
        if (pixelX >= 0 && pixelX < screenx && pixelY >= 0 && pixelY < screeny) {
            int index = pixelY * screenx + pixelX;
            pixels[index] = color;
        }

        x += xIncrement;
        y += yIncrement;
    }
}

void setPixel(Uint32 color, int x, int y){
    if(x >= 0 && x < screenx && y >= 0 && y < screeny){
        pixels[y * screenx + x] = color;
    }
}

void clear_screen(Uint32 color){
    for(int i = 0; i < screeny * screenx; ++i){pixels[i] = color;}
}

void draw_box_filled(int x, int y, Uint32 color, int xside, int yside){
    for(int xoff = -(xside / 2); xoff < xside / 2 + 1; ++xoff){
        for(int yoff = (yside / 2) + 1; yoff > -(yside / 2); --yoff){ 
            int newx = x + xoff;
            int newy = y + yoff;
            if(newx < screenx && newx > 0 && newy < screeny && newy > 0)
                pixels[newy * screenx + newx] = color;
        }
    }
}

float distance(int x1, int y1, int x2, int y2){
    float a = abs(x1 - x2);
    float b = abs(y1 - y2);
    return sqrt(a * a + b * b);
}

#define fps 120

float prevent_zero(float a){
    if(a == 0){
        return a + 0.01;
    } else {
        return a;
    }
}

float player_direction = 0;
float player_y_direction = 0;
float player_x = 0;
float player_y = 0;
float player_z = 0.5;

bool w_key, s_key, a_key, d_key;

float speed = 0.1;
float look_speed = 0.005;

void player_movement(){
    if(w_key){
        player_x += sin(player_direction) * speed;
        player_y += cos(player_direction) * speed;
    }
    if(s_key){
        player_x -= sin(player_direction) * speed;
        player_y -= cos(player_direction) * speed;
    }
    if(a_key){
        player_x += cos(-player_direction) * speed;
        player_y += sin(-player_direction) * speed;
    }
    if(d_key){
        player_x -= cos(-player_direction) * speed;
        player_y -= sin(-player_direction) * speed;
    }

    if(player_direction > PI){
        player_direction -= 2 * PI;
    }

    if(player_direction < -PI){
        player_direction += 2 * PI;
    }

    player_y_direction = SDL_clamp(player_y_direction, -PI, PI);
}

//FOV is in theta
#define FOV 1
#define pixels_per_theta ((screenx / 2) / FOV)

void render_wall(float lower_left_x, float lower_left_y, float lower_left_z, float x_length, float y_depth, float z_height){
    float x_angle_ll = atan(lower_left_x / lower_left_y);
    float y_angle_ll = atan(lower_left_z / lower_left_y);

    float x_angle_lr = atan((lower_left_x + x_length) / (lower_left_y + y_depth));
    float y_angle_lr = atan(lower_left_z / (lower_left_y + y_depth));

    float x_angle_ul = atan(lower_left_x / lower_left_y);
    float y_angle_ul = atan((lower_left_z + z_height) / lower_left_y);

    float x_angle_ur = atan((lower_left_x + x_length) / (lower_left_y + y_depth));
    float y_angle_ur = atan((lower_left_z + z_height) / (lower_left_y + y_depth));

    draw_line((player_direction - x_angle_ll) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ll) * pixels_per_theta + screeny / 2, 
              (player_direction - x_angle_lr) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_lr) * pixels_per_theta + screeny / 2, 0xFF00FFFF);

    draw_line((player_direction - x_angle_lr) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_lr) * pixels_per_theta + screeny / 2, 
              (player_direction - x_angle_ur) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ur) * pixels_per_theta + screeny / 2, 0xFF00FFFF);

    draw_line((player_direction - x_angle_ur) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ur) * pixels_per_theta + screeny / 2, 
              (player_direction - x_angle_ul) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ul) * pixels_per_theta + screeny / 2, 0xFF00FFFF);
            
    draw_line((player_direction - x_angle_ul) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ul) * pixels_per_theta + screeny / 2, 
              (player_direction - x_angle_ll) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ll) * pixels_per_theta + screeny / 2, 0xFF00FFFF);

    draw_line((player_direction - x_angle_lr) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_lr) * pixels_per_theta + screeny / 2, 
              (player_direction - x_angle_ul) * pixels_per_theta + screenx / 2, (player_y_direction - y_angle_ul) * pixels_per_theta + screeny / 2, 0xFF00FFFF);
}

void player_debug(){
    int x = player_x + screenx / 2;
    int y = player_y + screeny / 2;
    draw_box_filled(x, y, 0xFFFF0000, 10, 10);
    draw_line(x, y, x + (sin(player_direction) * 20), y + (cos(player_direction) * 20), 0xFFFF0000);
    draw_line(x, y, x + (cos(-player_direction) * 20), y + (sin(-player_direction) * 20), 0xFFFFFF00);
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenx, screeny, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screenx, screeny);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool quit = false;

    SDL_Event event;

    float delta_time;
    while (!quit) {
        clock_t start_time = clock();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }

                if(event.key.keysym.sym == SDLK_w){
                    w_key = true;
                }
                if(event.key.keysym.sym == SDLK_a){
                    a_key = true;
                }
                if(event.key.keysym.sym == SDLK_s){
                    s_key = true;
                }
                if(event.key.keysym.sym == SDLK_d){
                    d_key = true;
                }
            }
            
            if (event.type == SDL_KEYUP){
                if(event.key.keysym.sym == SDLK_w){
                    w_key = false;
                }
                if(event.key.keysym.sym == SDLK_a){
                    a_key = false;
                }
                if(event.key.keysym.sym == SDLK_s){
                    s_key = false;
                }
                if(event.key.keysym.sym == SDLK_d){
                    d_key = false;
                }
            }

            if (event.type == SDL_MOUSEMOTION) {
                // Get the x-coordinate of mouse movement
                int mouseX = event.motion.xrel;
                int mouseY = event.motion.yrel;

                // Clamp the mouse to the center of the screen
                SDL_WarpMouseInWindow(window, screenx / 2, screeny / 2);

                // Update player_direction based on mouseX
                player_direction += mouseX * look_speed;
                // Update player_direction based on mouseX
                player_y_direction += mouseY * look_speed;
            }
        }

        // Update the texture with the pixel data
        SDL_UpdateTexture(texture, NULL, pixels, screenx * sizeof(Uint32));

        // Clear the renderer
        SDL_RenderClear(renderer);

        // Render the texture
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        
        //ensure constant fps
        if(clock() % (1000 / fps) == 0){
            //----------Render code here----------//
            clear_screen(0xFF303030);
            player_movement();

            player_debug();

            render_wall(prevent_zero(0 - player_x), prevent_zero(0 - player_y), 0 - player_z, 10, 0, 1);
            draw_line(0 + screenx / 2, 0 + screeny / 2, 0 + 10 + screenx / 2, 0 + 0 + screeny / 2, 0xFF00FF00);
        }
        // Update the screen
        SDL_RenderPresent(renderer);

        delta_time = (clock() - start_time) / 1000.0;
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}