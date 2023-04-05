#include "expr.hpp"


const int num_f = 9;
const int num_o = 5;
std::string func_names[num_f] = {"ln", "tg", "sin", "cos", "arcsin", "arccos", "arctg", "exp", "sqrt"};
std::string func_derv[num_f] = {"d/c", "d/(cos(c)*cos(c))", "d*cos(c)", "0-d*sin(c)", "d/sqrt(1-c*c)", "0-d/sqrt(1-c*c)", "d/(1+c*c)", "d*exp(c)", "d/(2*sqrt(c))"};
double (*func[num_f])(double x) = {log, tan, sin, cos, asin, acos, atan, exp, sqrt};
char op_names[num_o] = {'&', '%', '*', '/', '^'};
short precedence[num_o] = {2, 2, 3, 3, 4};
bool left_assoc[num_o] = {1, 1, 1, 1, 0};
double (*operators[num_o])(double, double) =
{
    [](double x, double y) { return x - y; },
    [](double x, double y) { return x + y; },
    [](double x, double y) { return x * y; },
    [](double x, double y) { return x / y; },
    [](double x, double y) { return pow(x, y); },
};
std::string op_derv[num_o] = {"b - d", "b + d", "b*c+a*d", "(b*c-a*d)/(c*c)", "b*c*a^(c-1)+d*a^c*ln(a)"};


expr expr::simplify()
{
    std::vector<expr> st;
    for (int i = 0; i < q.size(); i++)
    {
        expr temp;
        //std::cout << q[i].id << std::endl;
        if (q[i].id == element::var)
        {
            temp.push_back(q[i]);
        }
        else if (q[i].id == element::numerical)
        {
            temp.push_back(q[i]);
        }
        else if (q[i].id == element::function)
        {
            expr arg = st.back();
            st.pop_back();

            if (arg.q.back().id == element::numerical)
            {
                element dummy;
                dummy.id = element::numerical;
                dummy.number = func[q[i].index](arg.q.back().number);

                temp.push_back(dummy);
            }
            else
            {
                temp.push_back(arg);
                temp.push_back(q[i]);
            }
            
        }
        else if (q[i].id == element::operat)
        {
            expr arg2 = st.back();
            st.pop_back();
            expr arg1 = st.back();
            st.pop_back();

            if (arg1.q.back().id == element::numerical && arg2.q.back().id == element::numerical)
            {
                element dummy;
                dummy.id = element::numerical;
                dummy.number = operators[q[i].index](arg1.q.back().number, arg2.q.back().number);

                temp.push_back(dummy);
            }
            else if (arg1.q.back().id == element::numerical)
            {
                if (op_names[q[i].index] == '+' && arg1.q.back().number == 0)
                    temp.push_back(arg2);
                else if (op_names[q[i].index] == '*' && arg1.q.back().number == 1)
                    temp.push_back(arg2);
                else if (op_names[q[i].index] == '*' && arg1.q.back().number == 0)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '/' && arg1.q.back().number == 0)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '^' && arg1.q.back().number == 0)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '^' && arg2.q.back().number == 1)
                    temp.push_back(arg1);
            }
            else if (arg2.q.back().id == element::numerical)
            {
                if (op_names[q[i].index] == '%' && arg2.q.back().number == 0)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '&' && arg2.q.back().number == 0)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '*' && arg2.q.back().number == 1)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '*' && arg2.q.back().number == 0)
                    temp.push_back(arg2);
                else if (op_names[q[i].index] == '/' && arg2.q.back().number == 1)
                    temp.push_back(arg1);
                else if (op_names[q[i].index] == '^' && arg2.q.back().number == 0)
                {
                    element dummy;
                    dummy.id = element::numerical;
                    dummy.number = 1;
                    temp.push_back(dummy);
                }
                else if (op_names[q[i].index] == '&' && arg2.q.back().number == 1)
                    temp.push_back(arg1);
            }
            if (temp.q.size() == 0)
            {
                temp.push_back(arg1);
                temp.push_back(arg2);
                temp.push_back(q[i]);
            }
            
        }
        st.push_back(temp);
    }
    *this = st[0];
    return *this;
}

expr expr::element::der(char x)
{
    if((id == var && var_name != x) || id == numerical)
        return expr("0");
    else if(id == var)
        return expr("1");
    else if(id == function)
        return expr(func_derv[index]);
    else if(id == operat)
        return expr(op_derv[index]);
    return expr();
}

expr expr::der(char x)
{
    
    std::stack<expr> f, fprim;
    for(int i = 0; i < q.size(); i++)
    {
        expr el_der = q[i].der(x);
        if(q[i].id == element::var || q[i].id == element::numerical)
        {
            expr temp;
            temp.push_back(q[i]);
            fprim.push(el_der);
            f.push(temp);
        }
        else if(q[i].id == element::function)
        {
            expr g = f.top();
            f.pop();
            expr gprim = fprim.top();
            fprim.pop();

            el_der = el_der.composition('c', g).composition('d', gprim);
            g.push_back(q[i]);

            f.push(g);
            fprim.push(el_der);
        }
        else if(q[i].id == element::operat)
        {
            expr second = f.top();
            f.pop();
            expr first = f.top();
            f.pop();
            expr second_prim = fprim.top();
            fprim.pop();
            expr first_prim = fprim.top();
            fprim.pop();

            el_der = el_der.composition('a', first).composition('b', first_prim).composition('c', second).composition('d', second_prim);

            first.push_back(second);
            first.push_back(q[i]);

            f.push(first);
            fprim.push(el_der);
        }
    }
    fprim.top().simplify();
    return fprim.top();
}

expr expr::composition(char x, expr other)
{
    expr new_one;
    for(int i = 0; i < q.size(); i++)
    {
        if (q[i].id == element::var && q[i].var_name == x)
            new_one.push_back(other);
        else
            new_one.push_back(q[i]);
    }
    return new_one;
}

expr expr::composition(char x, double value)
{
    expr new_one;
    element el;
    el.id = element::numerical;
    el.number = value;
    for(int i = 0; i < q.size(); i++)
    {
        if (q[i].id == element::var && q[i].var_name == x)
            new_one.push_back(el);
        else
            new_one.push_back(q[i]);
    }
    return new_one;
}

void expr::push_back(expr ex)
{
    for(int i = 0; i < ex.q.size(); i++)
        q.push_back(ex.q[i]);
}

double expr::evaluate()
{
    std::stack<double> st;
    for (int i = 0; i < q.size(); i++)
    {
        if(q[i].id == element::numerical)
            st.push(q[i].number);
        else if (q[i].id == element::operat)
        {
            double second = st.top();
            st.pop();
            double first = st.top();
            st.pop();
            double result = operators[q[i].index](first, second);
            st.push(result);
        }
        else if(q[i].id == element::function)
        {
            double x = st.top();
            st.pop();
            double result = func[q[i].index](x);
            st.push(result);
        }
    }
    return st.top();
}

std::string expr::to_RPN()
{
    std::stringstream rpn;

    for(int i = 0; i < q.size(); i++)
    {
        if(q[i].id == element::numerical)
            rpn << q[i].number << " ";
        else if(q[i].id == element::operat)
            rpn << op_names[q[i].index] << " ";
        else if(q[i].id == element::function)
            rpn << func_names[q[i].index] << " ";
        else if(q[i].id == element::var)
            rpn << q[i].var_name << " ";
    }
    return rpn.str();
}

expr::expr(std::string infix)
{
    std::stack<element> st;
    //a bit changing the string
    for(int i = 0; i < num_f; i++)
    {
        int found = 0;
        while (true)
        {
            found = infix.find(func_names[i], found);
            if(found == std::string::npos)
                break;
            if(found > 2 && infix.substr(found-3, 3) == "arc")
            {
                found += 1;
                continue;
            }
            infix.insert(found, 1, '0' + func_names[i].length());
            infix.insert(found, 1, '#');
            found += 3;
        }
    }
    int found = 0;
    while (true)
    {
        found = infix.find("+", found);
        if(found == std::string::npos)
            break;
        infix[found] = '%';
        found += 1;
    }
    found = 0;
    while (true)
    {
        found = infix.find("-", found);
        if(found == std::string::npos)
            break;
        infix[found] = '&';
        found += 2;
    }
    //using stringstream is simpler
    std::stringstream ss;
    ss << infix;
    while (!ss.eof())
    {
        double num;
        char c;
        //check if it is a number
        if (ss >> num)
        {
            element el;
            el.number = num;
            el.id = element::numerical;
            q.push_back(el);
            continue;
        }
        ss.clear();
        //if not, then read a character
        if (ss >> c)
        {
            //if it is a function
            if(c == '#')
            {
                int count;
                ss >> count;
                char name[7];
                int i;
                for(i = 0; i < count; i++)
                    ss >> name[i];
                name[i] = 0;
                for(int i = 0; i < num_f; i++)
                {
                    if(func_names[i] == name)
                    {
                        element el;
                        el.index = i;
                        el.id = element::function;
                        st.push(el);
                    }
                }
            }
            //parenthesis?
            else if(c == '(')
            {
                element el;
                el.id = element::left_p;
                st.push(el);
            }
            else if(c == ')')
            {
                while(st.top().id != element::left_p)
                {
                    q.push_back(st.top());
                    st.pop();
                }
                st.pop();
            }
            //if it is a variable
            else if(isalpha(c))
            {
                element el;
                el.id = element::var;
                el.var_name = c;
                q.push_back(el);
            }
            //if it is an evaluate()
            else
            {
                for(int i = 0; i < num_o; i++)
                {
                    if(c == op_names[i])
                    {
                        element el;
                        el.index = i;
                        el.id = element::operat;
                        el.precedence = precedence[i];
                        el.left_assoc = left_assoc[i];

                        while(!st.empty() && (st.top().id == element::operat && (st.top().precedence > el.precedence || (st.top().precedence == el.precedence && el.left_assoc == true)) || st.top().id == element::function))
                        {
                            q.push_back(st.top());
                            st.pop();
                        }

                        st.push(el);
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
    while(!st.empty())
    {
        q.push_back(st.top());
        st.pop();
    }
}