#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>

enum TypeShader
{
    VERTEX_SHADER = GL_VERTEX_SHADER,
    FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    GEOMETRY_SHADER = GL_GEOMETRY_SHADER
};

class Shader
{
public:

    Shader() : m_vertexShader(0), m_fragmentShader(0),
        m_geometryShader(0), m_isVertexShader(false),
        m_isFragmentShader(false), m_isGeometryShader(false)
    { }

    ~Shader() {}

    void loadShader(const char *shader, TypeShader type)
    {
        if(type == TypeShader::VERTEX_SHADER)
        {
           // std::string vertexshader = getShaderReader(shader);
           // const char *vertex_shader = vertexshader.c_str();

            m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(m_vertexShader, 1, &shader, NULL);
            glCompileShader(m_vertexShader);
            shaderCompileStatus(m_vertexShader);
            m_isVertexShader = true;
        }

        if(type == TypeShader::FRAGMENT_SHADER)
        {
           // std::string fragmentshader = getShaderReader(shader);
           // const char *fragment_shader = fragmentshader.c_str();
            m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(m_fragmentShader, 1, &shader, NULL);
            glCompileShader(m_fragmentShader);
            shaderCompileStatus(m_fragmentShader);
            m_isFragmentShader = true;
        }

        if(type == TypeShader::GEOMETRY_SHADER)
        {
            std::string geometryshader = getShaderReader(shader);
            const char *geometry_shader = geometryshader.c_str();
            m_geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(m_geometryShader, 1, &geometry_shader, NULL);
            glCompileShader(m_geometryShader);
            m_isGeometryShader = true;
        }
    }

    void useShaderProgram() const
    {
        glUseProgram(m_id);
    }

    GLuint getShaderProgram() const
    {
        return m_id;
    }

    void createShaderProgram()
    {
        m_id = glCreateProgram();

        if(m_isVertexShader)
        {
            glAttachShader(m_id, m_vertexShader);
        }
        if(m_isFragmentShader)
        {
            glAttachShader(m_id, m_fragmentShader);
        }
        if(m_isGeometryShader)
        {
            glAttachShader(m_id, m_geometryShader);
        }

        glLinkProgram(m_id);
        programCompileStatus(m_id);
    }

    std::string getShaderReader(const std::string &shader)
    {
        std::ifstream file(shader);
        std::stringstream buffer;

        if(file.is_open())
        {
            buffer << file.rdbuf();
        }
        else
        {
            std::cerr << "Cannot open file! " << ""<< shader << ""<< std::endl;
        }

        std::string contents(buffer.str());

        return contents;
    }

    void setUniformMatrix4x4(const std::string &type, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, type.c_str()), 1, GL_FALSE, &matrix[0][0]);
    }

    void setUniformInt(const std::string &type, const GLint value)
    {
        glUniform1i(glGetUniformLocation(m_id, type.c_str()), value);
    }

    void setUniformFloat(const std::string &type, const GLfloat value)
    {
        glUniform1f(glGetUniformLocation(m_id, type.c_str()), value);
    }

    void setUnifromVec2(const std::string &type, const glm::vec3 &value)
    {
        glUniform2f(glGetUniformLocation(m_id, type.c_str()), value.x, value.y);
    }

    void setUnifromVec2(const std::string &type, const float &x, const float &y)
    {
        glUniform2f(glGetUniformLocation(m_id, type.c_str()), x, y);
    }

    void setUnifromVec3(const std::string &type, const glm::vec3 value)
    {
        glUniform3fv(glGetUniformLocation(m_id, type.c_str()), 1, &value[0]);
    }

    void setUnifromVec3(const std::string &type, const float &x, const float &y, const float &z)
    {
        glUniform3f(glGetUniformLocation(m_id, type.c_str()), x, y, z);
    }

protected:

    void shaderCompileStatus(GLuint shader)
    {
        GLint isCompiled;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if(!isCompiled)
        {
            int logLenght;
            GLchar log[1024];
            glGetShaderInfoLog(shader, 1024, &logLenght, log);
            std::cerr << "[WARN] Shader compilation error : "  << log <<
                " - Log lenght: " << logLenght <<
                "\n";
        }
        else
        {
            int logLenght;
            GLchar log[1024];
            glGetShaderInfoLog(shader, 1024, &logLenght, log);
            std::cerr << "[INFO] Shader compilation success ! " << log <<
                " - Log lenght: " << logLenght <<
                "\n";
        }
    }

    void programCompileStatus(GLuint program)
    {
        GLint isCompiled;

        glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);
        if(!isCompiled)
        {
            int logLenght;
            GLchar log[1024];
            glGetProgramInfoLog(program, 1024, &logLenght, log);
            std::cerr << "[WARN] Program linker error : "  << log <<
                " - Log lenght: " << logLenght <<
                "\n";
        }
        else
        {
            int logLenght;
            GLchar log[1024];
            glGetProgramInfoLog(program, 1024, &logLenght, log);
            std::cerr << "[INFO] Pogram linker success ! " << log <<
                " - Log lenght: " << logLenght <<
                "\n";
        }
    }

private:

    // Shader program id
    GLuint m_id;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_geometryShader;

    bool m_isVertexShader;
    bool m_isFragmentShader;
    bool m_isGeometryShader;
};
