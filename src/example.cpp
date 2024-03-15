#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "application.hpp"
#include "shader.hpp"
#include "vertexArray.hpp"
#include "buffer.hpp"
#include "obj_importer.hpp"

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;

inline static
void BREAKPOINT()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    std::cout << "Breakpoint Triggered!\n";
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );

    if (type == GL_DEBUG_TYPE_ERROR) 
        BREAKPOINT();
}

auto get_time()
{
    return Time::now();
}

class Model
{
    public:
    obj Mesh;
    vertexArray VAO;
    buffer VBO = buffer(GL_ARRAY_BUFFER);
    glm::mat4 Transform = glm::mat4(1.0);

    Model() : VAO(3) {}

    void init()
    {
        Mesh.import("assets/teapot.obj");
        Transform = glm::translate(Transform, glm::vec3(0.0, 0.0, -10.0));
        VAO.Attributes[0] = {0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0};
        VAO.Attributes[1] = {1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)offsetof(obj::vertex, texcoord)};
        VAO.Attributes[2] = {2, 3, GL_FLOAT, GL_TRUE,  8*sizeof(float), (void*)offsetof(obj::vertex, normal)};
        VBO.generate();
        VBO.bufferData(Mesh.getVertices().size()*sizeof(obj::vertex), Mesh.getVertices().data(), GL_STATIC_DRAW);
        VAO.generateVAO();
        VAO.setVertexAttributes(VBO, 0, sizeof(obj::vertex));
    }
};

class MyGame : public application 
{
    public:

    MyGame(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    struct camera
    {
        glm::vec3 front = glm::vec3(0.0,0.0,-1.0);
        glm::vec3 right = glm::vec3(1.0,0.0,0.0);
        glm::vec3 up = glm::vec3(0.0,1.0,0.0);
        glm::vec2 rotation = glm::vec2(0.0);
        glm::vec3 pos = glm::vec3(0.0, 0.0, -4.0);
        glm::mat4 view = glm::mat4(1);
        bool update = false;

        void rotate()
        {
            update = true;

            glm::vec2 c(glm::cos(rotation)), s(glm::sin(rotation));

            front.x = s.x*c.y;
            front.y = s.y;
            front.z = -c.x*c.y;
            front = glm::normalize(front);

            right = glm::normalize(glm::cross(front, glm::vec3(0.0,1.0,0.0)));
            up =    glm::normalize(glm::cross(right, front));
        }

        void makeView()
        {
            view = glm::lookAt(pos, pos+front, glm::vec3(0.0,1.0,0.0));
        }

    } player;

    float dt = 0.0;

    glm::mat4 Projection;
    Model teapot;
    shaderProgram sProgram;
    Shader<Vertex> vShader;
    Shader<Fragment> fShader;

    buffer UBO = buffer(GL_UNIFORM_BUFFER);

    void init()
    {
        glClearColor(0.0,0.0,0.0,1.0);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        teapot.init();
        player.makeView();
        Projection = glm::perspective(glm::radians(70.0f), win.getWidth() / (float)win.getHeight(), 0.1f, 1000.0f);
        UBO.generate();
        UBO.bufferData(3*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        UBO.bindBufferBase(0);
        UBO.bufferSubData(sizeof(glm::mat4), 0, &Projection[0][0]);
        UBO.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.view[0][0]);
        UBO.bufferSubData(sizeof(glm::mat4), 2*sizeof(glm::mat4), &teapot.Transform[0][0]);
        vShader.loadBinary("shader/example.vert.spv");
        vShader.specialize();
        fShader.loadBinary("shader/example.frag.spv");
        fShader.specialize();
        sProgram.attachShader(vShader);
        sProgram.attachShader(fShader);
        sProgram.linkProgram();
    }

    void processKeyEvents(const SDL_KeyboardEvent &E)
    {
        switch(E.type)
        {
            case SDL_KEYUP:
            break;
            case SDL_KEYDOWN:
                switch(E.keysym.sym)
                {
                    case SDLK_w:
                    case SDLK_UP:
                        player.update = true;
                        player.pos += player.front*dt;
                    break;

                    case SDLK_s:
                    case SDLK_DOWN:
                        player.update = true;
                        player.pos -= player.front*dt;
                    break;

                    case SDLK_d:
                    case SDLK_LEFT:
                        player.update = true;
                        player.pos -= player.right*dt;
                    break;

                    case SDLK_a:
                    case SDLK_RIGHT:
                        player.update = true;
                        player.pos += player.right*dt;
                    break;

                    case SDLK_SPACE:
                        player.update =true;
                        player.pos.y += dt;
                    break;

                    case SDLK_LCTRL:
                        player.update = true;
                        player.pos.y -= dt;
                        break;
                }
                break;
        }
    }

    void processMouseEvents(const SDL_MouseMotionEvent& E)
    {
        glm::vec2 r(E.x, -E.y);
        r *= 0.001f;

        r.x = fmodf(r.x, 2*M_PIf);
        if (r.x < 0) r.x += 2*M_PIf;
        r.y = glm::clamp(r.y, -M_PI_2f*0.98f, M_PI_2f*0.98f);
        
        if (r != player.rotation)
        {
            player.rotation = r;
            player.rotate();
        }
    }

    void processWindowEvents(const SDL_WindowEvent &E)
    {
        switch(E.type)
        {
            case SDL_WINDOWEVENT_RESIZED:
                win.set_size(E.data1, E.data2);
                glViewport(0,0,E.data1,E.data2);
                break;
        }
    }

    void process_events() override
    {
        SDL_Event E;
        while (SDL_PollEvent(&E))
        {
            switch(E.type) 
            {
                case SDL_QUIT:
                    win.set_shouldClose(true);
                    break;
                case SDL_WINDOWEVENT:
                    processWindowEvents(E.window);
                    break;
                case SDL_MOUSEMOTION:
                    processMouseEvents(E.motion);
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    processKeyEvents(E.key);
                    break;
            }
        }
    }

    void mainLoop() override
    {
        init();
        while(!win.shouldClose())
        {
            auto T0 = get_time();
            process_events();

            if (player.update)
            {
                player.makeView();
                UBO.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.view[0][0]);
                player.update = false;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            teapot.VAO.bind();
            sProgram.use();
            glDrawArrays(GL_TRIANGLES, 0, teapot.Mesh.getVertices().size());
            teapot.VAO.unbind();
            sProgram.unuse();

            win.swapBuffer();
            auto T1 = get_time();
            fsec T = (T1 - T0);
            dt = T.count();
        }
    }

};

int main()
{
    MyGame App(500, 500);
    
    App.mainLoop();

    return 0;
}