#include "camera.h"

typedef struct camera {
    Vector position;
    double zoom;
    bool on;
} Camera;

Vector camera_get_position(Camera *camera) {
    return camera->position;
}

void camera_set_position(Camera *camera, Vector position) {
    camera->position = position;
}

double camera_get_zoom(Camera *camera) {
    return camera->zoom;
}

void camera_set_zoom(Camera *camera, double zoom) {
    camera->zoom = zoom;
}

Camera *init_camera(void) {
    Camera *camera = malloc(sizeof(Camera));
    assert(camera);
    camera->position = VEC_ZERO;
    camera->zoom = 1;
    camera->on = true;
    return camera;
}

void camera_free(Camera *camera) {
    free(camera);
}

void camera_turn_on(Camera *camera) {
    camera->on = true;
}

void camera_turn_off(Camera *camera) {
    camera->on = false;
}

bool camera_is_on(Camera *camera) {
    return camera->on;
}
