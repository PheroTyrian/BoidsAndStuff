#pragma once

#include "Renderer.h"

class Texture
{
private:
	unsigned int m_rendererID;
	std::string m_filepath;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_bitsPerPixel;
public:
	Texture(const std::string& filepath);
	~Texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }
	inline int getBPP() const { return m_bitsPerPixel; }
};

