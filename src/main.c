#include "array.h"
#include "lib3d.h"

void transition(l3Environment* env, int frame) {
    l3Object* obj = array_at(&env->objects, 0);
    obj->theta_y = frame * 5 * 3.14 / 180.0;
    obj->theta_x = frame * 5 * 3.14 / 180.0;
    l3Object* obj2 = array_at(&env->objects, 1);
    obj2->theta_z = frame * 5 * 3.14 / 180.0;
    // obj2->theta_x = frame * 5 * 3.14 / 180.0;
    // obj1->theta_y = frame * 5 * 3.14 / 180.0;

    // l3Object* obj2 = array_at(&env->objects, 2);
    // obj2->theta_z = frame * 5 * 3.14 / 180.0;
    // obj2->dx = frame;
}

int main(int argc, const char* argv[]) {
    // default options
    l3Options options;
    options.outdir = "bin";
    options.h = 1080;
    options.w = 1920;
    options.frames = 100;
    options.threads = 4;
    options.renderer = l3RaytracingRenderer;
    if (!l3ParseArgs(argc, argv, &options)) exit(0);

    l3Environment env;
    {
        l3RGB red = {255, 0, 0};
        l3RGB green = {0, 255, 0};
        l3RGB blue = {0, 0, 255};
        l3RGB white = {255, 255, 255};
        l3InitializeEnvironment(&env);
        env.w = options.w;
        env.h = options.h;
        env.outdir = options.outdir;

        // オブジェクト構築
        l3Object* _object = l3CreateObject();
        int vs[] = {
            l3AddVertexToObject(_object, l3CreateVertex(5, 5, 5, &red)),
            l3AddVertexToObject(_object, l3CreateVertex(-5, -5, 5, &green)),
            l3AddVertexToObject(_object, l3CreateVertex(5, -5, -5, &blue)),
            l3AddVertexToObject(_object, l3CreateVertex(-5, 5, -5, &white)),
        };
        // 右回りが表、数字はオブジェクトごとの頂点のインデックス
        l3Poligon* poligons[] = {
            l3CreatePoligon(0, 2, 1),
            l3CreatePoligon(0, 3, 2),
            l3CreatePoligon(1, 2, 3),
            l3CreatePoligon(0, 1, 3),
        };

        poligons[0]->material = l3PoligonMaterialColor;
        poligons[0]->color.r = 255;
        poligons[1]->material = l3PoligonMaterialColor;
        poligons[1]->color.g = 255;
        poligons[2]->material = l3PoligonMaterialColor;
        poligons[2]->color.b = 255;
        poligons[3]->material = l3PoligonMaterialColor;
        poligons[3]->color.r = 255;
        poligons[3]->color.b = 255;

        // l3Object* sphere = l3CreateObject();
        // int vs[] = {
        //     l3AddVertexToObject(sphere, l3CreateVertex(5, 5, 5, &red)),
        // };
        // l3Poligon* poligons[] = {
        //     l3CreatePoligon(0),
        // }

        // テクスチャ読み込み・貼り付け
        // l3Texture texture;
        // l3Load2DTexture("assets/tex4.ppm", &texture);
        // l3Mat32A texture_vertices = {0.5, 0.5, 0, 1, 1, 1};
        l3SetPoligonsToObject(_object, sizeof(poligons) / sizeof(l3Poligon*), poligons);
        l3SetTransposeObject(_object, 15, 15, 15);
        l3AddObjectToEnvironment(&env, _object);

        l3Object* obj2 = l3CloneObject(_object);
        l3SetTransposeObject(obj2, 30, 15, 20);
        l3AddObjectToEnvironment(&env, obj2);
        l3SetCameraInfoToEnvironment(&env, 14, 0, -10,
                                     20, 20, 20,
                                     0, 1, 0,
                                     radians(50), 2, 100);
    }

    l3MultithreadRenderer(&env, options.renderer, transition, options.frames, options.threads);
    // l3MultithreadRenderer(&env, l3RasterizingRenderer, transition, 100, 16);

    l3DestructEnvironment(&env);
}
