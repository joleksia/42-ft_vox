#include "ft_vox.h"

/* 'Camera2D' : orthographic 2D projection structure.
 * */

extern int ftCamera2DMatrix(Camera2D *pCamera, mat4 *pProj, mat4 *pView, vec2 size) {
    /* null-check */
    if (!pCamera) { return (0); }

    /* get 'proj' matrix */
    mat4 proj = alg_mat4ortho(0.0, size.x,
                              0.0, size.y,
                              -1.0, 1.0);

    /* get 'view' matrix */
    mat4 m_target = alg_mat4translate(alg_vec3(pCamera->target.x, pCamera->target.y, 0.0));
    mat4 m_offset = alg_mat4translate(alg_vec3(pCamera->offset.x, pCamera->offset.y, 0.0));
    mat4 m_rotate = alg_mat4rotate(alg_vec3(0.0, 0.0, 1.0), alg_deg2rad(pCamera->rotation));
    mat4 m_scale  = alg_mat4scale(alg_vec3(pCamera->scale, pCamera->scale, 1.0));
    mat4 view = alg_mat4mul(alg_mat4mul(alg_mat4mul(m_scale, m_rotate), m_target), m_offset);

    /* return */
    if (pProj) { *pProj = proj; }
    if (pView) { *pView = view; }

    /* success */
    return (1);
}


/* 'Camera3D' : perspective 3D projection structure.
 * */

extern int ftCamera3DMatrix(Camera3D *pCamera, mat4 *pProj, mat4 *pView, vec2 size) {
    /* null-check */
    if (!pCamera) { return (0); }

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
