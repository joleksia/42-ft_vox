#include "ft_vox.h"

extern int ft_camera_move(struct camera_s *self, const float value) {
    /* null-check */
    if (!self) { return (0); }

    if (STATE.input.keyboard->current[SDL_SCANCODE_D] ||
        STATE.input.keyboard->current[SDL_SCANCODE_A]
    ) {
        int8_t input = STATE.input.keyboard->current[SDL_SCANCODE_D] -
                       STATE.input.keyboard->current[SDL_SCANCODE_A];
        vec3 norm = alg_vec3norm(alg_vec3cross(self->rotation, self->up));
        self->position = alg_vec3add(self->position, alg_vec3mulf(norm, input * value));
    }
    
    if (STATE.input.keyboard->current[SDL_SCANCODE_SPACE] ||
        STATE.input.keyboard->current[SDL_SCANCODE_LSHIFT]
    ) {
        int8_t input = STATE.input.keyboard->current[SDL_SCANCODE_SPACE] -
                       STATE.input.keyboard->current[SDL_SCANCODE_LSHIFT];
        self->position = alg_vec3add(self->position, alg_vec3mulf(self->up, input * value));
    }
    
    if (STATE.input.keyboard->current[SDL_SCANCODE_W] ||
        STATE.input.keyboard->current[SDL_SCANCODE_S]
    ) {
        int8_t input = STATE.input.keyboard->current[SDL_SCANCODE_W] -
                       STATE.input.keyboard->current[SDL_SCANCODE_S];

        double sy = sin(alg_deg2rad(self->direction.z)),
               cy = cos(alg_deg2rad(self->direction.z));
        vec3 forward = alg_vec3(cy, 0.0f, sy);
        self->position = alg_vec3add(self->position, alg_vec3mulf(forward, input * value));
    }

    /* success */
    return (1);
}


extern int ft_camera_rotate(struct camera_s *self, const float value) {
    /* null-check */
    if (!self) { return (0); }

    /* apply 'offset' to 'direction' */
    self->direction.z += STATE.input.mouse->position.delta.x * value; /* yaw */
    self->direction.y -= STATE.input.mouse->position.delta.y * value; /* pitch */
    
    /* clear 'STATE.input.mouse' 'delta' */
    STATE.input.mouse->position.delta = alg_vec2(0.0, 0.0);

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


extern int ft_camera_matrix(struct camera_s *self, mat4 *proj, mat4 *view) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'Window's 'size' */
    vec2 size = { 0 };
    ft_window_get_size(STATE.window, (int *) &size.x, (int *) &size.y);

    /* get 'proj' matrix */
    if (proj) {
        *proj = alg_mat4persp(alg_deg2rad(self->fieldOfView), size.x / size.y, 0.001f, 65536.0f);
    }

    /* get 'view' matrix */
    if (view) {
        vec3 v_target = alg_vec3add(self->position,
                                    self->rotation);
        *view = alg_mat4lookat(self->position, v_target, self->up);
    }

    /* success */
    return (1);
}
