#include "ft_vox.h"

extern int ftEntitySystemCreate(struct EntitySystem *self, struct World *pWorld) {
    /* null-check */
    if (!self)   { return (0); }
    if (!pWorld) { return (0); }

    /* reference 'pWorld' parent */ 
    self->parent = pWorld;

    /* create 'entities' vector */
    size_t count = 0;
    size_t capacity = 8;
    struct
    Entity **data = (struct Entity **) calloc(capacity, sizeof(struct Entity *));
    if (!data) { return (0); }

    /* return 'entities' */
    self->entities.data = data;
    self->entities.count = count;
    self->entities.capacity = capacity;

    /* success */
    return (1);
    
}


extern int ftEntitySystemDelete(struct EntitySystem *self) {
    /* null-check */
    if (!self) { return (0); }

    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        Entity *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* release 'entity' */
        if (entity->delete) {
            entity->delete(entity);
        }
        free(entity);
    }

    /* release 'entities' */
    free(self->entities.data);

    /* success */
    return (1);
}


extern int ftEntitySystemUpdate(struct EntitySystem *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        Entity *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* run 'update' */
        if (entity->update) {
            entity->update(entity);
        }
    }

    /* success */
    return (1);
}


extern int ftEntitySystemRender(struct EntitySystem *self, struct VertexArray *pVao, struct Shader *pShader) {
    /* null-check */
    if (!self)    { return (0); }
    if (!pVao)    { return (0); }
    if (!pShader) { return (0); }
    
    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        Entity *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* run 'render' */
        if (entity->render) {
            entity->render(entity, pVao, pShader);
        }
    }

    /* success */
    return (1);
}


extern int ftEntitySystemInsert(struct EntitySystem *self, struct Entity *pEntity) {
    /* null-check */
    if (!self)    { return (0); }
    if (!pEntity) { return (0); }

    /* try to re-use any previous 'empty' spot */
    size_t count = self->entities.count;
    for (size_t i = 0; i < count; i++) {
        /* emplace if 'data[i]' is null */
        if (!self->entities.data[i]) {
            self->entities.data[i] = pEntity;

            /* set 'pEntity' members */
            pEntity->parent = self;
            pEntity->ID = i;

            /* run 'create' */
            if (pEntity->create) {
                pEntity->create(pEntity);
            }

            /* success */
            return (1);
        }
    }

    /* size-check */
    if (self->entities.count >= self->entities.capacity) {
        size_t capacity = self->entities.capacity * 2;

        /* try realloc. 'data' */
        struct
        Entity **data = (struct Entity **) calloc(capacity, sizeof(struct Entity *));
        if (!data) { return (0); }

        /* copy old 'data' to new' data' */
        size_t count = self->entities.count;
        for (size_t i = 0; i < count; i++) {
            data[i] = self->entities.data[i];
        }

        /* re-assign 'entitites' */
        free(self->entities.data);
        self->entities.data = data;
        self->entities.count = count;
        self->entities.capacity = capacity;
    }

    /* set 'pEntity' to 'count' index */
    pEntity->parent = self;
    pEntity->ID = self->entities.count;
    self->entities.data[self->entities.count++] = pEntity;
            
    /* run 'create' */
    if (pEntity->create) {
        pEntity->create(pEntity);
    }

    /* success */
    return (1);
}
