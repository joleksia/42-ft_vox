#include "ft_vox.h"

struct PlayerEntity {
    /* 'parent' : EntitySystem-parent object */
    struct
    EntitySystem *parent;

    /* 'ID' : ID of this entity, used for O(1) look-up into 'parent' */
    size_t ID;

    /* methods used by this entity */
    int (*create) (struct Entity *);
    int (*delete) (struct Entity *);
    int (*update) (struct Entity *);
    int (*render) (struct Entity *, struct VertexArray *, struct Shader *);

    /* 'PlayerEntity' specific */

    /* 'camera' : 3D perspective projection */
    struct
    Camera camera;

    /* 'position' : 3D position */
    vec3 position;

    /* 'block' : 3D position (block-specific) */
    vec3 block;
};


static int ftPlayerCreate(struct Entity *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'PlayerEntity' */
    struct
    PlayerEntity *player = (struct PlayerEntity *) self;
    if (!player) { return (0); }

    /* create 'camera' */
    player->camera = (struct Camera) {
        .position = alg_vec3(0.0, 0.0, 0.0),
        .rotation = alg_vec3(0.0, 0.0, -1.0),
        .direction = alg_vec3(0.0, 0.0, -90.0),
        .up = alg_vec3(0.0, 1.0, 0.0),
        .fieldOfView = 60.0,
    };

    /* success */
    return (1);
}


static int ftPlayerDelete(struct Entity *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'PlayerEntity' */
    struct
    PlayerEntity *player = (struct PlayerEntity *) self;
    if (!player) { return (0); }

    /* ... */ (void) player;

    /* success */
    return (1);
}


static int ftPlayerUpdate(struct Entity *self) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'PlayerEntity' */
    struct
    PlayerEntity *player = (struct PlayerEntity *) self;
    if (!player) { return (0); }
        
    /* enable 'locked' mode */
    if (Mouse->input.current[1]  == 1 &&
        Mouse->input.previous[1] == 0
    ) {
        Mouse->locked = 1;
        SDL_SetRelativeMouseMode(Mouse->locked);
    }

    /* disable 'locked' mode */
    if (Keyboard->current[SDL_SCANCODE_ESCAPE]  == 1 &&
        Keyboard->previous[SDL_SCANCODE_ESCAPE] == 0
    ) {
        Mouse->locked = 0;
        SDL_SetRelativeMouseMode(Mouse->locked);
    }

    /* update 'camera' */
    if (Mouse->locked) {
        ftCameraMove(&player->camera, 8.0 * Time->delta);
        ftCameraRotate(&player->camera, 0.1);
    }

    /* update 'player's 'position' */
    vec3 position = player->position = player->camera.position;

    /* 'player' position to 'block' position */
    vec3 block = FT_VOX_FLOAT_TO_BLOCK(position.x,
                                       position.y,
                                       position.z);

    /* re-center the 'world' center to 'block' if needed */
    if (alg_vec3ne(block, player->block)) {
        /* update 'player's 'block' position */
        player->block = block;

        /* get 'world' reference */
        struct
        World *world = self->parent->parent;
        if (world) {
            ftWorldSetCenter(world, block);
        }
    }

    /* success */
    return (1);
}


static int ftPlayerRender(struct Entity *self, struct VertexArray *pVao, struct Shader *pShader) {
    /* null-check */
    if (!self) { return (0); }

    /* get 'PlayerEntity' */
    struct
    PlayerEntity *player = (struct PlayerEntity *) self;
    if (!player) { return (0); }
   
    /* create 'u_proj' and 'u_view' projection matrices */
    mat4 u_proj = alg_mat4(1.0),
         u_view = alg_mat4(1.0);
    ftCameraMatrix(&player->camera, &u_proj, &u_view);
    ftShaderUniformSet(pShader, "u_proj", FT_UNIFORM_MATRIX4X4, &u_proj);
    ftShaderUniformSet(pShader, "u_view", FT_UNIFORM_MATRIX4X4, &u_view);

    /* 'pVao' is unused */
    (void) pVao;

    /* success */
    return (1);
}


int main(void) {

    /* create OpenGL window */
    
    if (!ftWindowCreate("42warsaw - ft_vox", 800, 600)) { return (1); }
   
    /* create input objects */
    
    ftKeyboardCreate();
    ftMouseCreate();
    ftTimeCreate();

    /* create 'shader' */

    struct
    Shader *shader = 0;
    ftShaderCreate(&shader);

    /* load 'vertex.glsl' */
    FILE *fp_vertex_glsl = fopen("src/glsl/vertex.glsl", "rb");
    if (fp_vertex_glsl) {
        ftShaderLoadGLSL(shader, fp_vertex_glsl, FT_SHADER_VERTEX);
        fclose(fp_vertex_glsl);
    }

    /* load 'fragment.glsl' */
    FILE *fp_fragment_glsl = fopen("src/glsl/fragment.glsl", "rb");
    if (fp_fragment_glsl) {
        ftShaderLoadGLSL(shader, fp_fragment_glsl, FT_SHADER_FRAGMENT);
        fclose(fp_fragment_glsl);
    }

    /* create vertex array */

    struct
    VertexArray *vao = 0;
    ftVertexArrayCreate(&vao);
    ftVertexArrayLayoutInt(vao, 0, 1, 0);

    /* create 'world' */

    srand(time(0));
    
    struct
    World world = { 0 };
    ftWorldCreate(&world, rand());
    ftWorldGenerate(&world);

    /* create 'player' entity */
    struct
    PlayerEntity *player = calloc(1, sizeof(struct PlayerEntity));
    player->create = ftPlayerCreate;
    player->delete = ftPlayerDelete;
    player->update = ftPlayerUpdate;
    player->render = ftPlayerRender;
    ftEntitySystemInsert(world.ecs, (struct Entity *) player);

    /* update-render loop */

    do {

        /* update */

        /* set new 'window' title */
        ftWindowSetTitle("42warsaw - ft_vox (FPS.: %d)", (int) (1.0 / Time->delta));

        /* update 'world' */
        ftWorldUpdate(&world);

        /* render */

        /* clear */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        /* render current vertex array */
        glUseProgram(shader->program);
        glBindVertexArray(vao->ID);

        /* render 'world' */
        ftWorldRender(&world, vao, shader);

        glBindVertexArray(0);
        glUseProgram(0);

        /* poll-events */
        ftWindowPollEvents();

    } while (!Window->flags.close);

    /* unload 'world' */
    ftWorldDelete(&world);

    /* unload OpenGL */

    ftShaderDelete(shader);
    ftVertexArrayDelete(vao);
    
    /* unload input objects */
    
    ftTimeDelete();
    ftMouseDelete();
    ftKeyboardDelete();

    /* quit */

    ftWindowDelete();

    return (0);
}
