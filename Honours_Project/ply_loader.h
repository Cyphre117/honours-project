#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

class PlyLoader
{
public:
	PlyLoader();
	~PlyLoader();

	void load( std::string filepath, std::vector<GLfloat>& data );

	enum class Format { None, ASCII, Binary };
	enum class PropertyIdent { discard, x, y, z, r, g, b };

	struct VertexProperty {
		PropertyIdent type;
		unsigned char size;
	};

	std::string toString( PropertyIdent p );

protected:
};