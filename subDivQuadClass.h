#pragma once

#include <cstdlib>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "camera.h"

class Quad
{
public:
	int get_resolution() const { return resolution; }

private:
	int resolution;
	GLfloat *vertices;
	GLuint *indices;

	Shader *shader;

	VAO *vao;
	VBO *vbo;
	EBO *ebo;

	GLint resolutionLocation;
	GLint camMatLocation;

public:
	Quad(int resolution);
	~Quad();

	void render(Camera cam);

private:
	void get_vertices();
	void get_indices();
};