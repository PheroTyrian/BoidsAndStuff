#include "Shader.h"
#include "Renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(const std::string & filepath) : m_filepath(filepath), m_rendererID(0)
{
	ShaderProgramSource source = parseShader(filepath);
	m_rendererID = createShader(source.vertexSource, source.fragmentSource);
}


Shader::~Shader()
{
	GLCall(glDeleteProgram(m_rendererID));
}

void Shader::bind() const
{
	GLCall(glUseProgram(m_rendererID));
}

void Shader::unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::setUniform1i(const std::string & name, int val)
{
	GLCall(glUniform1i(getUniformLocation(name), val));
}

void Shader::setUniform1f(const std::string & name, float val)
{
	GLCall(glUniform1f(getUniformLocation(name), val));
}

void Shader::setUniform4f(const std::string & name, float a, float b, float c, float d)
{
	GLCall(glUniform4f(getUniformLocation(name), a, b, c, d));
}

void Shader::setUniform4f(const std::string & name, float * data)
{
	GLCall(glUniform4f(getUniformLocation(name), data[0], data[1], data[2], data[3]));
}

void Shader::setUniformMat4f(const std::string & name, const glm::mat4 & matrix)
{
	GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

int Shader::getUniformLocation(const std::string & name)
{
	if (m_uniforms.find(name) != m_uniforms.end())
		return m_uniforms[name];

	GLCall(int location = glGetUniformLocation(m_rendererID, name.c_str()));

	if (location == -1)
		std::cout << "Warning: uniform " << name << " not found" << std::endl;

	m_uniforms[name] = location;
	return location;
}

ShaderProgramSource Shader::parseShader(const std::string& path)
{
	std::ifstream stream(path, std::ifstream::in);
	enum class shaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	std::string line;
	std::stringstream ss[2];
	shaderType type = shaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = shaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = shaderType::FRAGMENT;
		}
		else if (type != shaderType::NONE)
		{
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source)
{
	GLCall(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Shader compile error:" << std::endl << message << std::endl;
		GLCall(glDeleteShader(id));
		return 0;
	}
	return id;
}

unsigned int Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	GLCall(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	//Delete shader intermediaries
	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}
