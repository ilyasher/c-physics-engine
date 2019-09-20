#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <time.h>

#include "sdl_wrapper.h"
#include "body.h"

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define MS_PER_S 1e3
#define IMG_SCALE 1.0

/**
 * The coordinate at the center of the screen.
 */
Vector center;
/**
 * The coordinate difference from the center to the top right corner.
 */
Vector max_diff;

Scene *scene;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 *
 */
void *aux;
/**
 * The keypress handler, or NULL if none has been configured.
 */
KeyHandler key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/**
 * Boolean to use SDL_Image when rendering scene, by default false.
 */
bool is_SDL_image = false;

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */

char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        case SDLK_SPACE: return SPACEBAR;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(Vector min, Vector max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max)),
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
}

// IMPORTANT: CHANGED THE ARGUMENT TYPE FROM 'VOID' TO 'SCENE'.
// This is necessary to avoid making keyhandlers take global parameters.
// There may be a better way to change this.
bool sdl_is_done(void) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (!key_handler) break;
                char key = get_keycode(event->key.keysym.sym);
                if (!key) break;

                double timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time =
                    (timestamp - key_start_timestamp) / MS_PER_S;
                // changed args, now also takes scene
                key_handler(key, type, held_time, scene);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

bool is_on_screen(Vector v, double r) {
    if (v.x - r > WINDOW_WIDTH * 1.5)  return false;
    if (v.x + r < -WINDOW_WIDTH / 2)   return false;
    if (v.y - r > WINDOW_HEIGHT * 1.5) return false;
    if (v.y + r < -WINDOW_HEIGHT / 2)  return false;
    return true;

}

Vector transform_coordinate(Vector v, Vector adjusted_center, double center_x, double center_y, double scale) {
    Vector pos = vec_multiply(scale, vec_subtract(v, adjusted_center));
    return (Vector){center_x + pos.x, center_y - pos.y};
}

void sdl_draw_polygon_from_body(Body *body, RGBColor color) {
    List *points = body_get_shape(body);
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window
    int *width = malloc(sizeof(int)),
        *height = malloc(sizeof(int));
    // assert(width);
    // assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;

    // Convert each vertex to a point on screen
    short *x_points = malloc(sizeof(short) * n),
          *y_points = malloc(sizeof(short) * n);
    // assert(x_points);
    // assert(y_points);
    Vector adjusted_center = center;
    Camera *camera = scene_get_camera(scene);
    if (camera_is_on(camera)) {
        adjusted_center = vec_add(center, camera_get_position(camera));
        scale *= camera_get_zoom(camera);
    }

    if (body_get_depth(body)) {
        scale *= 0.3;
    }

    /* Only render the body if it appears on screen. */
    Vector adjusted_centroid = polygon_centroid(points);
    double radius = body_get_radius(body) * scale * sqrt(2);
    Vector pos = vec_multiply(scale, vec_subtract(adjusted_centroid, adjusted_center));
    if (!is_on_screen((Vector){center_x + pos.x, center_y - pos.y}, radius)) {
        return;
    }
    for (size_t i = 0; i < n; i++) {
        Vector *vertex = list_get(points, i);
        Vector point = transform_coordinate(*vertex, adjusted_center, center_x, center_y, scale);
        x_points[i] = point.x;
        y_points[i] = point.y;
    }

    if (!body_get_image(body) || !is_SDL_image) {
        // Draw polygon with the given color
        filledPolygonRGBA(
            renderer,
            x_points, y_points, n,
            color.r * 255, color.g * 255, color.b * 255, 255
        );
        free(x_points);
        free(y_points);
        list_free(points);
    }

    else if (is_SDL_image) {
        Vector centroid = body_get_centroid(body);
        SDL_Rect *dest = malloc(sizeof(SDL_Rect));
        int radius = (int) body_get_radius(body);
        Vector corner = VEC_ZERO;
        Vector vel = body_get_velocity(body);
        // if (vec_len(vel) != 0) {
        //   double corner_mag = radius * 1.4142; // square root of 2
        //   Vector vel_unit =  vec_unit(vel);
        //   Vector corner_vel = vec_multiply(corner_mag, vel_unit);
        //   if (vec_dot(vel, (Vector){1, 0}) < 0) {
        //       corner = vec_rotate(corner_vel, -M_PI / 2.0);
        //   } else {
        //       corner = vec_rotate(corner_vel, M_PI / 2.0);
        //   }
        // }
        // else {
          corner = (Vector) {(centroid.x - radius), (centroid.y + radius)};
        // }
        corner = transform_coordinate(corner, adjusted_center, center_x, center_y, scale);
        dest->x = corner.x;
        dest->y = corner.y;
        dest->w = 2 * scale * radius * IMG_SCALE;
        dest->h = 2 * scale * radius * IMG_SCALE;

        // Initializes and renders texture from body->image.
        SDL_Texture *texture = body_get_texture(body);
        SDL_RenderCopy(renderer, texture, NULL, dest);
        free(dest);

    }
}

void sdl_init_textures(Scene *s);

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

void SDL_image_toggle(void) {
  is_SDL_image = !is_SDL_image;
}

void sdl_render_scene(Scene *s) {
    sdl_clear();
    scene = s;
    if (is_SDL_image) {
      SDL_Texture *bkg = scene_get_bkg(s);
      SDL_Rect *dest = malloc(sizeof(SDL_Rect));
      dest->x = 0;
      dest->y = 0;
      dest->w = 1000;
      dest->h = 1000;
      SDL_RenderCopy(renderer, bkg, NULL, dest);
      free(dest);
    }

    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        Vector original_body_position = body_get_centroid(body);
        if (camera_is_on(scene_get_camera(scene))) {
            Vector camera_position = camera_get_position(scene_get_camera(scene));
            Vector displacement_vector = displacement(original_body_position, camera_position, WINDOW_WIDTH, WINDOW_HEIGHT);
            body_set_centroid(body, vec_add(camera_position, displacement_vector));
        }
        sdl_draw_polygon_from_body(body, body_get_color(body));
        body_set_centroid(body, original_body_position);
    }
    sdl_show();
}

void sdl_init_textures(Scene *s) {
    int *width = malloc(sizeof(int)),
        *height = malloc(sizeof(int));
    // assert(width);
    // assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;
    if (camera_is_on(scene_get_camera(s))) {
        scale *= camera_get_zoom(scene_get_camera(s));
    }
    if (scene_get_bkg_image(s)) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, scene_get_bkg_image(s));
        scene_set_bkg(s, texture);
    }
    for (int i = 0; i < scene_bodies(s); i ++) {
        Body *body = scene_get_body(s, i);
        if (body_get_image(body)) {
            SDL_Rect *dest = malloc(sizeof(SDL_Rect));
            // Initializes and renders texture from body->image.
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, body_get_image(body));
            body_set_texture(body, texture);
            free(dest);
        }
    }
}

void sdl_on_key(KeyHandler handler) {
    key_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}
