#pragma once
#include "glm.hpp"

/* guidelines that would be nice for this struct:
* No private members
* No elements that aren't meant to be assigned directly into the vertex array object
* No templates
*/ 


struct Vertice
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoord;
	//glm::vec3 normal;
};