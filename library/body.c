#include "body.h"
#include <stdio.h>

typedef struct body {
    List *points;
    double mass;
    double direction;
    double radius;
    RGBColor color;
    Vector velocity;
    Vector force;
    Vector impulse;
    double elasticity;
    double angular_velocity;
    Vector centroid;
    void *info;
    SDL_Surface *image;
    FreeFunc info_freer;
    bool remove;
    int depth;
    SDL_Texture *texture;
} Body;

Body *body_init(List *shape, double mass, RGBColor color) {
    return body_init_with_info(shape, mass, color, NULL, NULL);
}

Body *body_init_with_info(List *shape, double mass, RGBColor color,
                          void *info, FreeFunc info_freer) {

    Body *body = malloc(sizeof(Body));
    assert(body);

    body->points = shape;
    body->mass = mass;
    body->color = color;
    body->direction = 0;
    body->radius = 0;
    body->elasticity = 1;
    body->angular_velocity = 0;
    body->remove = 0;
    body->depth = 0;

    body->info = info;
    body->info_freer = info_freer;

    body->velocity = VEC_ZERO;
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;

    body->centroid = polygon_centroid(body->points);
    body->image = NULL;
    body->texture = NULL;

    return body;
}

void body_free(Body *body) {
    list_free(body->points);
    SDL_FreeSurface(body->image);
    if (body->info_freer) { body->info_freer(body->info); }
    free(body);
}

void body_set_image(Body *body, const char *filename) {
  SDL_Surface *image = IMG_Load(filename);
  body->image = image;
}

SDL_Surface *body_get_image(Body *body) {
  return body->image;
}

void body_set_texture(Body *body, SDL_Texture *texture) {
  body->texture = texture;
}

SDL_Texture *body_get_texture(Body *body) {
  return body->texture;
}

List *body_get_shape(Body *body) {
    List *copy = list_init(list_size(body->points), free);
    for (size_t i = 0; i < list_size(body->points); i++) {
        list_add(copy, vmalloc(*((Vector *)list_get(body->points, i))));
    }
    return copy;
}

Vector body_get_centroid(Body *body) {
    return body->centroid;
}

double body_get_direction(Body *body) {
    return body->direction;
}

double body_get_mass(Body *body) {
    return body->mass;
}

double body_get_radius(Body *body) {
  return body->radius;
}

Vector body_get_velocity(Body *body) {
    return body->velocity;
}

RGBColor body_get_color(Body *body) {
    return body->color;
}

void *body_get_info(Body *body) {
    return body->info;
}

void body_set_info(Body *body, void *inf){
  body->info = inf;
}


void body_set_color(Body *body, RGBColor color) {
    body->color = color;
}

void body_set_radius(Body *body, double radius) {
  body->radius = radius;
}

void body_vec_accelerate(Body *body, Vector v){
  body->velocity.x = body->velocity.x + v.x;
  body->velocity.y = body->velocity.y + v.y;
}

void body_double_accelerate(Body *body, double d){
  body_vec_accelerate(body, vec_rotate((Vector){d, 0}, body->direction));
}

void body_set_centroid(Body *body, Vector x) {
  polygon_translate(body->points, vec_subtract(x, body_get_centroid(body)));
  body->centroid = x;
}

void body_set_velocity(Body *body, Vector v) {
    body->velocity = v;
}

void body_add_force(Body *body, Vector force){
  body->force = vec_add(body->force, force);
}

void body_set_force(Body *body, Vector force){
  body->force = force;
}

Vector body_get_force(Body *body) {
  return body->force;
}

void body_set_depth(Body *body, int depth){
  body->depth = depth;
}

int body_get_depth(Body *body) {
  return body->depth;
}

Vector body_get_impulse(Body *body) {
  return body->impulse;
}

void body_add_impulse(Body *body, Vector impulse) {
    body->impulse = vec_add(body->impulse, impulse);
}

void body_set_rotation(Body *body, double angle) {
  polygon_rotate(body->points, angle - body->direction, polygon_centroid(body->points));
  body->direction = angle;
}

void body_tick(Body *body, double dt) {
    Vector old_vel = body->velocity;
    body_vec_accelerate(body, vec_multiply(1.0 / body->mass, body->impulse));
    body_vec_accelerate(body, vec_multiply(dt / body->mass, body->force));
    Vector ave_vel = vec_multiply(0.5, vec_add(old_vel, body->velocity));
    body_set_centroid(body, vec_add(body->centroid, vec_multiply(dt, ave_vel)));
    body->impulse = VEC_ZERO;
    body->force = VEC_ZERO;
}

void body_remove(Body *body) {
    body->remove = 1;
}

bool body_is_removed(Body *body) {
    return body->remove;
}

double body_distance(Body *b1, Body * b2){
  return vec_len(vec_subtract(body_get_centroid(b1), body_get_centroid(b2)));
}
