#pragma once

#include <sstream>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cctype>
#include <stack>
#include <vector>
//f=a
//f'=b
//g=c
//g'=d

class expr
{
private:
    struct element
    {
        double number;
        int index;
        int precedence;
        bool left_assoc;
        char var_name;
        enum specifier
        {
            numerical,
            operat,
            function,
            left_p,
            right_p,
            var
        } id;
        expr der(char x);
    };

    std::vector<element> q;
    void push_back(element el) {q.push_back(el);}
    void push_back(expr ex);

public:
    expr simplify();
    expr der(char var);
    expr() {}
    expr(std::string infix);
    double evaluate();
    expr composition(char x, expr other);
    expr composition(char x, double value);
    std::string to_RPN();
};