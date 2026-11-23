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
    int (*render) (struct entity_s *, struct shader_s *);

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
        ft_camera_move(&player->camera, 8.0 * time->delta);
        ft_camera_rotate(&player->camera, 0.1);
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


static int ft_player_render(struct entity_s *self, struct shader_s *shader) {
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
    STATE.proj = u_proj;
    STATE.view = u_view;
    (void) shader;

    /* success */
    return (1);
}


int main(int ac, char **av) {

    /* create OpenGL window */
    
    ft_window_create(&STATE.window, "42warsaw - ft_vox", 800, 600);
   
    /* create input objects */
    
    ft_time_create(&STATE.time);
    ft_keyboard_create(&STATE.input.keyboard);
    ft_mouse_create(&STATE.input.mouse);

    /* create blocks */

    ft_blocks_create();

    /* create 'shader' */

    struct
    shader_s *shader = 0;
    ft_shader_create(&shader);

    /* load 'vertex.glsl' */
    FILE *fp_vertex_glsl = fopen("src/glsl/vertex.glsl", "rb");
    if (fp_vertex_glsl) {
        ft_shader_load(shader, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
    }

    /* load 'fragment.glsl' */
    FILE *fp_fragment_glsl = fopen("src/glsl/fragment.glsl", "rb");
    if (fp_fragment_glsl) {
        ft_shader_load(shader, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    /* load 'atlas1.png' sampler */
   
    struct
    sampler_s *sampler = 0;
    ft_sampler_create(&sampler, "res/atlas/atlas1.png");

    /* set 'u_block_size' */
    vec2 u_block_size = { .x = FT_VOX_BLOCK_SIZE_X,
                          .y = FT_VOX_BLOCK_SIZE_Y };
    ft_shader_uniform(shader, "u_block_size", FT_UNIFORM_FLOAT2D, &u_block_size);

    /* set 'u_atlas_size' */
    vec2 u_atlas_size = { .x = sampler->width,
                          .y = sampler->height };
    ft_shader_uniform(shader, "u_atlas_size", FT_UNIFORM_FLOAT2D, &u_atlas_size);

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

        glUseProgram(shader->id);
        glBindTextureUnit(0, sampler->id);

        int slot = 0;
        ft_shader_uniform(shader, "u_atlas", FT_UNIFORM_INT1D, &slot);
        
        /* render 'world' */
        ft_world_render(STATE.world, shader);
        
        glBindTextureUnit(0, 0);
        glUseProgram(0);

        /* poll-events */
        ft_window_poll_events(STATE.window);

    } while (!STATE.window->flags.close);

    /* unload 'world' */
   
    ft_world_delete(STATE.world);

    /* unload OpenGL */

    ft_sampler_delete(sampler);
    ft_shader_delete(shader);

    /* unload input objects */
    ft_mouse_delete(STATE.input.mouse);
    ft_keyboard_delete(STATE.input.keyboard);
    ft_time_delete(STATE.time);

    /* quit */

    ft_window_delete(STATE.window);

    return (0);
}
