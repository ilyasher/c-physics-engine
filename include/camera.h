#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "vector.h"

typedef struct camera Camera;

Vector camera_get_position(Camera *camera);

void camera_set_position(Camera *camera, Vector position);

double camera_get_zoom(Camera *camera);

void camera_set_zoom(Camera *camera, double zoom);

Camera *init_camera(void);

void camera_free(Camera *camera);

void camera_turn_on(Camera *camera);

void camera_turn_off(Camera *camera);

bool camera_is_on(Camera *camera);
