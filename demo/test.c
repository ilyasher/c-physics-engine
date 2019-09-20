#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "color.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include "shapes.h"
#include "collision.h"
#include "forces.h"
#include "camera.h"
#include "constants.h"

#define DT 0.01
#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT CANVAS_WIDTH
#define COLOR ((RGBColor) {.r = 15.0/255, .g = 188.0/255, .b = 0})

int main(int argc, char *argv[]) {
    srand(time(0));

    List *bird_shape = make_ngon(20, 4);
    Body *bird = body_init(bird_shape, 10, COLOR);
    body_set_radius(bird, 4);
    body_set_image(bird, "images/birdfire0000.png");
    body_set_centroid(bird, (Vector) {100, 200});
    body_set_velocity(bird, (Vector) {5, 5});

    List *planet_shape = make_ngon(20, 10);
    Body *planet = body_init(planet_shape, 10, COLOR);
    body_set_radius(planet, 10);
    body_set_image(planet, "images/planet10000.png");
    body_set_centroid(planet, (Vector) {-400, CANVAS_HEIGHT/2 - 400});
    body_set_velocity(planet, (Vector) {-5, -5});

    Scene *scene = scene_init();
    scene_add_body(scene, bird);
    scene_add_body(scene, planet);
    scene_set_bkg(scene, "images/planet50000.png");

    Vector bottom_left = (Vector) {-CANVAS_WIDTH/2, -CANVAS_HEIGHT/2};
    Vector top_right = (Vector) {CANVAS_WIDTH/2, CANVAS_HEIGHT/2};
    sdl_init(bottom_left, top_right);
    sdl_render_scene(scene);
    double elapsed = 0;
    SDL_image_toggle();

    while (!sdl_is_done()) {
      double time = time_since_last_tick();
      elapsed += time;
      if (elapsed > DT) {
        scene_tick(scene, elapsed);
        sdl_render_scene(scene);
        elapsed = 0;
      }
    }
  scene_free(scene);
}
