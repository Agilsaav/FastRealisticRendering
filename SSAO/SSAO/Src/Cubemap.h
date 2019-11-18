#pragma once

#pragma once

#include "Renderer.h"

#include <vector>
#include <iostream>

class Cubemap
{
private:
	unsigned int m_RendererID;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

public:
	Cubemap(const std::vector<std::string> faces);
	~Cubemap();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeighth() const { return m_Height; }
};