#ifndef COMPUTE_SHADER_CLASS_H
#define COMPUTE_SHADER_CLASS_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string get_file_contents(const char* filename);

class ComputeShader
{
public:
	GLuint ID;
	ComputeShader(const char* computeFile);

	void Activate();
	void Delete();
};

#endif