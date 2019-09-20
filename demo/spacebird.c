#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
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
#define LARGE_NUMBER 1000000.0
#define LARGE_MASS LARGE_NUMBER
#define SMALL_MASS 1

#define SHIP_MAX_SPEED (0.3/DT)
#define SHIP_ACCELERATION (0.1/DT)
#define SHIP_MASS 1
#define IMPULSE_MAGNITUDE (0.1/DT)
#define SHIP_VEL (1/DT)
#define SHIP_COLOR ((RGBColor) {.r = 15.0/255, .g = 188.0/255, .b = 0})
#define MAX_LIVES 5

#define INITIAL_POSITION PLAYER_START_POSITION

#define NUM_PLANETS 10
#define PLANET_MASS 1000000.0

#define NUM_ASTEROIDS 20
#define ASTEROID_MASS 10.0
#define ASTEROID_INIT_VEL polar_to_cartesian(10, rand() % 6)

#define NUM_ALIENS 10
#define TRACKING_DISTANCE 100
#define ALIEN_TRACKING_SPEED (SHIP_MAX_SPEED * 4/5)

#define NUM_BLACK_HOLES 2
#define BLACK_HOLE_MASS (PLANET_MASS * 2)

#define NUM_STARS 0

#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 1000

#define BACKGROUND_INDEX 0
#define FIRST_STAR_INDEX 4
#define SHIP_INDEX 4 + NUM_STARS
#define HABITABLE_PLANET_INDEX (SHIP_INDEX + 1)
#define FIRST_PLANET_INDEX (HABITABLE_PLANET_INDEX + 1)
#define FIRST_BLACK_HOLE_INDEX (FIRST_PLANET_INDEX + NUM_PLANETS)
#define FIRST_ASTEROID_INDEX (FIRST_BLACK_HOLE_INDEX + NUM_BLACK_HOLES)
#define FIRST_ALIEN_INDEX (FIRST_ASTEROID_INDEX + NUM_ASTEROIDS)

#define MIN_ZOOM 9
#define MAX_ZOOM 12
#define ZOOM_DELAY 0.003

#define G 0.1

/* To do next:
 * bird animations
 */

Vector polar_to_cartesian(double radius, double angle) {
    return (Vector) {radius * cos(angle), radius * sin(angle)};
}

double vec_to_angle(Vector v) {
    if (v.x == 0) {
        return M_PI/2; // could also be M_PI * 3/2
    }
    return atan(v.y / v.x) + M_PI * (v.x < 0);
}

void ship_body_collision_handler(Body *body1, Body *body2, Vector axis, void *aux){
    *(bool *)body_get_info(body1) = false;
}


void generic_body_collision(void *aux){
  ColAux temp = *(ColAux *)aux;
  if((vec_dot(vec_subtract(body_get_velocity(temp.body1), body_get_velocity(temp.body2)),
              vec_subtract(body_get_centroid(temp.body2), body_get_centroid(temp.body1))) > 0)
  /*|| (vec_dot(vec_subtract(body_get_force(temp.body1), body_get_force(temp.body2)),
              vec_subtract(body_get_centroid(temp.body2), body_get_centroid(temp.body1))) > 0
      && vec_dot(vec_subtract(body_get_impulse(temp.body1), body_get_impulse(temp.body2)),
                  vec_subtract(body_get_centroid(temp.body2), body_get_centroid(temp.body1))) >= 0)*/){
    CollisionInfo info = find_circle_body_collision(temp.body1, temp.body2);
    if(info.collided){
      temp.handler(temp.body1, temp.body2, info.axis, temp.aux);
    }
  }
}

void create_body_collision(Scene *scene, Body *body1, Body *body2, CollisionHandler handler, void *aux, FreeFunc freer){
  ColAux *collisiondata = malloc(sizeof(ColAux));
  *collisiondata = (ColAux){body1, body2, handler, aux, freer};
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, generic_body_collision, (void *)collisiondata, bodies, (FreeFunc) col_aux_free);
}

void create_ship_body_collision(Scene *scene, Body *body1, Body *body2, void *aux, FreeFunc freer){
    create_body_collision(scene, body1, body2, ship_body_collision_handler, aux, freer);
}

// typedef struct body_info {
//     int depth;
// } BodyInfo;
//
// BodyInfo *body_info_init(int depth) {
//     BodyInfo *body_info = malloc(sizeof(BodyInfo));
//     assert(body_info);
//     body_info->depth = depth;
//     return body_info;
// }

void create_intro_background(Scene *scene, RGBColor color) {
    List *background_shape = make_rectangle(CANVAS_HEIGHT * 2, CANVAS_WIDTH * 2);
    Body *background = body_init(background_shape, LARGE_NUMBER, color);
    body_set_velocity(background, VEC_ZERO);
    body_set_centroid(background, VEC_ZERO);
    scene_add_body(scene, background);
}

Body *create_instructions(Scene *scene) {
    List *shape = make_rectangle(1, 1);
    Body *body = body_init(shape, LARGE_NUMBER, COLOR_BLACK);
    body_set_velocity(body, VEC_ZERO);
    body_set_centroid(body, (Vector){0, -CANVAS_HEIGHT/5});
    body_set_radius(body, 250);
    body_set_image(body, "images/welcome/instructions.png");
    scene_add_body(scene, body);
    return body;
}

void create_large_planet(Scene *scene, RGBColor color) {
    List *planet_shape = make_ngon(100, 1000);
    Body *planet = body_init(planet_shape, LARGE_NUMBER, color);
    body_set_radius(planet, 1000);
    body_set_image(planet, "images/planetwin0000.png");
    body_set_velocity(planet, VEC_ZERO);
    body_set_centroid(planet, (Vector){0, -1250});
    scene_add_body(scene, planet);
}

Body *create_win_ship(Scene *scene) {
    List *ship_shape = make_ship_shape(20, 30, 20);
    Body *ship = body_init(ship_shape, SHIP_MASS, SHIP_COLOR);
    body_set_radius(ship, 20);
    body_set_image(ship, "images/birdfireright.png");
    body_set_rotation(ship, M_PI/2);
    body_set_centroid(ship, (Vector) {0, 500});
    body_set_velocity(ship, VEC_ZERO);
    scene_add_body(scene, ship);
    return ship;
}

void create_background_tiles(Scene *scene, RGBColor color) {
    for (double i = 0; i < 2; i ++) {
        for (double j = 0; j < 2; j ++) {
            List *background_shape = make_rectangle(CANVAS_HEIGHT/2, CANVAS_WIDTH/2);
            Body *background = body_init(background_shape, LARGE_NUMBER, color);
            body_set_velocity(background, VEC_ZERO);
            body_set_depth(background, 1);
            body_set_centroid(background, (Vector) {(i - 0.5) * CANVAS_WIDTH/2, (j - 0.5) * CANVAS_HEIGHT/2});
            body_set_radius(background, CANVAS_WIDTH/4);
            body_set_image(background, SKY_IMAGES[(int)(i * 2 + j)]);
            scene_add_body(scene, background);
        }
    }
}

Body *create_ship(Scene *scene) {
    List *ship_shape = make_ship_shape(5, 6, 4);
    bool *alive = malloc(sizeof(bool));
    *alive = true;
    Body *ship = body_init_with_info(ship_shape, SHIP_MASS, SHIP_COLOR, alive, free);
    body_set_radius(ship, 4);
    body_set_image(ship, "images/birdfireright.png");
    body_set_centroid(ship, INITIAL_POSITION);
    scene_add_body(scene, ship);
    return ship;
}

Body *create_habitable_planet(Scene *scene) {
    List *planet_shape = make_ngon(20, 30);
    Body *planet = body_init(planet_shape, PLANET_MASS, COLOR_GREEN);
    body_set_radius(planet, 30);
    body_set_image(planet, "images/planetwin0000.png");
    body_set_centroid(planet, HABITABLE_PLANET_POSITION);
    body_set_velocity(planet, VEC_ZERO);
    scene_add_body(scene, planet);
    return planet;
}

void create_planets(Scene *scene) {
    for (int i = 0; i < NUM_PLANETS; i++) {
        double y_pos = PLANET_Y[i];
        double x_pos = PLANET_X[i];
        double planet_radius = PLANET_RADII[i];
        double mass = PLANET_MASS * pow(planet_radius, 3) / pow(30, 3);
        List *planet_shape = make_ngon(20, planet_radius);
        Body *planet = body_init(planet_shape, mass, COLOR_BLUE);
        body_set_radius(planet, planet_radius);
        body_set_image(planet, PLANET_IMAGES[i]);
        body_set_centroid(planet, (Vector) {x_pos, y_pos});
        body_set_velocity(planet, VEC_ZERO);
        scene_add_body(scene, planet);
    }
}

void create_asteroids(Scene *scene) {
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        int which = rand() % 2;
        double y_pos = ASTEROID_Y[i];
        double x_pos = ASTEROID_X[i];
        List *asteroid_shape = make_ngon(10, 8);
        Body *asteroid = body_init(asteroid_shape, ASTEROID_MASS, COLOR_YELLOW);
        body_set_radius(asteroid, 8);
        if (which) {
          body_set_image(asteroid, "images/asteroid10000.png");
        }
        else {
          body_set_image(asteroid, "images/asteroid20000.png");
        }
        body_set_centroid(asteroid, (Vector) {x_pos, y_pos});
        body_set_velocity(asteroid, VEC_ZERO);
        scene_add_body(scene, asteroid);
    }
}

void create_aliens(Scene *scene) {
    for (int i = 0; i < NUM_ALIENS; i++) {
        double y_pos = ALIENS_Y[i];
        double x_pos = ALIENS_X[i];
        List *alien_shape = make_ngon(10, 5);
        Body *alien = body_init(alien_shape, SMALL_MASS, COLOR_RED);
        body_set_radius(alien, 5);
        body_set_image(alien, "images/alien0000.png");
        body_set_centroid(alien, (Vector) {x_pos, y_pos});
        body_set_velocity(alien, ASTEROID_INIT_VEL);
        scene_add_body(scene, alien);
    }
}

void create_black_holes(Scene *scene) {
    for (int i = 0; i < NUM_BLACK_HOLES; i++) {
        double y_pos = BLACK_HOLE_Y[i];
        double x_pos = BLACK_HOLE_X[i];
        List *black_hole_shape = make_ngon(10, 20);
        Body *black_hole = body_init(black_hole_shape, BLACK_HOLE_MASS, COLOR_WHITE);
        body_set_radius(black_hole, 20);
        body_set_image(black_hole, "images/blackhole0000.png");
        body_set_centroid(black_hole, (Vector) {x_pos, y_pos});
        body_set_velocity(black_hole, VEC_ZERO);
        scene_add_body(scene, black_hole);
    }
}

void create_temp_stars(Scene *scene) {
    for (int i = 0; i < NUM_STARS; i++) {
        double y_pos = rand() % CANVAS_HEIGHT - CANVAS_HEIGHT/2;
        double x_pos = rand() % CANVAS_WIDTH - CANVAS_WIDTH/2;
        List *black_hole_shape = make_ngon(5, 0.5);
        Body *black_hole = body_init(black_hole_shape, BLACK_HOLE_MASS, COLOR_WHITE);
        body_set_centroid(black_hole, (Vector) {x_pos, y_pos});
        body_set_velocity(black_hole, VEC_ZERO);
        scene_add_body(scene, black_hole);
    }
}

void move_ship(char key, KeyEventType type, double held_time, Scene *scene) {
    Body *ship = scene_get_body(scene, SHIP_INDEX);
    Vector velocity = body_get_velocity(ship);
    double speed = vec_len(velocity);
    double angle = vec_to_angle(velocity);

    if (type == KEY_PRESSED) {
        switch (key) {
            case RIGHT_ARROW:
                body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle - M_PI/2));
                if (speed > SHIP_MAX_SPEED) body_add_impulse(ship, polar_to_cartesian(- 0.13 * IMPULSE_MAGNITUDE, angle));
                break;
            case LEFT_ARROW:
                body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle + M_PI/2));
                if (speed > SHIP_MAX_SPEED) body_add_impulse(ship, polar_to_cartesian(- 0.13 * IMPULSE_MAGNITUDE, angle));
                break;
            case UP_ARROW:
                if (speed < SHIP_MAX_SPEED) {
                    body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle));
                }
                break;
            case DOWN_ARROW:
                if (speed > 10) {
                    body_add_impulse(ship, polar_to_cartesian(-IMPULSE_MAGNITUDE, angle));
                } else {
                    body_add_impulse(ship, polar_to_cartesian(-IMPULSE_MAGNITUDE * speed / 11, angle));
                }
                break;
            }
        if (vec_dot(velocity, (Vector){1, 0}) < 0) {
            body_set_image(ship, "images/birdfireleft.png");
        } else {
            body_set_image(ship, "images/birdfireright.png");
        }
    }
    // if (type == KEY_PRESSED) {
    //     Vector impulse;
    //     switch (key) {
    //         case RIGHT_ARROW:
    //             impulse = polar_to_cartesian(IMPULSE_MAGNITUDE, 0);
    //             if (SHIP_MAX_SPEED > speed || vec_dot(impulse, velocity) < 0) body_add_impulse(ship, impulse);
    //             break;
    //         case LEFT_ARROW:
    //             impulse = polar_to_cartesian(IMPULSE_MAGNITUDE, M_PI);
    //             if (SHIP_MAX_SPEED > speed || vec_dot(impulse, velocity) < 0) body_add_impulse(ship, impulse);
    //             break;
    //         case UP_ARROW:
    //             impulse = polar_to_cartesian(IMPULSE_MAGNITUDE, M_PI / 2);
    //             if (SHIP_MAX_SPEED > speed || vec_dot(impulse, velocity) < 0) body_add_impulse(ship, impulse);
    //             break;
    //         case DOWN_ARROW:
    //             impulse = polar_to_cartesian(IMPULSE_MAGNITUDE, M_PI * 3/2);
    //             if (SHIP_MAX_SPEED > speed || vec_dot(impulse, velocity) < 0) body_add_impulse(ship, impulse);
    //             break;
    //     }
    //     if (vec_dot(velocity, (Vector){1, 0}) < 0) {
    //         body_set_image(ship, "images/birdfireleft.png");
    //     } else {
    //         body_set_image(ship, "images/birdfireright.png");
    //     }
    // }
    if (type == KEY_RELEASED) {
        /* Set velocity of ship to 0 only if left or right arrow keys
           are released. */
        switch (key) {
            case RIGHT_ARROW:
            case UP_ARROW:
            case DOWN_ARROW:
            case LEFT_ARROW:
                // body_set_velocity(scene_get_body(scene, SHIP_INDEX), VEC_ZERO);
                break;
        }
        if (vec_dot(velocity, (Vector){1, 0}) < 0) {
            body_set_image(ship, "images/birdleft.png");
        } else {
            body_set_image(ship, "images/birdright.png");
        }
      }
}


void wait_until_space(char key, KeyEventType type, double held_time, Scene *scene) {
    if (type == KEY_PRESSED) {
        switch (key) {
            case SPACEBAR:
                camera_turn_on(scene_get_camera(scene));
                break;
        }
    }
}

void reset_game(Scene *scene) {
    Body *ship = scene_get_body(scene, SHIP_INDEX);
    *(bool *)body_get_info(ship) = true;
    body_set_centroid(ship, INITIAL_POSITION);
    body_set_velocity(ship, VEC_ZERO);
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        double y_pos = ASTEROID_Y[i];
        double x_pos = ASTEROID_X[i];
        Body *asteroid = scene_get_body(scene, FIRST_ASTEROID_INDEX + i);
        body_set_centroid(asteroid, (Vector) {x_pos, y_pos});
        body_set_velocity(asteroid, VEC_ZERO);
    }
    for (int i = 0; i < NUM_ALIENS; i++) {
        double y_pos = ALIENS_Y[i];
        double x_pos = ALIENS_X[i];
        Body *alien = scene_get_body(scene, FIRST_ALIEN_INDEX + i);
        body_set_centroid(alien, (Vector) {x_pos, y_pos});
        body_set_velocity(alien, ASTEROID_INIT_VEL);
    }
}

int main(int argc, char *argv[]) {
    srand(time(0));
    Vector bottom_left = (Vector) {-CANVAS_WIDTH/2, -CANVAS_HEIGHT/2};
    Vector top_right = (Vector) {CANVAS_WIDTH/2, CANVAS_HEIGHT/2};
    sdl_init(bottom_left, top_right);

    Scene *intro_scene = scene_init();
    camera_turn_off(scene_get_camera(intro_scene));
    sdl_on_key((KeyHandler) wait_until_space);
    scene_set_bkg_image(intro_scene, "images/welcome/background.png");
    Body *instructions = create_instructions(intro_scene);

    Scene *win_scene = scene_init();
    create_large_planet(win_scene, (RGBColor){88/255.0, 206/255.0, 137/255.0});
    Body *win_ship = create_win_ship(win_scene);
    camera_turn_off(scene_get_camera(win_scene));

    Scene *game_scene = scene_init();
    create_background_tiles(game_scene, COLOR_BLACK);
    create_temp_stars(game_scene);
    Body *ship = create_ship(game_scene);
    Body *habitable_planet = create_habitable_planet(game_scene);
    create_planets(game_scene);
    create_black_holes(game_scene);
    create_asteroids(game_scene);
    create_aliens(game_scene);

    scene_set_bkg_image(game_scene, "images/sky/sky_full.png");
    scene_set_bkg_image(win_scene, "images/sky/sky_full.png");

    Camera *camera = scene_get_camera(game_scene);
    camera_set_zoom(camera, MIN_ZOOM);
    camera_set_position(camera, body_get_centroid(ship));

    double elapsed = 0;
    double total_time = 0;
    bool isOver = false;

    // add forces here
    for (int i = HABITABLE_PLANET_INDEX; i < FIRST_BLACK_HOLE_INDEX; i ++) {
        Body *planet = scene_get_body(game_scene, i);
        create_wrapping_newtonian_gravity(game_scene, G, planet, ship, (Vector){CANVAS_WIDTH, CANVAS_HEIGHT});
        create_ship_body_collision(game_scene, ship, planet, NULL, NULL);
    }
    for (int i = FIRST_BLACK_HOLE_INDEX; i < FIRST_BLACK_HOLE_INDEX + NUM_BLACK_HOLES; i ++) {
        Body *black_hole = scene_get_body(game_scene, i);
        create_wrapping_newtonian_gravity(game_scene, G, black_hole, ship, (Vector){CANVAS_WIDTH, CANVAS_HEIGHT});
        create_ship_body_collision(game_scene, ship, black_hole, NULL, NULL);
    }
    for (int i = HABITABLE_PLANET_INDEX; i < FIRST_BLACK_HOLE_INDEX; i ++) {
        Body *planet = scene_get_body(game_scene, i);
        for (int j = FIRST_ASTEROID_INDEX; j < FIRST_ALIEN_INDEX; j ++) {
            Body *asteroid = scene_get_body(game_scene, j);
            create_wrapping_newtonian_gravity(game_scene, G, planet, asteroid, (Vector){CANVAS_WIDTH, CANVAS_HEIGHT});
        }
    }
    for (int i = FIRST_BLACK_HOLE_INDEX; i < FIRST_BLACK_HOLE_INDEX + NUM_BLACK_HOLES; i ++) {
        Body *black_hole = scene_get_body(game_scene, i);
        for (int j = FIRST_ASTEROID_INDEX; j < FIRST_ALIEN_INDEX; j ++) {
            Body *asteroid = scene_get_body(game_scene, j);
            create_wrapping_newtonian_gravity(game_scene, G, black_hole, asteroid, (Vector){CANVAS_WIDTH, CANVAS_HEIGHT});
        }
    }
    for (int i = FIRST_ALIEN_INDEX; i < FIRST_ALIEN_INDEX + NUM_ALIENS; i++) {
        Body *alien = scene_get_body(game_scene, i);
        create_ship_body_collision(game_scene, ship, alien, NULL, NULL);
    }
    for (int j = FIRST_ASTEROID_INDEX; j < FIRST_ALIEN_INDEX; j ++) {
        Body *asteroid = scene_get_body(game_scene, j);
        create_ship_body_collision(game_scene, ship, asteroid, NULL, NULL);
    }


    /* Uncomment this if you want to see the whole scene at once. */
    // camera_turn_off(camera);
    SDL_image_toggle();

    sdl_init_textures(game_scene);
    sdl_init_textures(intro_scene);
    sdl_init_textures(win_scene);

    Scene *current_scene = intro_scene;
    // Scene *current_scene = game_scene;
    // sdl_on_key((KeyHandler) move_ship);

    while (!sdl_is_done()) {
        double dt = time_since_last_tick();
        total_time += dt;

        if (current_scene == intro_scene) {
            sdl_render_scene(intro_scene);
            int t1 = round (1000 * (total_time + 1000));
            int t2 = round (1000 * (-total_time + 1000));
            double mag = 250;
            mag += (t1 % 1000 > t2 % 1000) ? 0.1 * (t2 % 1000) : 0.1 * (t1 % 1000);
            body_set_radius(instructions, mag);
            if (camera_is_on(scene_get_camera(intro_scene))) {
                camera_turn_off(scene_get_camera(intro_scene));
                current_scene = game_scene;
                sdl_render_scene(game_scene);
                sdl_on_key((KeyHandler) move_ship);
            }
        }
        else if (current_scene == win_scene) {
            double y_pos = body_get_centroid(win_ship).y;
            if (y_pos > -240) {
                body_set_velocity(win_ship, (Vector) {0, -400 * (y_pos + 350) / 1850});
                body_set_image(win_ship, "images/birdfireright.png");
            } else {
                body_set_velocity(win_ship, VEC_ZERO);
                body_set_image(win_ship, "images/birdright.png");
            }
            scene_tick(win_scene, dt);
            sdl_render_scene(win_scene);
        }
        else if (current_scene == game_scene) {
            elapsed += dt;

            /* Win condition. */
            double min_distance = body_get_radius(ship) + body_get_radius(habitable_planet) + 5;
            if (vec_len(displacement(body_get_centroid(habitable_planet), body_get_centroid(ship), CANVAS_WIDTH, CANVAS_HEIGHT)) < min_distance) {
                if (vec_len(body_get_velocity(ship)) < 20) {
                    current_scene = win_scene;
                    sdl_on_key(NULL);
                }
            }

            if (elapsed > DT && !isOver) {

                body_set_rotation(ship, vec_to_angle(body_get_velocity(ship)));
                double zoom = camera_get_zoom(camera);
                double correct_zoom = MAX_ZOOM - (MAX_ZOOM - MIN_ZOOM) * vec_len(body_get_velocity(ship)) / SHIP_MAX_SPEED;
                camera_set_zoom(camera, zoom + (correct_zoom - zoom) * ZOOM_DELAY);

                Vector camera_position = camera_get_position(camera);
                Vector correct_camera_position = vec_add(body_get_centroid(ship), vec_multiply(1.5, body_get_velocity(ship)));
                camera_set_position(camera, vec_add(camera_position, vec_multiply(0.01, vec_subtract(correct_camera_position, camera_position))));
                // camera_set_position(camera, body_get_centroid(ship));
                /* Wrap player around screen. */
                for (int i = 0; i < scene_bodies(game_scene); i ++) {
                    Body *body = scene_get_body(game_scene, i);
                    Vector body_position = body_get_centroid(body);
                    double new_x = body_position.x;
                    if (new_x > CANVAS_WIDTH / 2) new_x -= CANVAS_WIDTH;
                    if (new_x < -CANVAS_WIDTH / 2) new_x += CANVAS_WIDTH;
                    double new_y = body_position.y;
                    if (new_y > CANVAS_HEIGHT / 2) new_y -= CANVAS_HEIGHT;
                    if (new_y < -CANVAS_HEIGHT / 2) new_y += CANVAS_HEIGHT;
                    if (body == ship) {
                        camera_set_position(camera, vec_add(camera_get_position(camera), vec_subtract((Vector) {new_x, new_y}, body_get_centroid(ship))));
                    }
                    body_set_centroid(body, (Vector) {new_x, new_y});
                }

                /* Tracking enemy */
                for (int i = FIRST_ALIEN_INDEX; i < FIRST_ALIEN_INDEX + NUM_ALIENS; i++) {
                    Body *alien = scene_get_body(game_scene, i);
                    Vector d = displacement(body_get_centroid(ship), body_get_centroid(alien), CANVAS_WIDTH, CANVAS_HEIGHT);
                    if (vec_len(d) <= TRACKING_DISTANCE) {
                        body_set_velocity(alien, vec_multiply((ALIEN_TRACKING_SPEED) / vec_len(d), d));
                    }
                }

                scene_tick(game_scene, elapsed);
                sdl_render_scene(game_scene);
                elapsed = 0;
            }

            if(!*(bool *)body_get_info(ship)){
                reset_game(game_scene);
                current_scene = intro_scene;
                sdl_on_key((KeyHandler) wait_until_space);
            }
        }
    }
    scene_free(intro_scene);
    scene_free(game_scene);
    scene_free(win_scene);
}
