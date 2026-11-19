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
    int (*render) (struct entity_s *, struct vao_s *, struct shader_s *);

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
        
    /* enable 'locked' mode */
    if (MOUSE->input.current[1]  == 1 &&
        MOUSE->input.previous[1] == 0
    ) {
        MOUSE->locked = 1;
        SDL_SetRelativeMouseMode(MOUSE->locked);
    }

    /* disable 'locked' mode */
    if (KEYBOARD->current[SDL_SCANCODE_ESCAPE]  == 1 &&
        KEYBOARD->previous[SDL_SCANCODE_ESCAPE] == 0
    ) {
        MOUSE->locked = 0;
        SDL_SetRelativeMouseMode(MOUSE->locked);
    }

    /* update 'camera' */
    if (MOUSE->locked) {
        ft_camera_move(&player->camera, 128.0 * TIME->delta);
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


static int ft_player_render(struct entity_s *self, struct vao_s *vao, struct shader_s *shader) {
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
    ft_shader_uniform(shader, "u_proj", FT_UNIFORM_MATRIX4X4, &u_proj);
    ft_shader_uniform(shader, "u_view", FT_UNIFORM_MATRIX4X4, &u_view);

    /* 'vao' is unused */
    (void) vao;

    /* success */
    return (1);
}


int main(void) {

    /* create OpenGL window */
    
    if (!ft_window_create("42warsaw - ft_vox", 800, 600)) { return (1); }
   
    /* create input objects */
    
    ft_keyboard_create();
    ft_mouse_create();
    ft_time_create();

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

    /* create vertex array */

    struct
    vao_s *vao = 0;
    ft_vao_create(&vao);
    ft_vao_layouti(vao, 0, 1, 0);

    /* create 'world' */

    srand(time(0));
    
    struct
    world_s *world = (struct world_s *) calloc(1, sizeof(struct world_s));
    ft_world_create(world, rand());

    /* create 'player' entity */
    struct
    player_s *player = calloc(1, sizeof(struct player_s));
    player->create = ft_player_create;
    player->delete = ft_player_delete;
    player->update = ft_player_update;
    player->render = ft_player_render;
    ft_ecs_insert(world->ecs, (struct entity_s *) player);

    /* update-render loop */

    do {

        /* update */

        /* set new 'window' title */
        u32 fps = round(1.0 / TIME->delta);
        ft_window_title("42warsaw - ft_vox (FPS.: %d)", fps);

        /* update 'world' */
        ft_world_update(world);

        /* render */

        /* clear */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        /* render current vertex array */
        glUseProgram(shader->id);
        glBindVertexArray(vao->id);

        /* render 'world' */
        ft_world_render(world, vao, shader);

        glBindVertexArray(0);
        glUseProgram(0);

        /* poll-events */
        ft_window_poll_events();

    } while (!WINDOW->flags.close);

    /* unload 'world' */
    ft_world_delete(world);

    /* unload OpenGL */

    ft_shader_delete(shader);
    ft_vao_delete(vao);
    
    /* unload input objects */
    
    ft_time_delete();
    ft_mouse_delete();
    ft_keyboard_delete();

    /* quit */

    ft_window_delete();

    return (0);
}
