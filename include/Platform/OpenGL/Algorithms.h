#pragma once
#include <vector>
#include "Vertice.h"
#include <glad/glad.h>

class Algorithms final
{

public:
	static bool triangulatePolygon(std::vector<Vertice> vertices, std::vector<Vertice> hole, std::vector<Vertice>& triangles, std::vector<GLuint>& indices);
	static double distSq(Vertice v1, Vertice v2);
	static bool checkRightAngled(Vertice v1, Vertice v2, Vertice v3);
private:

	static bool triangulatePolygon(int step, std::vector<Vertice> vertices, int corners, std::vector<Vertice> hole, std::vector<Vertice>& triangles);
	// No touchey
	virtual void nonConstructible() = 0;

	static bool pointInsideTriangle(Vertice A, Vertice B, Vertice C, Vertice point);
	static bool lineSegmentsIntersect(Vertice A, Vertice B, Vertice C, Vertice D);
};