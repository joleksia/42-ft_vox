#include "ft_vox.h"

/* 'Camera' : perspective 3D projection structure.
 * */

extern int ftCameraMatrix(Camera *pCamera, mat4 *pProj, mat4 *pView, vec2 size) {
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
