#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <stdbool.h>
#include "list.h"
#include "vector.h"
#include "body.h"

/**
 * Represents the status of a collision between two shapes.
 * The shapes are either not colliding, or they are colliding along some axis.
 */
typedef struct {
    /** Whether the two shapes are colliding */
    bool collided;
    /**
     * If the shapes are colliding, the axis they are colliding on.
     * This is a unit vector pointing from the first shape towards the second.
     * Normal impulses are applied along this axis.
     * If collided is false, this value is undefined.
     */
    Vector axis;
} CollisionInfo;

/**
 * A function called when a collision occurs.
 * @param body1 the first body passed to create_collision()
 * @param body2 the second body passed to create_collision()
 * @param axis a unit vector pointing from body1 towards body2
 *   that defines the direction the two bodies are colliding in
 * @param aux the auxiliary value passed to create_collision()
 */
typedef void (*CollisionHandler)
    (Body *body1, Body *body2, Vector axis, void *aux);

// stores collision data
typedef struct collision_aux {
  Body *body1;
  Body *body2;
  CollisionHandler handler;
  void *aux;
  FreeFunc freer;
} ColAux;

// frees a colaux
void col_aux_free(ColAux *colaux);

/**
 * Computes the status of the collision between two convex polygons.
 * The shapes are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from shape1 towards shape2.
 */
CollisionInfo find_collision(List *shape1, List *shape2);

/**
 * Computes the status of the collision between two circle bodies.
 *
 * @param body1 the first body
 * @param body2 the second body
 * @return whether the bodies are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from body1 towards body2.
 */
CollisionInfo find_circle_body_collision(Body *body1, Body *body2);


#endif // #ifndef __COLLISION_H__
