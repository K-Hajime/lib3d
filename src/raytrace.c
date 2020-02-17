#include "lib3d.h"

/**
 * レイとポリゴンの交点判定（ワールド座標）
 * ray_origin: Rayの始点
 * ray_direction: Rayの方向ベクトル（単位ベクトル）
 * poligon: 判定対象のポリゴン
 * (result) r: return trueの場合、交点。return falseの場合何もしない
 * (result) uv: return trueの場合、ポリゴン内のuv座標< b - a, c - a >。return falseの場合何もしない
 * return: 交点がある場合true
 */
bool l3IntersectRayPoligon(l3Mat41 ray_origin, l3Mat41 ray_direction, l3Poligon *poligon, l3Mat41 r, l3Mat21 uv) {
    l3Mat41A a_v0;  // ゼロ初期化不要
    l3Mat31 cw = poligon->vertices[0]->coordinateWorld;
    l3SubMat3(ray_origin, cw, a_v0);

    // クラメールの公式利用
    l3Mat31A cross_prod_r_d;
    l3Mat31 normal = poligon->normal, e2 = poligon->e2, e1 = poligon->e1;
    l3CrossProductVec3(a_v0, ray_direction, cross_prod_r_d);
    l3Type e = l3InnerProductVec3(ray_direction, normal);
    l3Type t = -l3InnerProductVec3(a_v0, normal) / e;
    l3Type u = l3InnerProductVec3(e2, cross_prod_r_d) / e;
    l3Mat31A minus_cross_prod_r_d;
    l3ScalarMulMat3(cross_prod_r_d, -1, minus_cross_prod_r_d);
    l3Type v = l3InnerProductVec3(e1, minus_cross_prod_r_d) / e;

    if (0 <= t && 0 <= u && u <= 1 && 0 <= v && v <= 1 && u + v <= 1) {
        r[0] = ray_origin[0] + t * ray_direction[0];
        r[1] = ray_origin[1] + t * ray_direction[1];
        r[2] = ray_origin[2] + t * ray_direction[2];
        uv[0] = u;
        uv[1] = v;
        return true;
    } else {
        return false;
    }
}

/**
 * ポリゴンの法線ベクトル（ワールド座標）
 * poligon: 対象のポリゴン
 * (result) normal: 法線ベクトル（ワールド座標）
 */
// void l3GetPoligonNormal(l3Poligon *poligon, l3Mat41 normal) {
//     l3Mat41A e1 = {0};
//     l3Mat41A e2 = {0};
//     l3Mat41A prod = {0};
//     l3SubMat(poligon->vertices[2]->coordinateWorld, poligon->vertices[0]->coordinateWorld, e1, 3);
//     l3SubMat(poligon->vertices[1]->coordinateWorld, poligon->vertices[0]->coordinateWorld, e2, 3);
//     l3CrossProductVec3(e2, e1, prod);
//     l3NormarizeVec(prod, normal, 3);
//     // l3ScalarMulMat(normal, -1, normal, 3);
// }

/**
 * レイトレーシングでポリゴンの点(u,v)のテクスチャ座標
 * poligon: 対象のポリゴン
 * uv: ポリゴン内のuv座標
 * (result) r: テクスチャ内でのxy座標（0-1）
 */
void l3GetRayPoligon2DTextureCoordinate(l3Poligon *poligon, l3Mat21 uv, l3Mat21 r) {
    l3Mat21A e1;
    l3Mat21A e2;
    l3SubMat2(&poligon->textureVertices[4], &poligon->textureVertices[0], e1);
    l3SubMat2(&poligon->textureVertices[2], &poligon->textureVertices[0], e2);
    r[0] = poligon->textureVertices[0] + uv[0] * e1[0] + uv[1] * e2[0];
    r[1] = poligon->textureVertices[1] + uv[0] * e1[1] + uv[1] * e2[1];
}

/**
 * レイと球の交点判定（ワールド座標）
 * ray_origin: Rayの始点
 * ray_direction: Rayの方向ベクトル（単位ベクトル）
 * sphere_center: 判定対象の球の中心座標
 * sphere_radius: 判定対象の球の半径
 * (result) r: return trueの場合、交点。return falseの場合何もしない
 * return: 交点がある場合true
 */
bool l3IntersectRaySphere(l3Mat41 ray_origin, l3Mat41 ray_direction, l3Mat41 sphere_center, l3Type sphere_radius, l3Mat41 r) {
    volatile l3Mat41A s_pc;
    l3SubMat3(ray_origin, sphere_center, s_pc);
    l3Type b = l3InnerProductVec3(s_pc, ray_direction);
    l3Type ab = l3VecAbs3(s_pc);
    l3Type c = ab * ab - sphere_radius * sphere_radius;
    l3Type d = b * b - c;
    if (d > 0) {
        l3Type t = -b - sqrt(d);
        if (t > 0) {
            r[0] = ray_origin[0] + t * ray_direction[0];
            r[1] = ray_origin[1] + t * ray_direction[1];
            r[2] = ray_origin[2] + t * ray_direction[2];
            return true;
        }
    } else {
        return false;
    }
}

/**
 * レイトレーシングで球上の点のテクスチャ座標
 * sphere_local: 球のローカル座標空間での点の位置
 * (result) r: テクスチャ内でのr座標（0-1）
 */
void l3GetRaySphere2DTextureCoordinate(l3Mat41 sphere_local, l3Mat21 r) {
    l3Type theta = acos(sphere_local[0] / sqrt(pow2(sphere_local[0]) + pow2(sphere_local[2])));
    l3Type phi = acos(sphere_local[1] / sqrt(pow2(sphere_local[0]) + pow2(sphere_local[1])));
    if (sphere_local[2] < 0) {
        theta = 2 * PI - theta;
    }
    r[0] = theta / (2 * PI);
    r[1] = phi / PI;
}

/**
 * レイと平面の交点判定（ワールド座標）
 * ray_origin: Rayの始点
 * ray_direction: Rayの方向ベクトル（単位ベクトル）
 * plane_normal: 平面の法線ベクトル
 * (result) r: return trueの場合、交点。return falseの場合何もしない
 * return: 交点がある場合true
 */
bool l3IntersectRayPlane(l3Mat41 ray_origin, l3Mat41 ray_direction, l3Mat41 plane_normal, l3Mat41 point, l3Mat41 r) {
    l3Mat31A tmp;
    l3SubMat3(point, ray_origin, tmp);
    l3Type t = l3InnerProductVec3(tmp, plane_normal) / l3InnerProductVec3(ray_direction, plane_normal);
    if (t > 0) {
        r[0] = ray_origin[0] + t * ray_direction[0];
        r[1] = ray_origin[1] + t * ray_direction[1];
        r[2] = ray_origin[2] + t * ray_direction[2];
        return true;
    } else {
        return false;
    }
}

bool l3IntersectRaySky(l3Mat41 ray_origin, l3Mat41 ray_direction, l3Mat41 sphere_center, l3Type sphere_radius, l3Mat41 r) {
    l3Mat41A s_pc;
    l3SubMat3(ray_origin, sphere_center, s_pc);
    l3Type b = l3InnerProductVec3(s_pc, ray_direction);
    l3Type ab = l3VecAbs3(s_pc);
    l3Type c = ab * ab - sphere_radius * sphere_radius;
    l3Type d = b * b - c;
    if (d > 0) {
        l3Type t = -b + sqrt(d);
        if (t > 0) {
            r[0] = ray_origin[0] + t * ray_direction[0];
            r[1] = ray_origin[1] + t * ray_direction[1];
            r[2] = ray_origin[2] + t * ray_direction[2];
            return true;
        }
    } else {
        return false;
    }
}

/**
 * 球の法線ベクトル
 * sphere_local: 球のローカル座標空間での点の位置
 * (result) normal: 法線ベクトル
 */
// void l3GetSphereNormal(l3Mat41 sphere_local, l3Mat41 r) {
//     l3NormarizeVec(sphere_local, r, 3);
// }

/**
 * 反射ベクトル
 * incident_vec: 入射ベクトル
 * normal: 法線ベクトル
 * (result) r: 反射ベクトル
 */
void l3GetReflectedVec(l3Mat41 incident_vec, l3Mat41 normal, l3Mat41 r) {
    l3Mat41A tmp;
    l3Type ip = l3InnerProductVec(normal, incident_vec, 3);
    l3ScalarMulMat(normal, -2 * ip, tmp, 3);
    l3AddMat(incident_vec, tmp, r, 3);
}

// rgbもmat31で表すのがいいかも
void l3MultiplyColor(l3RGB *a, l3RGB *b, l3RGB *r) {
    r->r = a->r * b->r / 255;
    r->g = a->g * b->g / 255;
    r->b = a->b * b->b / 255;
}

/**
 * ワールド座標空間からカメラ座標空間への基底変換行列
 */
void l3MakeWorldToCameraBasisChangeMat33(l3CameraInfo *camerainfo, l3Mat33 r) {
    l3Mat41A tmp = {0};
    l3Mat41A cx = {0};
    l3Mat41A cy = {0};
    l3Mat41A cz = {0};

    // cz
    l3SubMat(camerainfo->target, camerainfo->coordinate, tmp, 3);
    l3NormarizeVec(tmp, cz, 3);
    // cx
    l3CrossProductVec3(cz, camerainfo->upper, tmp);
    l3NormarizeVec(tmp, cx, 3);
    // cy
    l3CrossProductVec3(cx, cz, tmp);
    l3NormarizeVec(tmp, cy, 3);

    memcpy(&r[0], cx, sizeof(l3Type) * 3);
    memcpy(&r[3], cy, sizeof(l3Type) * 3);
    memcpy(&r[6], cz, sizeof(l3Type) * 3);
}

/**
 * Rayの始点
 * Ray始点での方向ベクトル
 */
void l3GetRayStartPointAndDirection(l3Mat33 p_world_to_camera, l3Mat31 camera_pos, l3Type near,
                                    l3Type w, l3Type h,
                                    l3Type u, l3Type v, l3Mat31 start_point, l3Mat31 direction) {
    l3Mat31A ray_pos_c = {(2 * u - w) / h,
                          1 - 2 * v / h,
                          near};
    l3Mat31A ray_pos_w = {0};
    l3MulMat(p_world_to_camera, ray_pos_c, ray_pos_w, 3, 3, 1);
    l3NormarizeVec(ray_pos_w, direction, 3);

    // l3AddMat(ray_pos_w, camera_pos, start_point, 3);
    l3AddMat(ray_pos_w, camera_pos, start_point, 3);
}

/**
 * Rayの着地点を見つける
 */
bool l3FindRayCrossPoint(l3Ray *ray, l3Environment *env) {
    l3Mat41A min_intersection = {0};
    l3Mat21A min_uv = {0};
    l3Poligon *min_poligon = NULL;
    l3Type min_length = 0;
    l3Mat41 rayStartPoint = ray->rayStartPoint,
            rayDirection = ray->rayDirection;
    l3Poligon **poligons = (l3Poligon **)env->poligons.data;
    bool found = false;

    for (size_t i = 0, l = env->poligons.length; i < l; i++) {
        l3Poligon *p = *(poligons + i);  //array_at(&env->poligons, i);
        switch (p->poligonType) {
            case l3PoligonTypeTriangle: {
                l3Mat41A intersection;
                l3Mat21A uv;
                if (!l3IntersectRayPoligon(rayStartPoint, rayDirection, p, intersection, uv)) {
                    break;
                }
                l3Type distance = l3DistanceVec3(rayStartPoint, intersection);
                if (!found || min_length > distance) {
                    memcpy(min_intersection, intersection, sizeof(l3Type) * 4);
                    memcpy(min_uv, uv, sizeof(l3Type) * 2);
                    min_length = distance;
                    min_poligon = p;
                    found = true;
                }
            } break;
            case l3PoligonTypeShpere: {
                l3Mat41A intersection = {0};
                if (!l3IntersectRaySphere(rayStartPoint, rayDirection,
                                          p->vertices[0]->coordinateWorld,
                                          p->sphere_radius, intersection)) {
                    break;
                }
                l3Type distance = l3DistanceVec3(rayStartPoint, intersection);
                if (!found || min_length > distance) {
                    memcpy(min_intersection, intersection, sizeof(l3Type) * 4);
                    min_length = distance;
                    min_poligon = p;
                    found = true;
                }
            } break;
            case l3PoligonTypePlane: {
                l3Mat41A intersection = {0};
                if (!l3IntersectRayPlane(rayStartPoint, rayDirection,
                                         p->normal, p->vertices[0]->coordinateWorld,
                                         intersection)) {
                    break;
                }
                l3Type distance = l3DistanceVec3(rayStartPoint, intersection);
                if (!found || min_length > distance) {
                    memcpy(min_intersection, intersection, sizeof(l3Type) * 4);
                    min_length = distance;
                    min_poligon = p;
                    found = true;
                }
            }
            case l3PoligonTypeSky: {
                l3Mat41A intersection = {0};
                if (!l3IntersectRaySky(rayStartPoint, rayDirection,
                                       env->camera.coordinate,
                                       env->camera.far, intersection)) {
                    break;
                }
                l3Type distance = l3DistanceVec3(rayStartPoint, intersection);
                if (!found || min_length > distance) {
                    memcpy(min_intersection, intersection, sizeof(l3Type) * 4);
                    min_length = distance;
                    min_poligon = p;
                    found = true;
                }
            } break;
        }
    }

    if (found) {
        memcpy(ray->uv, min_uv, sizeof(l3Type) * 2);
        memcpy(ray->intersection, min_intersection, sizeof(l3Type) * 3);
        ray->poligon = min_poligon;
    }
    return found;
}

void l3GetNormal(l3Poligon *poligon, l3Mat41 intersection, l3Mat41 normal, l3Environment *env) {
    switch (poligon->poligonType) {
        case l3PoligonTypeTriangle:
            l3NormarizeVec3(poligon->normal, normal);
            break;
        case l3PoligonTypeShpere: {
            l3Mat41A sphere_local;
            l3SubMat3(intersection, poligon->vertices[0]->coordinateWorld, sphere_local);
            l3NormarizeVec3(sphere_local, normal);
        } break;
        case l3PoligonTypePlane: {
            l3CopyMat(poligon->normal, normal, 3);
        } break;
        case l3PoligonTypeSky: {
            l3Mat41A sphere_local;
            l3SubMat3(env->camera.coordinate, intersection, sphere_local);
            l3NormarizeVec3(sphere_local, normal);
        } break;
    }
}

void l3GetLightDirection(l3Mat31 intersection, l3Poligon *light_poligon, l3Mat31 r) {
    switch (light_poligon->lightType) {
        case l3LightTypeParallel:
            l3ScalarMulMat3(light_poligon->normal, -1, r);
            break;
        case l3LightTypePoint:
            // 重心を求めずにやってる（適当に0番目の頂点）
            l3SubMat3(light_poligon->vertices[0]->coordinateWorld, intersection, r);
            l3NormarizeVec3(r, r);
            break;
    }
}

/**
 * 始点と方向が決まっているRayから、
 * 交点を見つけ、
 * その地点の色を求める
 * 交点がなかった場合にはそこで再帰終了、MAXDEPTHを超えた場合にも TODO: 角度によって
 * Depthの定義：たどる交点の数
 */
bool l3TraceRay(l3Ray *ray, l3Environment *env, int depth) {
    if (l3FindRayCrossPoint(ray, env)) {
        l3RGB sumcolor = {0};
        if (ray->poligon->poligonType == l3PoligonTypeSky) {
            ray->color.r = ray->poligon->color.r;
            ray->color.g = ray->poligon->color.g;
            ray->color.b = ray->poligon->color.b;
            return true;
        }
        l3Mat41A normal = {0};
        l3GetNormal(ray->poligon, ray->intersection, normal, env);
        // l3Mat31A inv_ray_direction;
        // l3ScalarMulMat3(ray->rayDirection, -1, inv_ray_direction);
        // l3Type theta = l3InnerProductVec(normal, inv_ray_direction, 2);
        l3Type k_s = 0.3,  //l3ReflectionRate(theta / (2 * PI), 0.2),
            k_d = 1 - k_s;

        for (int i = 0, l = env->poligons.length; i < l; ++i) {
            l3Poligon *light_poligon = array_at(&env->poligons, i);
            if (light_poligon->lightType) {
                // 拡散反射 すべての光源に対して
                // 分岐点光源並行光源
                l3Mat31A light;
                l3Ray kage_ray = {0};
                l3GetLightDirection(ray->intersection, light_poligon, light);
                l3NormarizeVec3(light, light);
                l3CopyMat(light, kage_ray.rayDirection, 3);
                l3Mat31A tmp;
                l3DivMat(light, 10, tmp, 3);
                l3AddMat3(ray->intersection, tmp, kage_ray.rayStartPoint);
                // 影は光源を無視
                bool cross = l3FindRayCrossPoint(&kage_ray, env);
                if (!cross ||
                     kage_ray.poligon->poligonType == l3PoligonTypeSky || kage_ray.poligon->lightType) {
                    // 影にならないとき
                    l3Type ipv3 = l3InnerProductVec3(normal, light);
                    l3Type l_d = max(min(1, ipv3), 0);
                    l3RGB material_color = ray->poligon->color;
                    if (light_poligon->lightType == l3LightTypePoint) {
                        l3Type distance = 10;  //l3DistanceVec3(light_poligon->vertices[0]->coordinateWorld, ray->intersection);
                        material_color.r *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.r / 255.0 / (distance * 0.1);
                        material_color.g *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.g / 255.0 / (distance * 0.1);
                        material_color.b *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.b / 255.0 / (distance * 0.1);
                    } else {
                        material_color.r *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.r / 255.0;
                        material_color.g *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.g / 255.0;
                        material_color.b *= l_d * k_d * light_poligon->lightIntensity * light_poligon->color.b / 255.0;
                    }
                    l3AddColor(sumcolor, material_color);
                }
            }
        }

        if (depth <= l3RAY_TRACE_MAX_DEPTH) {
            // 鏡面反射Ray
            // if (ray->poligon->poligonType != l3PoligonTypeSky) {

            // l3Ray specular = {0};
            // l3GetReflectedVec(ray->rayDirection, normal, specular.rayDirection);
            // l3AddMat3(ray->intersection, specular.rayDirection, specular.rayStartPoint);
            // if (l3TraceRay(&specular, env, depth + 1) &&
            //     !(ray->poligon->poligonType == l3PoligonTypePlane && specular.poligon->poligonType == l3PoligonTypeSky)) {
            //     l3ScalarMulColor(specular.color, 1.0 * 1.0 * k_s);
            //     l3AddColor(sumcolor, specular.color);
            // }

            // }
            // l3Ray specular = {0};
            // l3Mat41A dir = {0};
            // l3GetReflectedVec(ray->rayDirection, normal, dir);
            // // ランダムに微小角回転させる
            // l3Mat44A rotate = {0};
            // l3MakeRoundMat44(radians(rand() / (l3Type)RAND_MAX * 50),
            //                  radians(rand() / (l3Type)RAND_MAX * 50),
            //                  radians(rand() / (l3Type)RAND_MAX * 50), rotate);
            // l3MulMat4441(rotate, dir, specular.rayDirection);
            // l3NormarizeVec(specular.rayDirection, specular.rayDirection, 3);
            // l3AddMat(ray->intersection, specular.rayDirection, specular.rayStartPoint, 3);
            // if (l3TraceRay(&specular, env, depth + 1)) {
            //     l3MulColor(sumcolor, specular.color);
            // }
            // // 鏡面反射Ray
        }
        ray->color.r = sumcolor.r;
        ray->color.g = sumcolor.g;
        ray->color.b = sumcolor.b;
        return true;
    } else {
        //     ray->color.r = ray->poligon->color.r;
        //     ray->color.g = ray->poligon->color.g;
        //     ray->color.b = ray->poligon->color.b;
        return false;
    }
}

void l3SetWorldCoordinate(l3Environment *env) {
    for (int i = 0; i < env->objects.length; ++i) {
        l3Object *_object = array_at(&env->objects, i);
        l3Type lw[16] = {0};
        l3MakeLocalToWorldMat44(_object->dx, _object->dy, _object->dz,
                                _object->sx, _object->sy, _object->sz,
                                _object->theta_x, _object->theta_y, _object->theta_z, lw);
        for (int i = 0; i < _object->poligon_count; i++) {
            l3Poligon *_poligon = _object->poligons[i];
            switch (_poligon->poligonType) {
                case l3PoligonTypeTriangle: {
                    for (int j = 0; j < l3POLIGON_VERTEX_COUNT; j++) {
                        l3Vertex *_vertex = _poligon->vertices[j];
                        if (!_vertex->converted) {
                            // ローカル->ワールド
                            l3InitMat(_vertex->coordinateWorld, 4, 1);
                            l3MulMat4441(lw, _vertex->coordinate, _vertex->coordinateWorld);
                            _vertex->converted = true;
                        }
                    }
                    // 前処理
                    l3SubMat(_poligon->vertices[1]->coordinateWorld, _poligon->vertices[0]->coordinateWorld, _poligon->e1, 3);
                    l3SubMat(_poligon->vertices[2]->coordinateWorld, _poligon->vertices[0]->coordinateWorld, _poligon->e2, 3);
                    l3CrossProductVec3(_poligon->e2, _poligon->e1, _poligon->normal);
                } break;
                case l3PoligonTypeShpere: {
                    for (int j = 0; j < 2; j++) {
                        l3Vertex *_vertex = _poligon->vertices[j];
                        if (!_vertex->converted) {
                            // ローカル->ワールド
                            l3InitMat(_vertex->coordinateWorld, 4, 1);
                            l3MulMat4441(lw, _vertex->coordinate, _vertex->coordinateWorld);
                            _vertex->converted = true;
                        }
                    }
                } break;
                case l3PoligonTypePlane: {
                    l3Vertex *_vertex = _poligon->vertices[0];
                    if (!_vertex->converted) {
                        // ローカル->ワールド
                        l3InitMat(_vertex->coordinateWorld, 4, 1);
                        l3MulMat4441(lw, _vertex->coordinate, _vertex->coordinateWorld);
                        _vertex->converted = true;
                    }
                } break;
                case l3PoligonTypeSky: {
                } break;
            }
        }
    }
}