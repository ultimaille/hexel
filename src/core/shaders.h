#pragma once

#include <fstream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

struct ShaderManager : public std::map<std::string,GLuint> {
    GLuint compileShader(GLenum type,const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader,1,&source,nullptr);
        glCompileShader(shader);
        GLint success = GL_FALSE;
        glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
        if(!success) {
            char log[1024] = {};
            glGetShaderInfoLog(shader,sizeof(log),nullptr,log);
            Log::error("shader compilation failed",log);
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER,vertexSource);
        if (!vertexShader)
            return 0;

        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER,fragmentSource);
        if (!fragmentShader) {
            glDeleteShader(vertexShader);
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program,vertexShader);
        glAttachShader(program,fragmentShader);
        glLinkProgram(program);

        GLint success = GL_FALSE;
        glGetProgramiv(program,GL_LINK_STATUS,&success);

        if (!success) {
            char log[1024] = {};
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            Log::error("shader linking failed", log);
            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    std::string loadShader(const std::string& filename) {
        std::ifstream file(filename);
        if (!file)
            Log::error("Cannot open shader: ", filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void add(std::string path,std::string shader_name) {
        if (contains(shader_name)) {
            Log::error("duplicate shader name:", shader_name);
            return;
        }
        operator [](shader_name) = createShaderProgram(
                loadShader(path+shader_name+std::string(".vert")).c_str(),
                loadShader(path+shader_name+std::string(".frag")).c_str()
                );
        um_assert( operator[](shader_name) );
    }
};

