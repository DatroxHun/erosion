#include "computeClass.h"
#include "shaderClass.h"

ComputeShader::ComputeShader(const char* computeFile)
{
	std::string computeCode = get_file_contents(computeFile);

	const char* computeSource = computeCode.c_str();

	// create and compile shaders then attach them to shader program
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER); // create compute shader on GPU
	glShaderSource(computeShader, 1, &computeSource, NULL); // set compile destination and source (2nd arg -> string count to compile)
	glCompileShader(computeShader); // compile shader

	// handle compilation errors
	int success;
	char infoLog[512];
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// create shader program and attach shaders to it
	ID = glCreateProgram();
	glAttachShader(ID, computeShader);

	// link shaders one after the other in shader program and delete them (free memory)
	glLinkProgram(ID);
	glDeleteShader(computeShader);
}

void ComputeShader::Activate()
{
	glUseProgram(ID);
}

void ComputeShader::Delete()
{
	glDeleteProgram(ID);
}