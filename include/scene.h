#ifndef __SCENE_H__
#define __SCENE_H__

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "body.h"
#include "list.h"
#include "camera.h"

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene Scene;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*ForceCreator)(void *aux);

/**
 * A struct that packages a force creator and its associated bodies.
 * Contains a force creator and two associated bodies, which may be
 * null if the force creator acts on <2 bodies.
 */
typedef struct force_object ForceObj;

/**
 * Initializes a ForceObj with the given force creator and bodies.
 *
 * @param f the force creators
 * @param b1 one body to which the force creator is applied
 * @param b2 the second body (may be null)
 * @return the force object
 */
ForceObj *forceobj_init(ForceCreator f, Body *b1, Body *b2);

/**
 * Sets either assoc_body1 or assoc_body2 of a force creator to
 * be a given body. Which body is set is given in bodynum.
 *
 * @param f the force creator to be changed
 * @param b the body to be added to the force creator
 * @param bodynum which body the new body will replace. 1 for
 *        assoc_body1, 2 for assoc_body2
 */
void forceobj_setbody(ForceObj *f, Body *b, size_t bodynum);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
Scene *scene_init(void);

/**
 * Releases memory allocated for a given scene
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(Scene *scene);

/**
 * Sets the scene background to an SDL_Texture.
 *
 * @param scene the scene to be set
 * @param texture the texture to add
 */
void scene_set_bkg(Scene *scene, SDL_Texture *texture);

/**
 * Returns the background texture of the scene.
 */
SDL_Texture *scene_get_bkg(Scene *scene);

/**
 * Sets the scene background image to an SDL_Surface.
 *
 * @param scene the scene to be set
 * @param filename the filename to read the image from
 */
void scene_set_bkg_image(Scene *scene, const char *filename);

/**
 * Returns the background surface of the scene.
 */
SDL_Surface *scene_get_bkg_image(Scene *scene);

/**
 * Adds a texture to the list of scene textures.
 */
void scene_add_texture(Scene *scene, SDL_Texture *texture);

/**
 * Returns the list of scene textures.
 */
List *scene_textures_list(Scene *scene);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(Scene *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
Body *scene_get_body(Scene *scene, size_t index);

/**
* Returns the list of bodies in the scene.
*/
List *scene_bodies_list(Scene *scene);

/**
 * Returns the camera assosiated with the scene.
 */
Camera *scene_get_camera(Scene *scene);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(Scene *scene, Body *body);

/**
 * @deprecated Use body_remove() instead
 *
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(Scene *scene, size_t index);

/**
 * @deprecated Use scene_add_bodies_force_creator() instead
 * so the scene knows which bodies the force creator depends on
 */
void scene_add_force_creator(
    Scene *scene, ForceCreator forcer, void *aux, FreeFunc freer
);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 * The force creator is registered with a list of bodies it applies to,
 * so it can be removed when any one of the bodies is removed.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param bodies the list of bodies affected by the force creator.
 *   The force creator will be removed if any of these bodies are removed.
 *   This list does not own the bodies, so its freer should be NULL.
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_bodies_force_creator(
    Scene *scene, ForceCreator forcer, void *aux, List *bodies, FreeFunc freer
);

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 * If any bodies are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(Scene *scene, double dt);

#endif // #ifndef __SCENE_H__
