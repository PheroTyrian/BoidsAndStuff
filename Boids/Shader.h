#pragma once
#include "Renderer.h"
#include <string>
#include <unordered_map>

struct ShaderProgramSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

class Shader
{
private:
	unsigned int m_rendererID;
	std::string m_filepath;
	std::unordered_map<std::string, int> m_uniforms;
public:
	Shader(const std::string& filepath);
	~Shader();

	void bind() const;
	void unbind() const;

	void setUniform1i(const std::string& name, int val);
	void setUniform1f(const std::string& name, float val);
	void setUniform4f(const std::string& name, float a, float b, float c, float d);
	void setUniform4f(const std::string& name, float* data);
private:
	unsigned int getUniformLocation(const std::string& name);
	ShaderProgramSource parseShader(const std::string& path);
	unsigned int compileShader(unsigned int type, const std::string& source);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);
};

