#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define NUM_PARTICLES 1000
#define RADIUS 350
#define CENTER_X (WINDOW_WIDTH / 2)
#define CENTER_Y (WINDOW_HEIGHT / 2)
#define GRAVITY 0.5f
#define DAMPING 0.8f

typedef struct {
    float x, y;        // Position
    float vx, vy;      // Velocity
    float ax, ay;      // Acceleration
} Particle;

Particle particles[NUM_PARTICLES];

// Initialize particles in a grid pattern within the circle
void init_particles() {
    int index = 0;
    float spacing = (RADIUS * 2.0f) / sqrt(NUM_PARTICLES);
    
    for (float y = -RADIUS; y < RADIUS && index < NUM_PARTICLES; y += spacing) {
        for (float x = -RADIUS; x < RADIUS && index < NUM_PARTICLES; x += spacing) {
            if (x*x + y*y <= RADIUS*RADIUS) {
                particles[index].x = CENTER_X + x;
                particles[index].y = CENTER_Y + y;
                particles[index].vx = 0;
                particles[index].vy = 0;
                particles[index].ax = 0;
                particles[index].ay = 0;
                index++;
            }
        }
    }
}

// Update particle physics
void update_particles() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // Apply gravity
        particles[i].ay += GRAVITY;
        
        // Update velocity
        particles[i].vx += particles[i].ax;
        particles[i].vy += particles[i].ay;
        
        // Update position
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        
        // Reset acceleration
        particles[i].ax = 0;
        particles[i].ay = 0;
        
        // Circle collision
        float dx = particles[i].x - CENTER_X;
        float dy = particles[i].y - CENTER_Y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if (distance > RADIUS) {
            float angle = atan2(dy, dx);
            particles[i].x = CENTER_X + cos(angle) * RADIUS;
            particles[i].y = CENTER_Y + sin(angle) * RADIUS;
            
            // Reflect velocity with damping
            float normal_x = dx / distance;
            float normal_y = dy / distance;
            float dot_product = particles[i].vx * normal_x + particles[i].vy * normal_y;
            
            particles[i].vx = DAMPING * (particles[i].vx - 2 * dot_product * normal_x);
            particles[i].vy = DAMPING * (particles[i].vy - 2 * dot_product * normal_y);
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("FLIP Fluid Simulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return 1;
    }

    init_particles();
    
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        update_particles();

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw boundary circle
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        for (int i = 0; i < 360; i++) {
            float angle = i * M_PI / 180.0f;
            SDL_RenderDrawPoint(renderer, 
                CENTER_X + RADIUS * cos(angle),
                CENTER_Y + RADIUS * sin(angle));
        }

        // Draw particles
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_RenderDrawPoint(renderer, 
                (int)particles[i].x, 
                (int)particles[i].y);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
