#include "VAO.h"

VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO vbo, GLuint layout)
{
	vbo.Bind();

	// (id, # of components per vertex attrib, type of array component, should get normalized?, byte offset between vertex attrib, offset of first component in array)
	glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	// enable specific vertex attribute by id
	glEnableVertexAttribArray(0);

	vbo.Unbind();
}

void VAO::Bind()
{
	glBindVertexArray(ID);
}

void VAO::Unbind()
{
	glBindVertexArray(0);
}

void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}