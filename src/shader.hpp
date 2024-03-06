#pragma once

#include <string>

class shaderStage
{
    public:
    std::string src; //intending to only use spir-V shaders, but maybe add compiling in runtime
};

class shaderProgram
{
    public:

    shaderProgram();
    shaderProgram(shaderProgram&&);
    shaderProgram& operator=(shaderProgram&&);
    shaderProgram(shaderProgram&);
    shaderProgram& operator=(shaderProgram&);
    ~shaderProgram();


    void use();
    static void unuse();

    private:

    struct members
    {
        unsigned int ProgramID;
    } m;
};