#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

// constants
#define WIDTH 1200
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_RAY 0xffd43b
#define OBSTACLE_SPEED 2
#define RAY_THICKNESS 3
#define NUM_RAYS 500

// circle structure
struct Circle {
    double x;
    double y;
    double r;
};

// ray structure
struct Ray {
    double x, y;
    double angle;
};

// generate a circle using many 1 pixel rectangles
void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
    double radius_squared = pow(circle.r, 2);
    for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++) {
        for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++) {
            double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
            if (distance_squared < radius_squared) {
                SDL_Rect pixel = (SDL_Rect) {x, y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

// generate an array of ray structures given a circle and number of rays
void Generate_Rays(struct Circle circle, struct Ray rays[NUM_RAYS]) {
    for (int i = 0; i < NUM_RAYS; i++) {
        double angle = ((double) i / NUM_RAYS) * 2 * M_PI;
        struct Ray ray = {circle.x, circle.y, angle};
        rays[i] = ray;
    }
}

// draw the rays, stop if at end of screen or hit an object
void FillRays(SDL_Surface* surface, struct Ray rays[NUM_RAYS], Uint32 color, struct Circle object) {
   double radius_squared = pow(object.r, 2);
   for (int i = 0; i < NUM_RAYS; i++) {
        struct Ray ray = rays[i];
        int end_of_screen = 0;
        int object_hit = 0;

        double step = 1;
        double x_ray = ray.x;
        double y_ray = ray.y;
        while (!end_of_screen && !object_hit) {
            x_ray += step * cos(ray.angle);
            y_ray += step * sin(ray.angle);

            SDL_Rect ray_point = (SDL_Rect) {x_ray, y_ray, RAY_THICKNESS, RAY_THICKNESS};
            SDL_FillRect(surface, &ray_point, color);

            if (x_ray < 0 || x_ray > WIDTH) {
                end_of_screen = 1;
            }
            if (y_ray < 0 || y_ray > HEIGHT) {
                end_of_screen = 1;
            }

            double distance_squared = pow(x_ray - object.x, 2) + pow(y_ray - object.y, 2);
            if (distance_squared < radius_squared) {
                object_hit = 1;
            }
        }
   }
}

// bounce the provided circle up and down along the screen
void Bounce_Circle_Vertically(struct Circle* circle, double* obstacle_speed) {
    circle->y += *obstacle_speed;
    if (circle->y - circle->r < 0) {
        *obstacle_speed = -*obstacle_speed;
    }
    if (circle->y + circle->r > HEIGHT) {
        *obstacle_speed = -*obstacle_speed;
    }
}

// main method
int main(int argc, char *argv[]){
    // initialize a window
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    // create the basic 2 circles to start out with
    struct Circle circle = {200, 200, 40};
    struct Circle shadow_circle = {550, 300, 140};

    // create an "eraser" to clear the screen after each update
    SDL_Rect erase_rect = (SDL_Rect) {0, 0, WIDTH, HEIGHT};

    // declare and generate the rays for a given circle
    struct Ray rays[NUM_RAYS];
    Generate_Rays(circle, rays);

    // main game loop
    double obstacle_speed = 3;
    int simulation_running = 1;
    SDL_Event event;
    while (simulation_running) {
        // poll for events
        while (SDL_PollEvent(&event)) {
            // quit button should exit the game
            if (event.type == SDL_QUIT) {
                simulation_running = 0;
            }
            // click and drag should update circle location
            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0) {
                circle.x = event.motion.x;
                circle.y = event.motion.y;
                Generate_Rays(circle, rays);
            }
        }
        // "erase" screen using the black rect
        SDL_FillRect(surface, &erase_rect, COLOR_BLACK);

        // draw all objects on screen
        FillRays(surface, rays, COLOR_RAY, shadow_circle);
        FillCircle(surface, circle, COLOR_WHITE);
        FillCircle(surface, shadow_circle, COLOR_WHITE);        

        // update location of obstacle in scene
        Bounce_Circle_Vertically(&shadow_circle, &obstacle_speed);

        // update window
        SDL_UpdateWindowSurface(window);

        // framerate of 100
        SDL_Delay(10);
    }    
}