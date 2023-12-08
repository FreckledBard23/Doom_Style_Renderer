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

#define screenx 1000
#define screeny 800

#define null_color 0x00000000

Uint32 pixels[screenx * screeny];

void draw_line(int x1, int y1, int x2, int y2, uint32_t color, bool only_null) {
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
            if(only_null){
                int index = pixelY * screenx + pixelX;
                if(pixels[index] == null_color){
                    pixels[index] = color;
                }
            } else {
                int index = pixelY * screenx + pixelX;
                pixels[index] = color;
            }
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

void draw_wall(int ll_x, int ll_y, int lr_x, int lr_y, int ul_y, int ur_y, Uint32 color){
    int l_dx = lr_x - ll_x;
    int l_dy = lr_y - ll_y;
    int steps = abs(l_dx) > abs(l_dy) ? abs(l_dx) : abs(l_dy);

    float l_xIncrement = (float)l_dx / steps;
    float l_yIncrement = (float)l_dy / steps;

    float l_x = ll_x;
    float l_y = ll_y;

    int u_dy = ur_y - ul_y;

    float u_yIncrement = (float)u_dy / steps;

    float u_y = ul_y;

    for (int i = 0; i <= steps; ++i) {
        int ly = SDL_clamp(l_y, 0, screeny);
        int uy = SDL_clamp(u_y, 0, screeny);

        int min, max;
        if(ly < uy){
            min = ly;
            max = uy;
        } else {
            min = uy;
            max = ly;
        }

        for(int j = min; j < max; j++){
            setPixel(color, SDL_clamp(l_x, 0, screenx), j);
        }

        l_x += l_xIncrement;
        l_y += l_yIncrement;

        u_y += u_yIncrement;
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

// Define a structure to represent wall data
typedef struct {
    float x;
    float y;
    float z;
    float length;
    float depth;
    float height;
    Uint32 color;
} Wall;

int numWalls = 0;
const char* wall_filename = "wall_data.txt";

Wall* loadWallData(Wall* walls) {
    FILE* file = fopen(wall_filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return NULL;
    }

    // Read the number of walls from the first line
    if (fscanf(file, "%d", &numWalls) != 1) {
        fprintf(stderr, "Error reading the number of walls from file\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the walls array
    walls = malloc(numWalls * sizeof(Wall));

    for (int i = 0; i < numWalls; i++) {
        Wall wall;

        int result = fscanf(file, "%f %f %f %f %f %f %x", &wall.x, &wall.y, &wall.z, &wall.length, &wall.depth, &wall.height, &wall.color);
        if (result != 7) {
            fprintf(stderr, "Error reading data from file\n");
            break;
        }

        walls[i] = wall;
    }
    fclose(file);

    return walls;
}

float player_direction = 0;
float player_y_direction = 0;
float player_x = 4;
float player_y = -5;
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
        player_x -= cos(-player_direction) * speed;
        player_y -= sin(-player_direction) * speed;
    }
    if(d_key){
        player_x += cos(-player_direction) * speed;
        player_y += sin(-player_direction) * speed;
    }

    if(player_direction > PI){
        player_direction -= 2 * PI;
    }

    if(player_direction < -PI){
        player_direction += 2 * PI;
    }

    player_y_direction = SDL_clamp(player_y_direction, -PI, PI);
}

void prevent_y_behind_player(float *x1,float *y1, float x2,float y2)
{
    float y_distance = prevent_zero(y2 - *y1);
    float scale_factor = y2 / y_distance;

    *x1 = x2 - scale_factor * (x2 - (*x1));
    *y1 = 2;
}

//FOV is in theta
#define focal_plane_depth 1000

void render_wall(float lower_left_x, float lower_left_y, float lower_left_z, float x_length, float y_depth, float z_height, Uint32 color){
    float rotated_ll_x = lower_left_x * cos(player_direction) - lower_left_y * sin(player_direction);
    float rotated_ll_y = lower_left_x * sin(player_direction) + lower_left_y * cos(player_direction);

    float rotated_lr_x = (lower_left_x + x_length) * cos(player_direction) - (lower_left_y + y_depth) * sin(player_direction);
    float rotated_lr_y = (lower_left_x + x_length) * sin(player_direction) + (lower_left_y + y_depth) * cos(player_direction);

    if(rotated_ll_y <= 0 && rotated_lr_y <= 0){
        return;
    }

    if(rotated_ll_y <= 0){
        prevent_y_behind_player(&rotated_ll_x, &rotated_ll_y, rotated_lr_x, rotated_lr_y);
    }

    if(rotated_lr_y <= 0){
        prevent_y_behind_player(&rotated_lr_x, &rotated_lr_y, rotated_ll_x, rotated_ll_y);
    }

    float pixel_ll_x = (rotated_ll_x * focal_plane_depth) / rotated_ll_y + screenx / 2;
    float pixel_ll_y = screeny / 2 - (lower_left_z * focal_plane_depth) / rotated_ll_y;

    float pixel_lr_x = (rotated_lr_x * focal_plane_depth) / rotated_lr_y + screenx / 2;
    float pixel_lr_y = screeny / 2 - (lower_left_z * focal_plane_depth) / rotated_lr_y;

    float pixel_ul_y = screeny / 2 - ((lower_left_z + z_height) * focal_plane_depth) / rotated_ll_y;

    float pixel_ur_y = screeny / 2 - ((lower_left_z + z_height) * focal_plane_depth) / rotated_lr_y;

    draw_wall(pixel_ll_x, pixel_ll_y,
              pixel_lr_x, pixel_lr_y,
              pixel_ul_y, pixel_ur_y, color);
}

void player_debug(){
    int x = player_x + screenx / 2;
    int y = player_y + screeny / 2;
    draw_box_filled(x, y, 0xFFFF0000, 10, 10);
    draw_line(x, y, x + (sin(player_direction) * 20), y + (cos(player_direction) * 20), 0xFFFF0000, false);
    draw_line(x, y, x + (cos(-player_direction) * 20), y + (sin(-player_direction) * 20), 0xFFFFFF00, false);
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenx, screeny, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screenx, screeny);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool quit = false;

    SDL_Event event;

    Wall* wall_data = loadWallData(wall_data);

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
            clear_screen(null_color);
            player_movement();

            for(int i = 0; i < numWalls; i++){
                render_wall(wall_data[i].x - player_x, wall_data[i].y - player_y, wall_data[i].z - player_z, 
                            wall_data[i].length, wall_data[i].depth, wall_data[i].height, wall_data[i].color);
            }
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