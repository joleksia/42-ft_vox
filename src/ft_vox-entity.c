#include "ft_vox.h"

extern int ft_ecs_create(struct ecs_s **self, struct world_s *parent) {
    /* null-check */
    if (!self) { return (0); }

    /* alloc. 'ecs' */
    struct ecs_s *ecs = (struct ecs_s *) calloc(1, sizeof(struct ecs_s));
    if (!ecs) { return (0); }

    /* reference 'parent' */ 
    ecs->parent = parent;

    /* create 'entities' vector */
    size_t count = 0;
    size_t capacity = 8;
    struct
    entity_s **data = (struct entity_s **) calloc(capacity, sizeof(struct entity_s *));
    if (!data) { return (0); }

    /* return 'entities' */
    ecs->entities.data = data;
    ecs->entities.count = count;
    ecs->entities.capacity = capacity;

    /* return 'self' */
    *self = ecs;

    /* success */
    return (1);
    
}


extern int ft_ecs_delete(struct ecs_s *self) {
    /* null-check */
    if (!self) { return (0); }

    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        entity_s *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* release 'entity' */
        if (entity->delete) {
            entity->delete(entity);
        }
        free(entity);
    }

    /* release 'entities' */
    free(self->entities.data);

    /* release 'self' */
    free(self);

    /* success */
    return (1);
}


extern int ft_ecs_update(struct ecs_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        entity_s *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* run 'update' */
        if (entity->update) {
            entity->update(entity);
        }
    }

    /* success */
    return (1);
}


extern int ft_ecs_render(struct ecs_s *self) {
    /* null-check */
    if (!self) { return (0); }
    
    /* iterate over every 'entity' */
    for (size_t i = 0; i < self->entities.count; i++) {
        struct
        entity_s *entity = self->entities.data[i];
        if (!entity) { continue; }

        /* run 'render' */
        if (entity->render) {
            entity->render(entity);
        }
    }

    /* success */
    return (1);
}


extern int ft_ecs_insert(struct ecs_s *self, struct entity_s *entity) {
    /* null-check */
    if (!self)    { return (0); }
    if (!entity) { return (0); }

    /* try to re-use any previous 'empty' spot */
    size_t count = self->entities.count;
    for (size_t i = 0; i < count; i++) {
        /* emplace if 'data[i]' is null */
        if (!self->entities.data[i]) {
            self->entities.data[i] = entity;

            /* set 'entity' members */
            entity->parent = self;
            entity->id = i;

            /* run 'create' */
            if (entity->create) {
                entity->create(entity);
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
        entity_s **data = (struct entity_s **) calloc(capacity, sizeof(struct entity_s *));
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

    /* set 'entity' to 'count' index */
    entity->parent = self;
    entity->id = self->entities.count;
    self->entities.data[self->entities.count++] = entity;
            
    /* run 'create' */
    if (entity->create) {
        entity->create(entity);
    }

    /* success */
    return (1);
}
