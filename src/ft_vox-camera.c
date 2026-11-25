#include "ft_vox.h"

extern vec3 ft_camera_move(struct camera_s *self, struct keyboard_s *keyboard, const float value) {
    /* get 'input' inputs */
    u8 *key= keyboard->current;

    /* move : 'left-right' */
    vec3 lr = { 0 };
    if (key[SDL_SCANCODE_D] ||
        key[SDL_SCANCODE_A]
    ) {
        i8 input = key[SDL_SCANCODE_D] -
                   key[SDL_SCANCODE_A];
        lr = alg_vec3cross(self->rotation, self->up);
        lr = alg_vec3norm(lr);
        lr = alg_vec3mulf(lr, input * value);
    }
    
    /* move : 'top-down' */
    vec3 td = { 0 };
    if (key[SDL_SCANCODE_SPACE] ||
        key[SDL_SCANCODE_LSHIFT]
    ) {
        i8 input = key[SDL_SCANCODE_SPACE] -
                   key[SDL_SCANCODE_LSHIFT];
        td = alg_vec3mulf(self->up, input * value);
    }
   
    /* move : 'forward-backward' */
    vec3 fb = { 0 };
    if (key[SDL_SCANCODE_W] ||
        key[SDL_SCANCODE_S]
    ) {
        i8 input = key[SDL_SCANCODE_W] -
                   key[SDL_SCANCODE_S];

        double sy = sin(alg_deg2rad(self->direction.z)),
               cy = cos(alg_deg2rad(self->direction.z));
        fb = alg_vec3(cy, 0.0f, sy);
        fb = alg_vec3mulf(fb, input * value);
    }

    /* success */
    vec3 move = alg_vec3add(lr, td);
         move = alg_vec3add(fb, move);
    vec3 norm = alg_vec3norm(move);
         norm = alg_vec3mulf(norm, value);
    return (alg_vec3add(self->position, norm));
}


extern vec3 ft_camera_rotate(struct camera_s *self, struct mouse_s *mouse, const float value) {
    /* apply 'offset' to 'direction' */
    self->direction.z += mouse->position.delta.x * value; /* yaw */
    self->direction.y -= mouse->position.delta.y * value; /* pitch */
    
    /* clear 'mouse's 'delta' */
    mouse->position.delta = alg_vec2(0.0, 0.0);

    /* clamp 'pitch' in [-89.9, 89.9] */
    self->direction.y = alg_clamp(self->direction.y, -89.9, 89.9);

    /* apply 'direction' to 'rotation' */
    double sp = sin(alg_deg2rad(self->direction.y)),
           cp = cos(alg_deg2rad(self->direction.y)),
           sy = sin(alg_deg2rad(self->direction.z)),
           cy = cos(alg_deg2rad(self->direction.z));
    vec3 rotation = alg_vec3(cy * cp, sp, sy * cp);

    /* success */
    return (alg_vec3norm(rotation));
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
