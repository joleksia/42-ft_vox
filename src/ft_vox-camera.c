#include "ft_vox.h"

extern int ftCameraMove(struct Camera *self, const float value) {
    /* null-check */
    if (!self) { return (0); }

    if (Keyboard->current[SDL_SCANCODE_D] ||
        Keyboard->current[SDL_SCANCODE_A]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_D] -
                       Keyboard->current[SDL_SCANCODE_A];
        vec3 norm = alg_vec3norm(alg_vec3cross(self->rotation, self->up));
        self->position = alg_vec3add(self->position, alg_vec3mulf(norm, input * value));
    }
    
    if (Keyboard->current[SDL_SCANCODE_SPACE] ||
        Keyboard->current[SDL_SCANCODE_LSHIFT]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_SPACE] -
                       Keyboard->current[SDL_SCANCODE_LSHIFT];
        self->position = alg_vec3add(self->position, alg_vec3mulf(self->up, input * value));
    }
    
    if (Keyboard->current[SDL_SCANCODE_W] ||
        Keyboard->current[SDL_SCANCODE_S]
    ) {
        int8_t input = Keyboard->current[SDL_SCANCODE_W] -
                       Keyboard->current[SDL_SCANCODE_S];

        double sy = sin(alg_deg2rad(self->direction.z)),
               cy = cos(alg_deg2rad(self->direction.z));
        vec3 forward = alg_vec3(cy, 0.0f, sy);
        self->position = alg_vec3add(self->position, alg_vec3mulf(forward, input * value));
    }

    /* success */
    return (1);
}


extern int ftCameraRotate(struct Camera *self, const float value) {
    /* null-check */
    if (!self) { return (0); }

    /* apply 'offset' to 'direction' */
    self->direction.z += Mouse->position.delta.x * value; /* yaw */
    self->direction.y -= Mouse->position.delta.y * value; /* pitch */
    
    /* clear 'Mouse' 'delta' */
    Mouse->position.delta = alg_vec2(0.0, 0.0);

    /* clamp 'pitch' in [-89.0, 89.0] */
    self->direction.y = alg_clamp(self->direction.y, -89.9, 89.9);

    /* apply 'direction' to 'rotation' */
    double sp = sin(alg_deg2rad(self->direction.y)),
           cp = cos(alg_deg2rad(self->direction.y)),
           sy = sin(alg_deg2rad(self->direction.z)),
           cy = cos(alg_deg2rad(self->direction.z));
    self->rotation = alg_vec3norm(alg_vec3(cy * cp, sp, sy * cp));

    /* success */
    return (1);
}


extern int ftCameraMatrix(struct Camera *self, mat4 *pProj, mat4 *pView) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'Window's 'size' */
    vec2 size = { 0 };
    ftWindowGetSize((int *) &size.x, (int *) &size.y);

    /* get 'proj' matrix */
    mat4 proj = alg_mat4persp(alg_deg2rad(self->fieldOfView), size.x / size.y, 0.001f, 65536.0f);

    /* get 'view' matrix */
    vec3 v_target = alg_vec3add(self->position,
                                self->rotation);
    mat4 view = alg_mat4lookat(self->position, v_target, self->up);

    /* return */
    if (pProj) { *pProj = proj; }
    if (pView) { *pView = view; }

    /* success */
    return (1);
}
