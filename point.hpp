#pragma once
#include <glm/glm.hpp>
#include "expr.hpp"
#include <glm/gtx/string_cast.hpp>

//you have to decide what condition stops newton method

class point
{
public:
    glm::vec3 p;
    glm::vec3 n;
    glm::vec3 t1, t2;
    bool blocked;
    double omega;

    point(glm::vec3 q, expr f, expr grad1, expr grad2, expr grad3,/*float (*f)(glm::vec3), glm::vec3 (*grad)(glm::vec3),*/ float epsilon = 0.01)
    {
        glm::vec3 p1;
        p = q;
        int i = 3;
        while (true)
        {
            expr grad11 = grad1.composition('x', p.x).composition('y', p.y).composition('z', p.z);
            expr grad22 = grad2.composition('x', p.x).composition('y', p.y).composition('z', p.z);
            expr grad33 = grad3.composition('x', p.x).composition('y', p.y).composition('z', p.z);

            n.x = grad11.evaluate();
            n.y = grad22.evaluate();
            n.z = grad33.evaluate();

            //n = grad(p);
            expr f1 = f.composition('x', p.x).composition('y', p.y).composition('z', p.z);
            float fvalue = f1.evaluate();

            p1 = p - fvalue / (n.x * n.x + n.y * n.y + n.z * n.z) * n;

            //if(fvalue < epsilon)
            if (distance(p, p1) < epsilon)
            {
                p = p1;
                break;
            }
            std::swap(p, p1);
        }
        blocked = false;
        double l = length(n);
        n /= l;

        if (n.x != 0 || n.y != 0)
        {
            t1.x = n.y;
            t1.y = -n.x;
            t1.z = 0;
        }
        else
        {
            t1.x = n.z;
            t1.y = 0;
            t1.z = -n.x;
        }

        t1 = t1 / length(t1);

        t2 = glm::cross(n, t1);

        omega = -1;
    }
};