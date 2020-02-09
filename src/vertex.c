#include "lib3d.h"

void l3MakeRoundXMat44(l3Type theta, l3Mat44 r) {
    r[0] = 1;
    l3Type cos_theta = cos(theta),
           sin_theta = sin(theta);
    r[5] = cos_theta;
    r[6] = sin_theta;
    r[9] = -sin_theta;
    r[10] = cos_theta;
    r[15] = 1;
}
void l3MakeRoundYMat44(l3Type theta, l3Mat44 r) {
    l3Type cos_theta = cos(theta),
           sin_theta = sin(theta);
    r[0] = cos_theta;
    r[2] = -sin_theta;
    r[5] = 1;
    r[8] = sin_theta;
    r[10] = cos_theta;
    r[15] = 1;
}
void l3MakeRoundZMat44(l3Type theta, l3Mat44 r) {
    l3Type cos_theta = cos(theta),
           sin_theta = sin(theta);
    r[0] = cos_theta;
    r[1] = sin_theta;
    r[4] = -sin_theta;
    r[5] = cos_theta;
    r[10] = 1;
    r[15] = 1;
}
void l3MakeTransposeMat44(l3Type dx, l3Type dy, l3Type dz, l3Mat44 r) {
    r[0] = 1;
    r[5] = 1;
    r[10] = 1;
    r[12] = dx;
    r[13] = dy;
    r[14] = dz;
    r[15] = 1;
}
void l3MakeScaleMat44(l3Type sx, l3Type sy, l3Type sz, l3Mat44 r) {
    r[0] = sx;
    r[5] = sy;
    r[10] = sz;
    r[15] = 1;
}

void l3MakeLocalToWorldMat44(l3Type dx, l3Type dy, l3Type dz,
                             l3Type sx, l3Type sy, l3Type sz,
                             l3Type theta_x, l3Type theta_y, l3Type theta_z, l3Mat44 r) {
    l3Type s[16] = {0};
    l3Type d[16] = {0};
    l3Type rx[16] = {0};
    l3Type ry[16] = {0};
    l3Type rz[16] = {0};
    l3MakeTransposeMat44(dx, dy, dz, d);
    l3MakeScaleMat44(sx, sy, sz, s);
    l3MakeRoundXMat44(-theta_x, rx);
    l3MakeRoundYMat44(-theta_y, ry);
    l3MakeRoundZMat44(-theta_z, rz);

    l3Type* mat44s[] = {s, d, rz, ry, rx};
    l3MulMat44s44(5, mat44s, r);
}

/**
 * camera = target = upper = r
 */
void l3MakeWorldToCameraMat44(l3Mat41 camera, l3Mat41 target, l3Mat41 upper, l3Mat44 r) {
    l3Type tmp[4] = {0},
           cx[4] = {0},
           cy[4] = {0},
           cz[4] = {0};
    // cz
    l3SubMat(target, camera, tmp, 3);
    l3NormarizeVec(tmp, cz, 3);
    // cx
    l3CrossProductVec3(cz, upper, tmp);
    l3NormarizeVec(tmp, cx, 3);
    // cy
    l3CrossProductVec3(cx, cz, tmp);
    l3NormarizeVec(tmp, cy, 3);

    r[0] = cx[0];
    r[4] = cx[1];
    r[8] = cx[2];
    r[1] = cy[0];
    r[5] = cy[1];
    r[9] = cy[2];
    r[2] = cz[0];
    r[6] = cz[1];
    r[10] = cz[2];
    r[12] = -l3InnerProductVec(camera, cx, 3);
    r[13] = -l3InnerProductVec(camera, cy, 3);
    r[14] = -l3InnerProductVec(camera, cz, 3);
    r[15] = 1;
}

void l3MakeCameraToProjectionMat44(l3Type angle, l3Type aspect, l3Type near, l3Type far, l3Mat44 r) {
    l3Type tan_angle_2 = tan(angle / 2);
    r[5] = 1 / tan_angle_2;
    r[0] = r[5] / aspect;
    r[10] = far / (far - near);
    r[11] = 1;
    r[14] = -near * r[10];
}

void l3MakeProjectionToScreenMat44(l3Type width, l3Type height, l3Mat44 r) {
    l3Type width_2 = width / 2.0;
    l3Type height_2 = height / 2.0;
    r[0] = width_2;
    r[5] = -height_2;
    r[10] = 1;
    r[12] = width_2;
    r[13] = height_2;
    r[15] = 1;
}

void l3AppendPoligonsFromObject(l3Object* _object, l3Mat44 wcps, int w, int h, array* poligons) {
    l3Type lw[16] = {0};
    l3MakeLocalToWorldMat44(_object->dx, _object->dy, _object->dz,
                            _object->sx, _object->sy, _object->sz,
                            _object->theta_x, _object->theta_y, _object->theta_z, lw);
    l3Type lwcps[16] = {0};
    l3MulMat4444(wcps, lw, lwcps);

    l3Type r[4] = {0};
    for (int i = 0; i < _object->poligon_count; i++) {
        l3Poligon* _poligon = _object->poligons[i];
        for (int j = 0; j < l3POLIGON_VERTEX_COUNT; j++) {
            l3Vertex* _vertex = _poligon->vertices[j];
            if (!_vertex->converted) {
                l3InitMat(r, 4, 1);
                // ローカル->ワールド->プロジェクション->スクリーン座標変換
                l3MulMat4441(lwcps, _vertex->coordinate, r);
                l3DivMat(r, r[3], r, 4);

                // 結果格納
                memcpy(_vertex->coordinate2d, r, sizeof(_vertex->coordinate2d));
                _vertex->converted = true;
            }
        }
        // ポリゴンに対する諸設定はここで済ます
        l3SetMaxZofPoligon(_poligon);
        l3SetOuterRectPoligon(_poligon);
        array_push(poligons, _poligon);
    }
}

void l3ClearVertices(int count, l3Vertex* vs[]) {
    while (count) vs[--count]->converted = false;
}