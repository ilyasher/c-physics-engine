#include <math.h>
#include "collision.h"

#define SMALL -1e20
#define LARGE 1e20

/**
 * Determines whether two convex polygons intersect.
 * The polygons are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding
 */

 void col_aux_free(ColAux *colaux){
   if(colaux->freer != NULL){
     colaux->freer(colaux->aux);
   }
   free(colaux);
 }

 CollisionInfo find_circle_body_collision(Body *body1, Body *body2){
     if(body_distance(body1, body2) < body_get_radius(body1) + body_get_radius(body2)){
         return (CollisionInfo){true, vec_subtract(body_get_centroid(body2), body_get_centroid(body1))};
     }
     return (CollisionInfo){false};
 }


 CollisionInfo find_collision(List *shape1, List *shape2) {

    /* How to find axes:
    For each polygon:
    For each adjacent pair of point, subtract the vectors,
    Rotate by 90 degrees, and then divide it by its length
    (to make it a unit vector).
    */


    double corners1[4] = {0, 0, 0, 0};
    double corners2[4] = {0, 0, 0, 0};
    for (size_t i = 0; i < list_size(shape1); i++) {
        Vector vec = *(Vector *)list_get(shape1, i);
        corners1[0] = (corners1[0] < vec.x) ? corners1[0] : vec.x;
        corners1[1] = (corners1[1] < vec.y) ? corners1[1] : vec.y;
        corners1[2] = (corners1[2] > vec.x) ? corners1[2] : vec.x;
        corners1[3] = (corners1[3] > vec.y) ? corners1[3] : vec.y;
    }
    for (size_t i = 0; i < list_size(shape2); i++) {
        Vector vec = *(Vector *)list_get(shape2, i);
        corners2[0] = (corners2[0] < vec.x) ? corners2[0] : vec.x;
        corners2[1] = (corners2[1] < vec.y) ? corners2[1] : vec.y;
        corners2[2] = (corners2[2] > vec.x) ? corners2[2] : vec.x;
        corners2[3] = (corners2[3] > vec.y) ? corners2[3] : vec.y;
    }
    if(((corners1[0]<corners2[2] && corners1[1]<corners2[3])
      && (corners1[2]>corners2[0] && corners1[3]>corners2[1]))){
        List *axes = list_init(list_size(shape1) + list_size(shape2), NULL);
        for (size_t i = 0; i < list_size(shape1) - 1; i ++) {
            Vector p1 = *((Vector *) list_get(shape1, i));
            Vector p2 = *((Vector *) list_get(shape1, i + 1));
            Vector unit_vec = vec_subtract(p1, p2);
            unit_vec = vec_rotate(unit_vec, M_PI / 2);
            unit_vec = vec_multiply(1 / vec_len(unit_vec), unit_vec);
            list_add(axes, vmalloc(unit_vec));
        }

        for (size_t i = 0; i < list_size(shape2) - 1; i ++) {
            Vector p1 = *((Vector *) list_get(shape2, i));
            Vector p2 = *((Vector *) list_get(shape2, i + 1));
            Vector unit_vec = vec_subtract(p1, p2);
            unit_vec = vec_rotate(unit_vec, M_PI / 2);
            unit_vec = vec_multiply(1 / vec_len(unit_vec), unit_vec);
            list_add(axes, vmalloc(unit_vec));
        }

        Vector collision_axis = VEC_ZERO;
        double min_overlap = LARGE;
        for (size_t i = 0; i < list_size(axes); i ++) {
            double min1 = LARGE, min2 = LARGE;
            double max1 = SMALL, max2 = SMALL;
            for (size_t j = 0; j < list_size(shape1); j ++) {
                Vector p1 = *((Vector *) list_get(axes, i));
                Vector p2 = *((Vector *) list_get(shape1, j));
                double dot = vec_dot(p1, p2);
                if (dot < min1) {
                  min1 = dot;
                }
                if (dot > max1) {
                  max1 = dot;
                }
            }
            for (size_t j = 0; j < list_size(shape2); j ++) {
                Vector p1 = *((Vector *) list_get(axes, i));
                Vector p2 = *((Vector *) list_get(shape2, j));
                double dot = vec_dot(p1, p2);
                if (dot < min2) {
                  min2 = dot;
                }
                if (dot > max2) {
                   max2 = dot;
                }
            }
            if (!(min1 < max2 && min2 < max1)) {
                return (CollisionInfo){false};
            }
            double temp1 = max1 - min2;
            double temp2 = max2 - min1;
            if (temp1 > 0 && temp1 < min_overlap){
                min_overlap = temp1;
                collision_axis = *((Vector *) list_get(axes, i));
            } else if (temp2 > 0 && temp2 < min_overlap){
                min_overlap = temp2;
                collision_axis = *((Vector *) list_get(axes, i));
            }
        }

        return (CollisionInfo){true, collision_axis};

        /* To find the projections:
        Dot the positions vectors of each vertex with the unit vector.
        Record the smallest and largest.
        */

        /*
        To check whether the projections intersect:
        Smallest1 < Largest2 AND Largest1 > Smallest2
        */

        /*
        For each axis, check whether the projections intersect.
        If at any point it does not intersect, then the polygons
        do not intersect.
        */

      }
      return (CollisionInfo){false};







}
