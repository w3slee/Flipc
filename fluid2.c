#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define NUM_PARTICLES 1000
#define RADIUS 350
#define CENTER_X (WINDOW_WIDTH / 2)
#define CENTER_Y (WINDOW_HEIGHT / 2)
#define BASE_GRAVITY 0.5f
#define DAMPING 0.99f        // Higher damping to preserve more energy
#define TILT_SPEED 0.1f
#define MAX_TILT 2.0f
#define VELOCITY_CAP 15.0f   // Increased max velocity
#define MIN_VELOCITY 0.2f    // Increased minimum velocity
#define RANDOM_FORCE 0.05f   // Constant random force

typedef struct {
    float x, y;
    float vx, vy;
    float ax, ay;
    int active;  // Flag to ensure particle stays active
} Particle;

typedef struct {
    float x, y;
    float target_x, target_y;
} Accelerometer;

Particle particles[NUM_PARTICLES];
Accelerometer accel = {0.0f, BASE_GRAVITY, 0.0f, BASE_GRAVITY};

// Initialize particles with guaranteed valid positions
void init_particles() {
    int index = 0;
    float spacing = (RADIUS * 2.0f) / sqrt(NUM_PARTICLES);

    for (float y = -RADIUS + spacing; y < RADIUS - spacing && index < NUM_PARTICLES; y += spacing) {
        for (float x = -RADIUS + spacing; x < RADIUS - spacing && index < NUM_PARTICLES; x += spacing) {
            if (x*x + y*y <= (RADIUS-spacing)*(RADIUS-spacing)) {
                particles[index].x = CENTER_X + x;
                particles[index].y = CENTER_Y + y;
                particles[index].vx = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
                particles[index].vy = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
                particles[index].ax = 0;
                particles[index].ay = 0;
                particles[index].active = 1;
                index++;
            }
        }
    }

    // Ensure we fill up to NUM_PARTICLES
    while (index < NUM_PARTICLES) {
        float angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
        float r = ((float)rand() / RAND_MAX) * (RADIUS - spacing);
        particles[index].x = CENTER_X + cos(angle) * r;
        particles[index].y = CENTER_Y + sin(angle) * r;
        particles[index].vx = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
        particles[index].vy = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
        particles[index].ax = 0;
        particles[index].ay = 0;
        particles[index].active = 1;
        index++;
    }
}

void update_accelerometer() {
    float dx = accel.target_x - accel.x;
    float dy = accel.target_y - accel.y;

    accel.x += dx * 0.1f;
    accel.y += dy * 0.1f;
}

void handle_input(const Uint8* keyboard_state) {
    if (keyboard_state[SDL_SCANCODE_SPACE]) {
        accel.target_x = 0.0f;
        accel.target_y = BASE_GRAVITY;
    }

    if (keyboard_state[SDL_SCANCODE_LEFT]) {
        accel.target_x = fmax(accel.target_x - TILT_SPEED, -MAX_TILT);
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
        accel.target_x = fmin(accel.target_x + TILT_SPEED, MAX_TILT);
    }
    if (keyboard_state[SDL_SCANCODE_UP]) {
        accel.target_y = fmax(accel.target_y - TILT_SPEED, -MAX_TILT);
    }
    if (keyboard_state[SDL_SCANCODE_DOWN]) {
        accel.target_y = fmin(accel.target_y + TILT_SPEED, MAX_TILT);
    }

    // R key adds energy to the system
    if (keyboard_state[SDL_SCANCODE_R]) {
        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].vx += ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
            particles[i].vy += ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        }
    }
}

// Ensure velocity never goes below minimum
void maintain_minimum_velocity(float* vx, float* vy) {
    float speed = sqrt((*vx) * (*vx) + (*vy) * (*vy));
    if (speed < MIN_VELOCITY) {
        if (speed < 0.0001f) {  // Prevent division by zero
            float angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
            *vx = cos(angle) * MIN_VELOCITY;
            *vy = sin(angle) * MIN_VELOCITY;
        } else {
            float scale = MIN_VELOCITY / speed;
            *vx *= scale;
            *vy *= scale;
        }
    }
}

void update_particles() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].active = 1;  // Reactivate any inactive particles
            continue;
        }

        // Apply base acceleration plus random force
        particles[i].ax = accel.x + ((float)rand() / RAND_MAX - 0.5f) * RANDOM_FORCE;
        particles[i].ay = accel.y + ((float)rand() / RAND_MAX - 0.5f) * RANDOM_FORCE;

        // Update velocity
        particles[i].vx = particles[i].vx * DAMPING + particles[i].ax;
        particles[i].vy = particles[i].vy * DAMPING + particles[i].ay;

        // Ensure minimum velocity
        maintain_minimum_velocity(&particles[i].vx, &particles[i].vy);

        // Cap maximum velocity
        float speed = sqrt(particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy);
        if (speed > VELOCITY_CAP) {
            float scale = VELOCITY_CAP / speed;
            particles[i].vx *= scale;
            particles[i].vy *= scale;
        }

        // Update position
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        // Circle collision
        float dx = particles[i].x - CENTER_X;
        float dy = particles[i].y - CENTER_Y;
        float distance = sqrt(dx*dx + dy*dy);

        if (distance > RADIUS) {
            // Move back inside circle
            float angle = atan2(dy, dx);
            particles[i].x = CENTER_X + cos(angle) * (RADIUS - 1.0f);
            particles[i].y = CENTER_Y + sin(angle) * (RADIUS - 1.0f);

            // Bounce with energy preservation
            float normal_x = dx / distance;
            float normal_y = dy / distance;
            float dot_product = particles[i].vx * normal_x + particles[i].vy * normal_y;

            particles[i].vx = DAMPING * (particles[i].vx - 2 * dot_product * normal_x);
            particles[i].vy = DAMPING * (particles[i].vy - 2 * dot_product * normal_y);

            // Add slight tangential velocity
            float tang_x = -normal_y;
            float tang_y = normal_x;
            float rand_tang = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
            particles[i].vx += tang_x * rand_tang;
            particles[i].vy += tang_y * rand_tang;

            // Ensure minimum velocity after collision
            maintain_minimum_velocity(&particles[i].vx, &particles[i].vy);
        }
    }
}

void draw_gravity_indicator(SDL_Renderer* renderer) {
    int indicator_length = 50;
    int start_x = WINDOW_WIDTH - 70;
    int start_y = 70;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        SDL_RenderDrawPoint(renderer,
            start_x + indicator_length * cos(angle),
            start_y + indicator_length * sin(angle));
    }

    float normalized_x = accel.x / MAX_TILT;
    float normalized_y = accel.y / MAX_TILT;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer,
        start_x,
        start_y,
        start_x + normalized_x * indicator_length,
        start_y + normalized_y * indicator_length);
}

int main() {
    srand(time(NULL));

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

        const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
        handle_input(keyboard_state);

        update_accelerometer();
        update_particles();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw boundary
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        for (int i = 0; i < 360; i++) {
            float angle = i * M_PI / 180.0f;
            SDL_RenderDrawPoint(renderer,
                CENTER_X + RADIUS * cos(angle),
                CENTER_Y + RADIUS * sin(angle));
        }

        // Draw all active particles
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].active) {
                SDL_RenderDrawPoint(renderer,
                    (int)particles[i].x,
                    (int)particles[i].y);
            }
        }

        draw_gravity_indicator(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

