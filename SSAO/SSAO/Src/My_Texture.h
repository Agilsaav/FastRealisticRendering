#pragma once

#include "Renderer.h"

class My_Texture
{
private:
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_With, m_Height, m_BPP;

public:
	My_Texture(const std::string& path);
	~My_Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_With; }
	inline int GetHeighth() const { return m_Height; }
};
