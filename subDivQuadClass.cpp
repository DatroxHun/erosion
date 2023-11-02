#include "subDivQuadClass.h"

Quad::Quad(int resolution)
{
	this->resolution = resolution;

	shader = new Shader("quad.vert", "quad.frag");

	get_vertices();
	get_indices();

	vao = new VAO();
	vao->Bind();
	vbo = new VBO(vertices, 3 * (resolution + 1) * (resolution + 1) * sizeof(GLfloat));
	ebo = new EBO(indices, 6 * resolution * resolution * sizeof(GLuint));
	vao->LinkVBO(*vbo, 0);
	vao->Unbind();
	vbo->Unbind();
	ebo->Unbind();

	resolutionLocation = glGetUniformLocation(shader->ID, "resolution");
	camMatLocation = glGetUniformLocation(shader->ID, "cam_mat");
}

void Quad::get_vertices()
{
	vertices = (GLfloat*)malloc(3 * (resolution + 1) * (resolution + 1) * sizeof(GLfloat));

	int vert_res = resolution + 1;

	for (int i = 0; i < vert_res; i++)
	{
		for (int j = 0; j < vert_res; j++)
		{
			vertices[3 * (i + j * vert_res) + 0] = -1.0 + 2.0 * i / (GLfloat)resolution; // x
			vertices[3 * (i + j * vert_res) + 1] = 0.0/*(i + j) / (GLfloat)(4.0 * resolution)*/; // y
			vertices[3 * (i + j * vert_res) + 2] = -1.0 + 2.0 * j / (GLfloat)resolution; // z
		}
	}
}

void Quad::get_indices()
{
	indices = (GLuint*)malloc(6 * resolution * resolution * sizeof(GLuint));

	int vert_res = resolution + 1;

	for (GLuint i = 0; i < resolution; i++)
	{
		for (GLuint j = 0; j < resolution; j++)
		{
			indices[6 * (i + j * resolution) + 0] = (i + 0) + (j + 0) * vert_res;
			indices[6 * (i + j * resolution) + 1] = (i + 1) + (j + 0) * vert_res;
			indices[6 * (i + j * resolution) + 2] = (i + 1) + (j + 1) * vert_res;
			indices[6 * (i + j * resolution) + 3] = (i + 0) + (j + 0) * vert_res;
			indices[6 * (i + j * resolution) + 4] = (i + 1) + (j + 1) * vert_res;
			indices[6 * (i + j * resolution) + 5] = (i + 0) + (j + 1) * vert_res;
		}
	}
}

void Quad::render(Camera cam)
{
	// set our shaderProgram the current Program
	shader->Activate();
	vao->Bind();

	glUniform1i(resolutionLocation, resolution);
	glUniformMatrix4fv(camMatLocation, 1, GL_FALSE, value_ptr(cam.get_matrix()));

	// object type, indices count, indices data type, index of indices
	glDrawElements(GL_TRIANGLES, 6 * resolution * resolution, GL_UNSIGNED_INT, 0);
}

Quad::~Quad()
{
	free(vertices);
	vertices = NULL;
	free(indices);
	indices = NULL;

	vao->Delete();
	vbo->Delete();
	ebo->Delete();
	shader->Delete();
}