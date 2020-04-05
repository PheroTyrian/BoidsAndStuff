#include "Renderer.h"

//VERTEX BUFFER ---------------------------------------------------

VertexBuffer::VertexBuffer(const void * data, unsigned int size)
{
	GLCall(glGenBuffers(1, &m_rendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_rendererID));
}

void VertexBuffer::bind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
}

void VertexBuffer::unbind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

//INDEX BUFFER ---------------------------------------------------

IndexBuffer::IndexBuffer(const unsigned int * data, unsigned int count) : m_count(count)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	GLCall(glGenBuffers(1, &m_rendererID));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_rendererID));
}

void IndexBuffer::bind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
}

void IndexBuffer::unbind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

//RENDERER ---------------------------------------------------

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}
