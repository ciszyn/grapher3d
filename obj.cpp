#include "obj.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "point.hpp"
#include "expr.hpp"
#include <vector>
#include <fstream>
#include <iostream>

glm::vec3 operator*(float x, glm::vec3 vec)
{
    return glm::vec3(x * vec.x, x * vec.y, x * vec.z);
}

obj::obj(std::string func, float delta, glm::vec3 source, bool (*isBlocked)(glm::vec3 &v)) : f(func), isBlocked(isBlocked), delta(delta)
{
    grad1 = f.der('x');
    grad2 = f.der('y');
    grad3 = f.der('z');
    point first(source, f, grad1, grad2, grad3);
    vertices.push_back(first);

    polygons.push_back(std::vector<int>()); //should be vector<point>

    //preparing hexagon
    for(int i = 0; i < 6; i++)
    {
        glm::vec3 hex = first.p + delta * cos(i * M_PI / 3) * first.t1 + delta * sin(i * M_PI / 3) * first.t2;
        vertices.push_back(point(hex, f, grad1, grad2, grad3));
        polygons[0].push_back(i + 1);   //push_back(point)?
    }

    for(int i = 0; i < 5; i++)
    {
        triangles.push_back(triangle(0, i+1, i+2));
    }
    triangles.push_back(triangle(0, 6, 1));

    //i is responsible for a given polygon
    //j is resposible for a point in this polygon
    for(int i = 0; i < polygons.size(); i++)
    {
        bool check = false;
        int blocked = 0;

        while (polygons[i].size() - blocked > 1)
        {
            blocked = 0;

            //angle calculation
            for(int j = 0; j < polygons[i].size(); j++) //here you need only to change it in polygons[i]
            {
                int index = polygons[i][j];

                if (vertices[index].omega == -1)
                {
                    int first = j == 0 ? polygons[i][polygons[i].size()-1] : polygons[i][j-1];
                    int third = j == polygons[i].size() - 1 ? polygons[i][0] : polygons[i][j+1];
                    vertices[index].omega = calc_angle(vertices[first], vertices[index], vertices[third]);
                }
            }

            //complementing single triangles
            for(int j = 0; j < polygons[i].size(); j++)
            {
                int index = polygons[i][j];
                if(vertices[index].omega <= M_PI/3) //yeah it is too complicated, omega change only in polygons[i]
                {
                    int first = j == 0 ? polygons[i][polygons[i].size() - 1] : polygons[i][j - 1];
                    int third = j == polygons[i].size() - 1 ? polygons[i][0] : polygons[i][j+1];
                    int fourth = j + 2 > polygons[i].size() - 1 ? polygons[i][j+2-polygons[i].size()] : polygons[i][j + 2];
                    int zero = j < 2 ? polygons[i][polygons[i].size() - 2 + j] : polygons[i][j - 2];
                    vertices[first].omega = vertices[third].omega = -1;
                    triangles.push_back(triangle(index, first, third));
                    polygons[i].erase(polygons[i].begin() + j);

                    vertices[first].omega = calc_angle(vertices[zero], vertices[first], vertices[third]);
                    vertices[third].omega = calc_angle(vertices[first], vertices[third], vertices[fourth]);
                    j -= 1;
                }
            }

            //distance check inside a polygon
            //fill bridge with triangles
            for(int j = 0; j < polygons[i].size(); j++) //thats awful
                for(int k = j+3; k < polygons[i].size(); k++)
                {
                    if((j == 0 && (k==polygons[i].size()-1 || k==polygons[i].size()-2 )) || (j==1 && k==polygons[i].size()-1))
                        continue;
                    int index1 = polygons[i][j];
                    int index2 = polygons[i][k];
                    float dist;
                    if ((dist=distance(vertices[index1].p, vertices[index2].p)) < delta)
                    {
                        int first = j == 0 ? polygons[i][polygons[i].size() - 1] : polygons[i][j - 1];
                        float between_omega = calc_angle(vertices[first], vertices[index1], vertices[index2]);
                        if(between_omega > vertices[index1].omega)
                            break;
                        vertices[index1].omega = vertices[index2].omega = -1;
                        point begining = vertices[index1];
                        point ending = vertices[index2];
                        vertices.push_back(begining);
                        vertices.push_back(ending);
                        //create new polygon
                        polygons.push_back(std::vector<int>());
                        polygons[polygons.size()-1].push_back(vertices.size()-2);
                        for(int l = j+1; l < k; l++)
                            polygons[polygons.size()-1].push_back(polygons[i][l]);
                        polygons[polygons.size()-1].push_back(vertices.size()-1);

                        //erase these points from previous polygon
                        polygons[i].erase(polygons[i].begin() + j + 1, polygons[i].begin()+k);
                        int secondary = j + 1 == polygons[i].size() ? 0 : j + 1;
                        vertices[index1].omega = calc_angle(j, polygons[i], vertices);
                        vertices[index2].omega = calc_angle(secondary, polygons[i], vertices);

                        if (vertices[index1].omega > vertices[index2].omega)
                        {
                            calc_vert(vertices, triangles, polygons[i], secondary);
                            calc_vert(vertices, triangles, polygons[i], j);
                        }
                        else
                        {
                            int n = calc_vert(vertices, triangles, polygons[i], j);
                            calc_vert(vertices, triangles, polygons[i], secondary+n-2);
                        }
                        
                        goto escape;
                    }
                }
            escape:

            //distance check between polygons
            //fill bridge with triangles
            for(int j = 0; j < polygons.size(); j++)
            {
                if(i == j)
                    continue;
                for(int k = 0; k < polygons[i].size(); k++)
                    for(int l = 0; l < polygons[j].size(); l++)
                    {
                        int index1 = polygons[i][k];
                        int index2 = polygons[j][l];
                        float dis;
                        //change it
                        
                        if ((dis = distance(vertices[index1].p, vertices[index2].p))<delta)
                        {

                            int first = k == 0 ? polygons[i][polygons[i].size() - 1] : polygons[i][k - 1];
                            float between_omega = calc_angle(vertices[first], vertices[index1], vertices[index2]);

                            if(between_omega > vertices[index1].omega)
                                continue;


                            //throw it there, is it good ?
                            std::vector<int> temp;
                            for(int m = 0; m <= k; m++)
                                temp.push_back(polygons[i][m]);
                            for(int m = l; m < polygons[j].size(); m++)
                                temp.push_back(polygons[j][m]);
                            for(int m = 0; m <= l; m++)
                                temp.push_back(polygons[j][m]);
                            for(int  m = k; m < polygons[i].size(); m++)
                                temp.push_back(polygons[i][m]);

                            vertices[index1].omega = calc_angle(k, temp, vertices);
                            vertices[index2].omega = calc_angle(k + 1, temp, vertices);

                            if(i > j)
                            {
                                polygons.erase(polygons.begin() + i);
                                polygons.erase(polygons.begin() + j);
                            }
                            else
                            {
                                polygons.erase(polygons.begin() + j);
                                polygons.erase(polygons.begin() + i);
                            }

                            polygons.push_back(temp);
                            i = polygons.size() - 1;

                            if (vertices[index1].omega < vertices[index2].omega)
                            {
                                int n = calc_vert(vertices, triangles, polygons[i], k);
                                calc_vert(vertices, triangles, polygons[i], k+n - 1);
                            }
                            else
                            {
                                calc_vert(vertices, triangles, polygons[i], k+1);
                                calc_vert(vertices, triangles, polygons[i], k);
                            }
                            vertices[index1].omega = vertices[index2].omega = -1;
                            
                            goto escape1;
                        }
                    }
            }
            escape1:

            //looking for a vertex with smallest angle
            int smallest = 0;
            for(int j = 0; j < polygons[i].size(); j++)
            {
                int index = polygons[i][j];
                if(vertices[index].blocked)
                {
                    blocked++;
                    continue;
                }
                if (vertices[index].omega == -1)
                {
                    int first = j == 0 ? polygons[i][polygons[i].size() - 1] : polygons[i][j - 1];
                    int third = j == polygons[i].size() - 1 ? polygons[i][0] : polygons[i][j + 1];
                    vertices[index].omega = calc_angle(vertices[first], vertices[index], vertices[third]);
                }

                int indemin = polygons[i][smallest];
                smallest = vertices[indemin].omega < vertices[index].omega ? smallest : j;
                //check if you should expand it!
            }
            calc_vert(vertices, triangles, polygons[i], smallest);
            if (polygons[i].size() == 3)
            {
                triangles.push_back(triangle(polygons[i][1], polygons[i][0], polygons[i][2]));
                break;
            }
        }
    }
}

float obj::calc_angle(point v1, point p, point v2)
{
    glm::mat3x3 sys(p.n.x, p.n.y, p.n.z, p.t1.x, p.t1.y, p.t1.z, p.t2.x, p.t2.y, p.t2.z);
    glm::mat3x3 transform = glm::inverse(sys);

    glm::vec3 v1prim = transform * (v1.p-p.p);
    glm::vec3 v2prim = transform * (v2.p-p.p);

    float omega = atan2(v2prim.z, v2prim.y) - atan2(v1prim.z, v1prim.y);
    if(omega < 0)
        omega += 2 * M_PI;

    return omega;
}

float obj::calc_angle(int index, std::vector<int> polygon, std::vector<point> vertices)
{
    int first = index == 0 ? polygon[polygon.size()-1] : polygon[index-1];
    int third = index == polygon.size() - 1 ? polygon[0] : polygon[index+1];

    point p = vertices[polygon[index]];
    point v1 = vertices[first];
    point v2 = vertices[third];

    glm::mat3x3 sys(p.n.x, p.n.y, p.n.z, p.t1.x, p.t1.y, p.t1.z, p.t2.x, p.t2.y, p.t2.z);
    glm::mat3x3 transform = glm::inverse(sys);

    glm::vec3 v1prim = transform * (v1.p-p.p);
    glm::vec3 v2prim = transform * (v2.p-p.p);

    float omega = atan2(v2prim.z, v2prim.y) - atan2(v1prim.z, v1prim.y);
    if(omega < 0)
        omega += 2 * M_PI;

    return omega;
}

int obj::calc_vert(std::vector<point> &vertices, std::vector<triangle> &triangles, std::vector<int> &polygon, int index)
{
    int second = polygon[index];
    int first = index == 0 ? polygon[polygon.size() - 1] : polygon[index - 1];
    int third = index == polygon.size() - 1 ? polygon[0] : polygon[index+1];

    point v1 = vertices[first];
    point p = vertices[second];
    point v2 = vertices[third];

    if(p.omega == -1)
        p.omega = calc_angle(v1, p, v2);
    
    int n = int(3 * p.omega / M_PI + 1);
    float d_omega = p.omega / n;
    while (d_omega < 0.8 && n > 1)
    {
        n -= 1;
        d_omega = p.omega / n;
    }
    if(n == 1 && d_omega > 0.8 && glm::distance(v1.p, v2.p) > 1.2)
    {
        n = 2;
        d_omega = p.omega / n;
    }
    else if(p.omega < 3 && (glm::distance(v1.p, p.p) < delta/2 || glm::distance(v2.p, p.p) < delta/2))
    {
        n = 1;
        d_omega = p.omega;
    }
    if (n == 1)
    {
        triangles.push_back(triangle(second, first, third));
    }
    else
    {
        glm::mat3x3 sys(p.n.x, p.n.y, p.n.z, p.t1.x, p.t1.y, p.t1.z, p.t2.x, p.t2.y, p.t2.z);
        glm::mat3x3 transform = glm::inverse(sys);

        glm::vec3 v1prim = transform * v1.p;
        glm::vec3 v2prim = transform * v2.p;

        v1prim.x = v2prim.x = 0.f;

        //map to tangent plane of p
        v1prim = sys * v1prim;
        v2prim = sys * v2prim;
        
        int vert_size = vertices.size();
        for (int i = 1; i < n; i++)
        {
            glm::vec3 pos = p.p + (glm::mat3x3)glm::rotate(glm::mat4(1.f), i * d_omega, p.n) * (delta / distance(v1.p, p.p) * (v1.p - p.p));
            point new_one(pos, f, grad1, grad2, grad3);
            new_one.blocked = isBlocked(new_one.p);

            vertices.push_back(new_one);

            if(i == 1)
                triangles.push_back(triangle(second, first, vert_size));
            else
                triangles.push_back(triangle(second, vert_size-1, vert_size));
            vert_size++;
        }
        triangles.push_back(triangle(third, second, vert_size-1));
    }
    vertices[first].omega = vertices[third].omega = -1;

    polygon.erase(polygon.begin() + index);
    for(int k = 1; k < n; k++)
        polygon.insert(polygon.begin() + index, vertices.size() - k);

    return n;
}

std::ostream& operator<< (std::ostream &fout, obj &object)
{
    int v_size = object.vertices.size();
    int f_size = object.triangles.size();

    for(int i = 0; i < v_size; i++)
        fout << "v " << object.vertices[i].p.x << " " << object.vertices[i].p.y << " " << object.vertices[i].p.z << std::endl;
    for(int i = 0; i < v_size; i++)
        fout << "vn " << object.vertices[i].n.x << " " << object.vertices[i].n.y << " " << object.vertices[i].n.z << std::endl;
    for(int i = 0; i < f_size; i++)
        fout << "f " << object.triangles[i].a + 1 << "//" << object.triangles[i].a + 1 << " " << object.triangles[i].b + 1 << "//" << object.triangles[i].b + 1 << " " << object.triangles[i].c + 1 << "//" << object.triangles[i].c + 1 << std::endl;
    return fout;
}

std::vector<glm::vec3> obj::new_v_array()
{
    std::vector<glm::vec3> arr;
    for (int i = 0; i < triangles.size(); i++)
    {
        point vertex = vertices[triangles[i].a];
        arr.push_back(vertex.p);
        vertex = vertices[triangles[i].b];
        arr.push_back(vertex.p);
        vertex = vertices[triangles[i].c];
        arr.push_back(vertex.p);
    }
    return arr;
}

std::vector<glm::vec3> obj::new_vn_array()
{
    std::vector<glm::vec3> arr;
    for (int i = 0; i < triangles.size(); i++)
    {
        point vertex = vertices[triangles[i].a];
        arr.push_back(vertex.n);
        vertex = vertices[triangles[i].b];
        arr.push_back(vertex.n);
        vertex = vertices[triangles[i].c];
        arr.push_back(vertex.n);
    }
    return arr;
}