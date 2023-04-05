#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <fstream>
#include "shader.h"
#include "obj.hpp"
#include "expr.hpp"


bool isBlocked(glm::vec3 &v)
{
    bool val = false;
    if (v.x > 3)
    {
        v.x = 3;
        val = true;
    }
    if (v.x < -3)
    {
        v.x = -3;
        val = true;
    }
    if (v.y > 3)
    {
        v.y = 3;
        val = true;
    }
    if (v.y < -3)
    {
        v.y = -3;
        val = true;
    }
    if (v.z > 3)
    {
        v.z = 3;
        val = true;
    }
    if (v.z < -3)
    {
        v.z = -3;
        val = true;
    }
    return val;
}

int main(int argc, char **argv)
{
    std::string equation;
    bool (*f_b)(glm::vec3 &) = [](glm::vec3 &){return false;};

    if(argc == 1)
        return 0;
    if (strcmp(argv[1], "-b") == 0)
    {
        f_b = isBlocked;
        equation = argv[2];
    }
    else
        equation = argv[1];
    
    obj object(equation, 0.3, glm::vec3(0, 2, 0), f_b);
    // 16*16*z*z-(1-(x/6)*(x/6)-(y/3.5)*(y/3.5))*((x-3.9)*(x-3.9)+y*y-1.2*1.2)*(x*x+y*y-1.2*1.2)*((x+3.9)*(x+3.9)+y*y-1.2*1.2)
    // (sqrt(x*x+y*y)-10)^2+z^*z=25
    int width = 800;
    int height = 600;
    sf::Window window(sf::VideoMode(width, height), "grapher", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialize glew\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glClearColor(.1f, .1f, .1f, 1.f);

    std::vector<glm::vec3> v = object.new_v_array();
    std::vector<glm::vec3> vn = object.new_vn_array();


    GLuint vID;
    glGenVertexArrays(1, &vID);
    glBindVertexArray(vID);

    GLuint vbuffer;
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(glm::vec3), &v[0], GL_STATIC_DRAW);

    GLuint vnbuffer;
    glGenBuffers(1, &vnbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vnbuffer);
    glBufferData(GL_ARRAY_BUFFER, vn.size() * sizeof(glm::vec3), &vn[0], GL_STATIC_DRAW);

    float r = 16;

    glm::mat4 Projection = glm::perspective(glm::radians(45.f), float(width)/float(height), .1f, 100.f);
    glm::mat4 View = glm::lookAt(glm::vec3(0, 0, r), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 Model = glm::mat4(1.f);
    glm::mat4 MVP = Projection * View * Model;


    //needs to be done
    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint LightID = glGetUniformLocation(programID, "lightPos");

    window.setActive(false);

    bool running = true;
    int mouse_start_x, mouse_start_y;

    float ax = 0;
    float ay = 0;

    float prev_ax = 0;
    float prev_ay = 0;

    glm::vec3 dir(0, 0, 1);
    glm::vec3 up(0, 1, 0);
    glm::vec3 right(-1, 0, 0);

    bool mesh = false;
    bool back_culling = true;

    while (running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                running = false;
            else if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W && !mesh)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                mesh = true;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W && mesh)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                mesh = false;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S && !back_culling)
            {
                glEnable(GL_CULL_FACE);
                back_culling = true;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S && back_culling)
            {
                glDisable(GL_CULL_FACE);
                back_culling = false;
            }
            else if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                mouse_start_x = sf::Mouse::getPosition().x;
                mouse_start_y = sf::Mouse::getPosition().y;
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Left)
            {
                prev_ax = ax;
                prev_ay = ay;
            }
            else if (event.type == sf::Event::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                ax = prev_ax - (sf::Mouse::getPosition().x - mouse_start_x)/100.;
                ay = prev_ay + (sf::Mouse::getPosition().y - mouse_start_y)/100.;

                right = glm::vec3(sin(ax-3.14f/2.f), 0, cos(ax-3.14f/2.f));
                glm::vec3 dir(cos(ay) * sin(ax), sin(ay), cos(ay)*cos(ax));
                up = glm::cross(right, dir);

                View = glm::lookAt(glm::vec3(r * cos(ay) * sin(ax), r*sin(ay), r*cos(ax)*cos(ay)), glm::vec3(0, 0, 0), up);

                MVP = Projection * View * Model;
            }
            else if(event.type == sf::Event::MouseWheelScrolled)
            {
                r -= event.mouseWheelScroll.delta;

                View = glm::lookAt(glm::vec3(r * cos(ay) * sin(ax), r*sin(ay), r*cos(ax)*cos(ay)), glm::vec3(0, 0, 0), up);

                MVP = Projection * View * Model;
            }
        }
        window.setActive(true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);

        glm::vec3 lightPos =  glm::vec3(r*sin(ax) * cos(ay), r*sin(ay), r*cos(ax)*cos(ay)) + 0.5f * r * up - 0.5f * r * right;
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vnbuffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glDrawArrays(GL_TRIANGLES, 0, v.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        window.setActive(false);
        window.display();
    }

    glDeleteBuffers(1, &vbuffer);
    glDeleteBuffers(1, &vnbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &vID);
    return 0;
}