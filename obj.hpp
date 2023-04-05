#pragma once
#include <glm/glm.hpp>
#include "point.hpp"
#include <vector>
#include <iostream>
#include "expr.hpp"

class obj
{
    friend std::ostream &operator<<(std::ostream &fout, obj &object);

private:
    struct triangle
    {
        int a, b, c;
        triangle(int a, int b, int c) : a(a), b(b), c(c) {}
    };
    float calc_angle(point v1, point p, point v2);
    float calc_angle(int index, std::vector<int> polygon, std::vector<point> vertices);
    int calc_vert(std::vector<point> &vertices, std::vector<triangle> &triangles, std::vector<int> &polygon, int index);

    //float (*f)(glm::vec3);
    //glm::vec3 (*grad)(glm::vec3 p);
    expr f;
    expr grad1;
    expr grad2;
    expr grad3;

    bool (*isBlocked)(glm::vec3 &v);
    float delta;

    std::vector<point> vertices;
    std::vector<std::vector<int>> polygons;
    std::vector<triangle> triangles;
    
public:
    obj(std::string f, float delta = 0.3f, glm::vec3 first = glm::vec3(1, 1, 1), bool (*isBlocked)(glm::vec3 &v) = [](glm::vec3 &p) { return false; });
    std::ostream &operator<<(std::ostream &fout);
    std::vector<glm::vec3> new_v_array();
    std::vector<glm::vec3> new_vn_array();
};