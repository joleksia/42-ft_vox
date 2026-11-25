#include "ft_vox.h"

struct player_s {
    /* 'parent' : parent reference */
    struct
    ecs_s *parent;

    /* 'id' : ID of this entity, used for O(1) look-up into 'parent' */
    size_t id;

    /* methods used by this entity */
    int (*create) (struct entity_s *);
    int (*delete) (struct entity_s *);
    int (*update) (struct entity_s *);
    int (*render) (struct entity_s *);

    /* 'player_s' specific */

    /* 'camera' : 3D perspective projection */
    struct
    camera_s camera;

    /* 'position' : 3D position */
    vec3 position;

    /* 'block' : 3D position (block-specific) */
    vec3 block;
};


static int ft_player_create(struct entity_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'player_s' */
    struct
    player_s *player = (struct player_s *) self;
    if (!player) { return (0); }

    /* create 'camera' */
    player->camera = (struct camera_s) {
        .position = alg_vec3(0.0, 0.0, 0.0),
        .rotation = alg_vec3(0.0, 0.0, -1.0),
        .direction = alg_vec3(0.0, 0.0, -90.0),
        .up = alg_vec3(0.0, 1.0, 0.0),
        .fieldOfView = 60.0,
    };

    /* success */
    return (1);
}


static int ft_player_delete(struct entity_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'player_s' */
    struct
    player_s *player = (struct player_s *) self;
    if (!player) { return (0); }

    /* ... */ (void) player;

    /* success */
    return (1);
}


static int ft_player_update(struct entity_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'player_s' */
    struct
    player_s *player = (struct player_s *) self;
    if (!player) { return (0); }

    /* get 'input's */
    struct
    keyboard_s *keyboard = STATE.input.keyboard;
    if (!keyboard) { return (0); }

    struct
    mouse_s *mouse = STATE.input.mouse;
    if (!mouse) { return (0); }

    /* get 'time' */
    struct
    time_s *time = STATE.time;
    if (!time) { return (0); }
        
    /* enable 'locked' mode */
    if (mouse->input.current[1]  == 1 &&
        mouse->input.previous[1] == 0
    ) {
        mouse->locked = 1;
        SDL_SetRelativeMouseMode(mouse->locked);
    }

    /* disable 'locked' mode */
    if (keyboard->current[SDL_SCANCODE_ESCAPE]  == 1 &&
        keyboard->previous[SDL_SCANCODE_ESCAPE] == 0
    ) {
        mouse->locked = 0;
        SDL_SetRelativeMouseMode(mouse->locked);
    }

    /* update 'camera' */
    if (mouse->locked) {
        struct
        camera_s *camera = &player->camera;
        camera->position = ft_camera_move(camera, keyboard, 16.0 * time->delta);
        camera->rotation = ft_camera_rotate(camera, mouse, 0.1);
    }

    /* update 'player's 'position' */
    vec3 position = player->position = player->camera.position;

    /* 'player' position to 'block' position */
    vec3 block = FT_VOX_FLOAT_TO_BLOCK(position.x,
                                       position.y,
                                       position.z);

    /* update 'player's positions */
    player->block = block;
    player->position = position;

    /* change 'parent's origin */
    ft_world_center(self->parent->parent, block);

    /* success */
    return (1);
}


static int ft_player_render(struct entity_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'player_s' */
    struct
    player_s *player = (struct player_s *) self;
    if (!player) { return (0); }
   
    /* create 'u_proj' and 'u_view' projection matrices */
    mat4 u_proj = alg_mat4(1.0),
         u_view = alg_mat4(1.0);
    ft_camera_matrix(&player->camera, &u_proj, &u_view);
    STATE.gfx.proj = u_proj;
    STATE.gfx.view = u_view;

    /* success */
    return (1);
}


int main(int ac, char **av) {

    /* create 'window' */
    
    ft_window_create(&STATE.window, "42warsaw - ft_vox", 800, 600);
    
    /* create 'time' */

    ft_time_create(&STATE.time);
   
    /* create 'input' objects */
    
    ft_keyboard_create(&STATE.input.keyboard);
    ft_mouse_create(&STATE.input.mouse);

    /* create blocks */

    ft_blocks_create();

    /* create 'shader's */

    if (ft_shader_create(&STATE.gfx.solid)) {
        FILE *fp_vertex_glsl = fopen("src/glsl/vsolid.glsl", "rb");
        ft_shader_load(STATE.gfx.solid, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
        
        FILE *fp_fragment_glsl = fopen("src/glsl/fsolid.glsl", "rb");
        ft_shader_load(STATE.gfx.solid, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    if (ft_shader_create(&STATE.gfx.trans)) {
        FILE *fp_vertex_glsl = fopen("src/glsl/vtrans.glsl", "rb");
        ft_shader_load(STATE.gfx.trans, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
        
        FILE *fp_fragment_glsl = fopen("src/glsl/ftrans.glsl", "rb");
        ft_shader_load(STATE.gfx.trans, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    if (ft_shader_create(&STATE.gfx.water)) {
        FILE *fp_vertex_glsl = fopen("src/glsl/vwater.glsl", "rb");
        ft_shader_load(STATE.gfx.water, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
        
        FILE *fp_fragment_glsl = fopen("src/glsl/fwater.glsl", "rb");
        ft_shader_load(STATE.gfx.water, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    /* load 'atlas1.png' sampler */
   
    ft_sampler_create(&STATE.gfx.atlas1, "res/atlas/atlas1.png");
    ft_sampler_bind(STATE.gfx.atlas1, 0);

    /* create 'world' */

    i32 seed = 0;
    if (ac > 1) {
        seed = atoi(av[1]);
    } else {
        srand(time(0));
        seed = rand();
    }
    ft_world_create(&STATE.world, seed);

    /* create 'player' entity */
    struct
    player_s *player = calloc(1, sizeof(struct player_s));
    player->create = ft_player_create;
    player->delete = ft_player_delete;
    player->update = ft_player_update;
    player->render = ft_player_render;
    ft_ecs_insert(STATE.world->ecs, (struct entity_s *) player);

    /* update-render loop */

    do {

        /* update */

        /* update 'world' */
        ft_world_update(STATE.world);

        /* render */

        /* clear */
        glClearColor(99.0 / 255.0, 149.0 / 255.0, 238.0 / 255.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        /* render 'world' */
        ft_world_render(STATE.world);

        /* poll-events */
        ft_window_poll_events(STATE.window);

    } while (!STATE.window->flags.close);

    /* unload 'world' */
   
    ft_world_delete(STATE.world);

    /* unload OpenGL */

    ft_sampler_delete(STATE.gfx.atlas1);
    ft_shader_delete(STATE.gfx.solid);
    ft_shader_delete(STATE.gfx.trans);
    ft_shader_delete(STATE.gfx.water);

    /* unload input objects */
    ft_mouse_delete(STATE.input.mouse);
    ft_keyboard_delete(STATE.input.keyboard);
    ft_time_delete(STATE.time);

    /* quit */

    ft_window_delete(STATE.window);

    return (0);
}
