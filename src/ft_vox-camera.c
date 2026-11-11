#include "ft_vox.h"

extern int ftCameraMove(Camera *pCamera, const float value) {
    /* null-check */
    if (!pCamera) { return (0); }

    if (Keyboard->current[SDL_SCANCODE_D] ||
        Keyboard->current[SDL_SCANCODE_A]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_D] -
                       Keyboard->current[SDL_SCANCODE_A];
        vec3 norm = alg_vec3norm(alg_vec3cross(pCamera->rotation, pCamera->up));
        pCamera->position = alg_vec3add(pCamera->position, alg_vec3mulf(norm, input * value));
    }
    
    if (Keyboard->current[SDL_SCANCODE_SPACE] ||
        Keyboard->current[SDL_SCANCODE_LSHIFT]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_SPACE] -
                       Keyboard->current[SDL_SCANCODE_LSHIFT];
        pCamera->position = alg_vec3add(pCamera->position, alg_vec3mulf(pCamera->up, input * value));
    }
    
    if (Keyboard->current[SDL_SCANCODE_W] ||
        Keyboard->current[SDL_SCANCODE_S]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_W] -
                       Keyboard->current[SDL_SCANCODE_S];

        double sy = sin(alg_deg2rad(pCamera->direction.z)),
               cy = cos(alg_deg2rad(pCamera->direction.z));
        vec3 forward = alg_vec3(cy, 0.0f, sy);
        pCamera->position = alg_vec3add(pCamera->position, alg_vec3mulf(forward, input * value));
    }

    /* success */
    return (1);
}


extern int ftCameraRotate(Camera *pCamera, const float value) {
    /* null-check */
    if (!pCamera) { return (0); }

    /* set mouse to 'relative' if 'locked' set */
    SDL_SetRelativeMouseMode(Mouse->locked);

    /* if 'locked' set */
    if (Mouse->locked) {
        /* apply 'offset' to 'direction' */
        pCamera->direction.z += Mouse->position.delta.x * value; /* yaw */
        pCamera->direction.y -= Mouse->position.delta.y * value; /* pitch */
        
        /* clear 'Mouse' 'delta' */
        Mouse->position.delta = alg_vec2(0.0, 0.0);

        /* clamp 'pitch' in [-89.0, 89.0] */
        pCamera->direction.y = alg_clamp(pCamera->direction.y, -89.0, 89.0);

        /* apply 'direction' to 'rotation' */
        double sp = sin(alg_deg2rad(pCamera->direction.y)),
               cp = cos(alg_deg2rad(pCamera->direction.y)),
               sy = sin(alg_deg2rad(pCamera->direction.z)),
               cy = cos(alg_deg2rad(pCamera->direction.z));
        pCamera->rotation = alg_vec3norm(alg_vec3(cy * cp, sp, sy * cp));
    }

    /* success */
    return (1);
}


extern int ftCameraMatrix(Camera *pCamera, mat4 *pProj, mat4 *pView) {
    /* null-check */
    if (!pCamera) { return (0); }

    /* get 'Window's 'size' */
    vec2 size = { 0 };
    ftWindowGetSize((int *) &size.x, (int *) &size.y);

    /* get 'proj' matrix */
    mat4 proj = alg_mat4persp(alg_deg2rad(pCamera->fieldOfView), size.x / size.y, 0.001f, 65536.0f);

    /* get 'view' matrix */
    vec3 v_target = alg_vec3add(pCamera->position,
                                pCamera->rotation);
    mat4 view = alg_mat4lookat(pCamera->position, v_target, pCamera->up);

    /* return */
    if (pProj) { *pProj = proj; }
    if (pView) { *pView = view; }

    /* success */
    return (1);
}
