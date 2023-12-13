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

#define screenx 1080
#define screeny 608

//also backround
#define null_color 0x00000000

Uint32 pixels[screenx * screeny];

// struct for wall data
typedef struct {
    float x;
    float y;
    float length;
    float depth;
    Uint32 color;
    float distance;
} Wall;

//struct for sectors
typedef struct
{
    int min_wall_index;
    int max_wall_index;
    float min_z;
    float height;
    Uint32 bottom_col;
    Uint32 top_col;
    float distance;

    //--top and bottom surfaces--
    int surface; //marker for which
    //y vals for lines
    int* surface_y1;
    int* surface_y2;
} Sector;

//main arrays for map data
Wall* wall_data;
Sector* sector_data;

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

        int result = fscanf(file, "%f %f %f %f %x", &wall.x, &wall.y, &wall.length, &wall.depth, &wall.color);
        if (result != 5) {
            fprintf(stderr, "Error reading data from file\n");
            break;
        }

        walls[i] = wall;
    }
    fclose(file);

    return walls;
}

int numSectors = 0;
const char* sector_filename = "sector_data.txt";

Sector* loadSectorData(Sector* sectors) {
    FILE* file = fopen(sector_filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return NULL;
    }

    // Read the number of sectors from the first line
    if (fscanf(file, "%d", &numSectors) != 1) {
        fprintf(stderr, "Error reading the number of sectors from file\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the sector array
    sectors = malloc(numSectors * sizeof(Sector));

    for (int i = 0; i < numSectors; i++) {
        Sector sector;

        int result = fscanf(file, "%d %d %f %f %x %x", &sector.min_wall_index,   &sector.max_wall_index,
                                                       &sector.min_z,            &sector.height, 
                                                       &sector.bottom_col,       &sector.top_col);
        if (result != 6) {
            fprintf(stderr, "Error reading data from file\n");
            break;
        }

        //allocate surface arrays
        sector.surface_y1 = malloc((screenx + 1) * sizeof(int));
        sector.surface_y2 = malloc((screenx + 1) * sizeof(int));

        sectors[i] = sector;
    }
    fclose(file);

    return sectors;
}

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
    if(x > 0 && x < screenx - 1 && y > 0 && y < screeny - 1){
        pixels[y * screenx + x] = color;
    }
}

//draws screen x and y coords to screen
void draw_wall(int ll_x, int ll_y, int lr_x, int lr_y, int ul_y, int ur_y, Uint32 color, int sector_num){
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

    int min_y = SDL_min(ul_y, ur_y);

    for (int i = 0; i < steps; ++i) {
        int ly = SDL_clamp(l_y, 0, screeny);
        int uy = SDL_clamp(u_y, 0, screeny);

        int x = SDL_clamp((int)l_x, 0, screenx);

        int min, max;
        if(ly < uy){
            min = ly;
            max = uy;
        } else {
            min = uy;
            max = ly;
        }

        for(int j = min; j < max; j++){
            setPixel(color, x, j);
        }

        if(sector_data[sector_num].surface == 1) {
            if(sector_data[sector_num].surface_y1[x] < sector_data[sector_num].surface_y2[x]){
                sector_data[sector_num].surface_y1[x] = max;
            } else {
                sector_data[sector_num].surface_y2[x] = max;
            }
        }
        if(sector_data[sector_num].surface == 2) {
            if(sector_data[sector_num].surface_y1[x] < sector_data[sector_num].surface_y2[x]){
                sector_data[sector_num].surface_y1[x] = min;
            } else {
                sector_data[sector_num].surface_y2[x] = min;
            }
        }

        l_x += l_xIncrement;
        l_y += l_yIncrement;

        u_y += u_yIncrement;
    }
}

//renders top and bottom surfaces of sectors
void render_surface_y_vals(int sector_num){
    for(int x = 0; x < screenx; x++){
        int min = SDL_min(sector_data[sector_num].surface_y1[x], sector_data[sector_num].surface_y2[x]);
        int max = SDL_max(sector_data[sector_num].surface_y1[x], sector_data[sector_num].surface_y2[x]);

        if(min == -1 || max == -1){
            continue;
        }

        for(int y = min; y < max; y++){
            if(sector_data[sector_num].surface == 1){
                setPixel(sector_data[sector_num].bottom_col, x, y);
            }
            if(sector_data[sector_num].surface == 2){
                setPixel(sector_data[sector_num].top_col, x, y);
            }
        }
    }
}

void clear_screen(Uint32 color){
    for(int i = 0; i < screeny * screenx; ++i){pixels[i] = color;}
}

//draws rectangles. Takes in center coords and side lengths
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

float distance(float x1, float y1, float x2, float y2){
    float a = abs(x1 - x2);
    float b = abs(y1 - y2);
    return sqrt(a * a + b * b);
}

//rendering stuff
#define focal_plane_depth 1000
#define fps 120

//about based on focal_plane_depth
#define FOV 0.5

float prevent_zero(float a){
    if(a == 0){
        return a + 0.01;
    } else {
        return a;
    }
}

float player_direction = 0.1;
float player_y_direction = 0;
float player_x = 0;
float player_y = -5;
float player_z = 0.5;

bool w_key, s_key, a_key, d_key;

//movement and mouse sensitivity
float speed = 0.1;
float look_speed = 0.002 * PI;

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

//clip walls to not go behind player
void prevent_y_behind_player(float *x1, float *y1, float x2, float y2)
{
    float y_distance = prevent_zero(y2 - *y1);
    float scale_factor = y2 / y_distance;

    *x1 = x2 - scale_factor * (x2 - (*x1));
    *y1 = 2;
}

//takes in player relative coords, rotates them, and converts to screen coords. draw_wall renders this to the screen
void render_wall(float lower_left_x, float lower_left_y, float lower_left_z, float x_length, float y_depth, float z_height, Uint32 color, int sector_num){
    float rotated_ll_x = lower_left_x * cos(player_direction) - lower_left_y * sin(player_direction);
    float rotated_ll_y = lower_left_x * sin(player_direction) + lower_left_y * cos(player_direction);

    float rotated_lr_x = (lower_left_x + x_length) * cos(player_direction) - (lower_left_y + y_depth) * sin(player_direction);
    float rotated_lr_y = (lower_left_x + x_length) * sin(player_direction) + (lower_left_y + y_depth) * cos(player_direction);

    //if behind player
    if(rotated_ll_y <= 0 && rotated_lr_y <= 0){
        return;
    }

    //if left behind player, clip
    if(rotated_ll_y <= 0){
        prevent_y_behind_player(&rotated_ll_x, &rotated_ll_y, rotated_lr_x, rotated_lr_y);
    }

    //if right behind player, clip
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
              pixel_ul_y, pixel_ur_y, color, sector_num);

}

//not really needed, but it's here
//draws player and lines for w and d movement
void player_debug(){
    int x = player_x + screenx / 2;
    int y = player_y + screeny / 2;
    draw_box_filled(x, y, 0xFFFF0000, 10, 10);
    draw_line(x, y, x + (sin(player_direction) * 20), y + (cos(player_direction) * 20), 0xFFFF0000);
    draw_line(x, y, x + (cos(-player_direction) * 20), y + (sin(-player_direction) * 20), 0xFFFFFF00);
}

//the big function
void render_all_sectors(){
    //sort all sectors based on distance
    for(int sec = 0; sec < numSectors - 1; sec++){    
        for(int w = 0; w < numSectors - sec - 1; w++){
            if(sector_data[w].distance < sector_data[w+1].distance){ 
                Sector stec = sector_data[w]; 
                sector_data[w] = sector_data[w+1];
                sector_data[w+1] = stec; 
            }
        }
    }


    //loop through all sectors
    for(int sec = 0; sec < numSectors; sec++){
        sector_data[sec].distance = 0;

        //determine if top/bottom surface needed
        if(sector_data[sec].min_z > player_z){
            sector_data[sec].surface = 1; // bottom
        } else if(sector_data[sec].height < player_z){
            sector_data[sec].surface = 2; // top
        } else {
            sector_data[sec].surface = 0; //none
        }

        //fill surface_y arrays with null values
        for(int i = 0; i < screenx; i++){
            sector_data[sec].surface_y1[i] = -1;
            sector_data[sec].surface_y2[i] = -1;
        }

        //sort walls inside sector
        int walls_in_sector = ((sector_data[sec].max_wall_index - sector_data[sec].min_wall_index) + 1);

        //array for walls in current sector
        Wall sector_walls[walls_in_sector];

        //fill sector_walls and compute wall distances
        for(int i = 0; i < walls_in_sector; i++){
            sector_walls[i] = wall_data[sector_data[sec].min_wall_index + i];
            sector_walls[i].distance = distance(player_x, player_y, (sector_walls[i].x * 2 + sector_walls[i].length) / 2,
                                                                    (sector_walls[i].y * 2 + sector_walls[i].depth)  / 2);
        }

        //sort sector_walls
        for(int i = 0; i < walls_in_sector - 1; i++){    
            for(int w = 0; w < walls_in_sector - i - 1; w++){
                if(sector_walls[w].distance < sector_walls[w+1].distance){ 
                    Wall wa = sector_walls[w]; 
                    sector_walls[w] = sector_walls[w+1];
                    sector_walls[w+1] = wa; 
                }
            }
        }

        //draw sector_walls
        for (int wall = 0; wall < walls_in_sector; wall++)
        {   
            float start_x = sector_walls[wall].x - player_x;
            float start_y = sector_walls[wall].y - player_y;
            float start_z = sector_data[sec].min_z - player_z;
            float length = sector_walls[wall].length;
            float depth = sector_walls[wall].depth;
            float height = sector_data[sec].height;
                    
            render_wall(start_x, start_y, start_z, 
                        length,  depth,   height,  sector_walls[wall].color, sec);

            // add up all wall distances for the sector
            sector_data[sec].distance += sector_walls[wall].distance;
        }

        //divide total wall distance by number of walls to get average for the sector
        sector_data[sec].distance = sector_data[sec].distance / walls_in_sector;

        //draw top and bottom surfaces
        render_surface_y_vals(sec);
    }
}

void draw_floor(){
    for(int x = 0; x < screenx; x++){
        for(int y = screeny / 2; y <= screeny; y++){
            float dist = y - (screeny / 2);
            int brightness = SDL_clamp(255 - (255 / dist * 10), 0, 255);

            Uint32 color = brightness * 65536 + brightness * 256 + brightness;

            setPixel(color, x, y);
        }
    }
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenx, screeny, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screenx, screeny);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool quit = false;

    SDL_Event event;

    //init map data
    wall_data = loadWallData(wall_data);
    sector_data = loadSectorData(sector_data);

    float delta_time;
    while (!quit) {
        clock_t start_time = clock();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            
            //read movement key downs
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

            //read movement key ups
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

            //read mouse
            if (event.type == SDL_MOUSEMOTION) {
                // Get the coordinates of mouse movement
                int mouseX = event.motion.xrel;
                int mouseY = event.motion.yrel;

                // Clamp the mouse to the center of the screen
                SDL_WarpMouseInWindow(window, screenx / 2, screeny / 2);

                // Update player_direction based on mouseX
                player_direction += mouseX * look_speed;

                // Update player_y_direction based on mouseY
                // does nothing right now
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
            
            draw_floor();
            render_all_sectors();
        }
        
        // Update the screen
        SDL_RenderPresent(renderer);

        //compute delta_time
        delta_time = (clock() - start_time) / 1000.0;
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}