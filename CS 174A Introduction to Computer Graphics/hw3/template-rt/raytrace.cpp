/* 
Chang Liu
CS 174A Project 3
raytrace.cpp
–––––––––––––––––––––––––––––––––––––––––––––––––– */

#define _CRT_SECURE_NO_WARNINGS
#include "matm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int g_width;
int g_height;

struct Ray
{
    vec4 origin;
    vec4 dir;
};

#define MAX_DEPTH 3

struct Sphere {

    string name; 

    vec4 pos; 
    vec4 scale;
    vec4 color;

    float ka, kd, ks, kr;
    float n;                    // shininess factor

};

struct Light {
    string name;

    vec4 pos; 

    vec4 color;
};

// global variables
vector<Sphere> g_spheres;
vector<Light> g_lights;
vec4 g_ambient; 
vec4 g_bg; 

vector<vec4> g_colors;

float g_left;
float g_right;
float g_top;
float g_bottom;
float g_near;

string g_output;


// -------------------------------------------------------------------
// Utility Functions
inline vec3 toVec3( vec4 in ) { return vec3( in[0], in[1], in[2] ); }

inline mat4 getSphereMatrix( Sphere s ) { 
    mat4 result = mat4();
    result *= Translate(s.pos.x, s.pos.y, s.pos.z);
    result *= Scale(s.scale.x, s.scale.y, s.scale.z);
    return result; 
}


// -------------------------------------------------------------------
// Input file parsing
vec4 toVec4(const string& s1, const string& s2, const string& s3)
{
    stringstream ss(s1 + " " + s2 + " " + s3);
    vec4 result;
    ss >> result.x >> result.y >> result.z;
    result.w = 1.0f;
    return result;
}

float toFloat(const string& s)
{
    stringstream ss(s);
    float f;
    ss >> f;
    return f;
}

void parseLine(const vector<string>& vs)
{
    const int num_labels = 11; 
    const string labels[] = {       
        "NEAR",                 // 0
        "LEFT",                 // 1
        "RIGHT",                // 2
        "BOTTOM",               // 3
        "TOP",                  // 4
        "RES",                  // 5
        "SPHERE",               // 6
        "LIGHT",                // 7
        "BACK",                 // 8
        "AMBIENT",              // 9
        "OUTPUT"                // 10
    };

    unsigned label_id = find( labels, labels + num_labels, vs[0] ) - labels;
    switch(label_id) {
        case 0: g_near = toFloat( vs[1] ); break;     // NEAR
        case 1: g_left = toFloat( vs[1] ); break;     // LEFT
        case 2: g_right = toFloat( vs[1] ); break;    // RIGHT
        case 3: g_bottom = toFloat( vs[1] ); break;   // BOTTOM
        case 4: g_top = toFloat( vs[1] ); break;      // TOP
        case 5:                                       // RES
            g_width = (int)toFloat(vs[1]);
            g_height = (int)toFloat(vs[2]);
            g_colors.resize(g_width * g_height);
            break; 
        case 6: 
        {
            Sphere s;
            s.name = vs[1];
            s.pos = toVec4( vs[2], vs[3], vs[4] );
            s.scale = toVec4( vs[5], vs[6], vs[7] );
            s.color = toVec4( vs[8], vs[9], vs[10] );
            s.ka = toFloat( vs[11] );
            s.kd = toFloat( vs[12] );
            s.ks = toFloat( vs[13] );
            s.kr = toFloat( vs[14] );
            s.n = toFloat( vs[15] );

            g_spheres.push_back(s);
            break;
        }
        case 7:
        {
            Light l;
            l.name = vs[1];
            l.pos = toVec4( vs[2], vs[3], vs[4] ); 
            l.color = toVec4( vs[5], vs[6], vs[7] );
            g_lights.push_back(l); 
            break;
        }
        case 8: g_bg = toVec4( vs[1], vs[2], vs[3] ); break;
        case 9: g_ambient = toVec4( vs[1], vs[2], vs[3] ); break;
        case 10: g_output = vs[1]; break;
    }


    
}

void loadFile(const char* filename)
{
    ifstream is(filename);
    if (is.fail())
    {
        cout << "Could not open file " << filename << endl;
        exit(1);
    }
    string s;
    vector<string> vs;
    while(!is.eof())
    {
        vs.clear();
        getline(is, s);
        istringstream iss(s);
        while (!iss.eof())
        {
            string sub;
            iss >> sub;
            vs.push_back(sub);
        }
        parseLine(vs);
    }
}


// -------------------------------------------------------------------
// Utilities
void setColor(int ix, int iy, const vec4& color)
{
    int iy2 = g_height - iy - 1; // Invert iy coordinate.
    g_colors[iy2 * g_width + ix] = color;
}


// -------------------------------------------------------------------
// Intersection routine
float intersect(Ray ray, Sphere &sphere, int& type) 
{
    int lowest_index = -1; 
    float lowest = numeric_limits<float>::max();
    for (int i = 0; i < g_spheres.size(); i++) {
        mat4 inverse_matrix = mat4();
        Sphere sphere = g_spheres[i];
        if (InvertMatrix(getSphereMatrix(sphere), inverse_matrix)) {
            vec3 origin = toVec3(inverse_matrix * ray.origin);
            vec3 dir = toVec3(inverse_matrix * ray.dir);

            // ray = dir * t + origin
            // sphere = |P| - 1 = 0
            // |ray| - 1 = 0

            float a = dot(dir, dir);
            float b = dot(origin, dir);
            float c = dot(origin, origin) - 1;

            float d = b * b - a * c;
            if (d >= 0) {
                float sol1 = -b / a + sqrtf(d) / a;
                float sol2 = -b / a - sqrtf(d) / a; 

                if (sol1 > 1.0f && sol1 < lowest) {
                    lowest = sol1; 
                    if (sol2 > 1.0f) { type = 1; }      // NORMAL SPHERE
                    else { type = 2; }                  // HOLLOW SPHERE
                    lowest_index = i; 
                }
                if (sol2 > 1.0 && sol2 < lowest) {
                    lowest = sol2;
                    if (sol1 > 1.0f) { type = 1; }      // NORMAL SPHERE
                    else { type = 2; }                  // HOLLOW SPHERE
                    lowest_index = i;
                }
            }
        }
    }

    if (lowest_index != -1) {
        sphere = g_spheres[lowest_index]; 
    }

    return lowest;
}

// -------------------------------------------------------------------
// Ray tracing
vec4 checkColor(vec4 color) 
{
    vec4 result = color; 
    if (color.x < 0.0f) { result.x = 0.0f; }
    if (color.y < 0.0f) { result.y = 0.0f; }
    if (color.z < 0.0f) { result.z = 0.0f; }
    if (color.x > 1.0f) { result.x = 1.0f; }
    if (color.y > 1.0f) { result.y = 1.0f; }
    if (color.z > 1.0f) { result.z = 1.0f; }

    return result; 
}

vec4 trace(const Ray& ray, int depth)
{
    if (depth > MAX_DEPTH) return g_bg;

    Sphere sphere; 
    int ray_intersect = -1; 
    float t = intersect(ray, sphere, ray_intersect);

    // found intersection
    if (ray_intersect != -1) {

        vec4 hit = ray.origin + t * ray.dir; 

        vec4 normal_vec = hit - vec4(sphere.pos.x, sphere.pos.y, sphere.pos.z, 1.0f);
        mat4 trans = Scale(sphere.scale.x, sphere.scale.y, sphere.scale.z);
        mat4 inv;
        InvertMatrix(transpose(trans), inv);
        normal_vec.w = 0;
        normal_vec = normalize(inv * inv * normal_vec);

        vec4 ambient = vec4(g_ambient.x, g_ambient.y, g_ambient.z, 0.0f) * sphere.color * sphere.ka;
        vec4 diffuse = vec4( 0.0f, 0.0f, 0.0f, 0.0f);
        vec4 specular = vec4( 0.0f, 0.0f, 0.0f, 0.0f);


        for (int i = 0; i < g_lights.size(); i++) {
            Light l = g_lights[i];
            Ray light_ray;
            light_ray.dir = normalize(vec4(l.pos.x - hit.x, l.pos.y - hit.y, l.pos.z - hit.z, 0.0f));
            light_ray.origin = hit;

            Sphere block;
            int reflect_intersect = -1; 
            float shadow = intersect(light_ray, block, reflect_intersect);
            if (reflect_intersect != -1 && shadow >= 0.0001f) {
                continue;
            }

            float d = dot(light_ray.dir, normal_vec);

            if (d <= 0.0f) continue;
            diffuse += (sphere.color * d * l.color);

            vec4 r = normalize(((2 * d) * normal_vec) - light_ray.dir); 
            vec4 v = normalize(ray.origin - hit);
            specular += (powf(dot(r, v), sphere.n) * l.color * sphere.ks);
        }


        diffuse = vec4(diffuse.x * sphere.kd, diffuse.y * sphere.kd, diffuse.z * sphere.kd, 0.0f);

        vec4 color = diffuse + ambient + specular;

        if (ray_intersect == 2) {
            color = checkColor(color);
            return color; 
        }

        float r = dot(ray.dir, normal_vec) * -2;
        Ray reflect; 
        reflect.dir = normalize(r * normal_vec + ray.dir);
        reflect.origin = hit;

        vec4 reflection = trace(reflect, depth+1);
        if (reflection.x != g_bg.x && reflection.y != g_bg.y && reflection.z != g_bg.z ) {
            color += reflection * sphere.kr; 
        }

        // cap color 
        color = checkColor(color);


        return color;
    }

    // return BG if no intersection
    return g_bg;
}

vec4 getDir(int ix, int iy)
{
    vec4 dir;

    float x = g_left + ((float) ix / g_width) * (g_right - g_left);
    float y = g_bottom + ((float) iy / g_height) * (g_top - g_bottom);
    float z = -g_near;


    dir = vec4(x, y, z, 0.0f);
    return normalize(dir);
}

void renderPixel(int ix, int iy)
{
    Ray ray;
    ray.origin = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = getDir(ix, iy);

    vec4 color = trace(ray, 0);
    setColor(ix, iy, color);
}

void render()
{
    for (int iy = 0; iy < g_height; iy++)
        for (int ix = 0; ix < g_width; ix++)
            renderPixel(ix, iy);
}


// -------------------------------------------------------------------
// PPM saving

void savePPM(int Width, int Height, const char* fname, unsigned char* pixels) 
{
    FILE *fp;
    const int maxVal=255;

    printf("Saving image %s: %d x %d\n", fname, Width, Height);
    fp = fopen(fname,"wb");
    if (!fp) {
        printf("Unable to open file '%s'\n", fname);
        return;
    }
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", Width, Height);
    fprintf(fp, "%d\n", maxVal);

    for(int j = 0; j < Height; j++) {
        fwrite(&pixels[j*Width*3], 3, Width, fp);
    }

    fclose(fp);
}

void saveFile()
{
    // Convert color components from floats to unsigned chars.
    unsigned char* buf = new unsigned char[g_width * g_height * 3];
    for (int y = 0; y < g_height; y++)
        for (int x = 0; x < g_width; x++)
            for (int i = 0; i < 3; i++)
                buf[y*g_width*3+x*3+i] = (unsigned char)(((float*)g_colors[y*g_width+x])[i] * 255.0f);
    
    savePPM(g_width, g_height, g_output.c_str(), buf);
    delete[] buf;
}

string printVec(vec4 v) {
    string res = "(" + to_string(v.x) + ", " + to_string(v.y) + ", " + to_string(v.z) + ")";
    return res;
}

void debug() 
{
    for (int i = 0; i < g_spheres.size(); i++) {
        cout << "SPHERE " << i + 1 << endl << "POSITION = " << printVec(g_spheres[i].pos) << endl << "SCALE = " << printVec(g_spheres[i].scale) << endl;
    }


}


// -------------------------------------------------------------------
// Main

int main(int argc, char* argv[])
{


    if (argc < 2)
    {
        cout << "Usage: template-rt <input_file.txt>" << endl;
        exit(1);
    }
    loadFile(argv[1]);
    render();
    saveFile();
    debug();
	return 0;
}

