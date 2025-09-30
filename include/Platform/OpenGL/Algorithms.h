#pragma once
#include <vector>
#include "Vertice.h"
#include <glad/glad.h>

class Algorithms final
{

public:
	static bool triangulatePolygon(std::vector<Vertice>& vertices, std::vector<Vertice>& hole, std::vector<Vertice>& result, std::vector<GLuint>& indices, int resultAsTriangles);
	static double distSq(Vertice& v1, Vertice& v2);
	static bool isRightAngled(Vertice& v1, Vertice& v2, Vertice& v3);
	static bool pointInsideTriangle(Vertice& A, Vertice& B, Vertice& C, Vertice& point);
	static bool lineSegmentsIntersect(Vertice A, Vertice B, Vertice C, Vertice D);
private:
	static bool combineHole(std::vector<Vertice>& vertices, std::vector<Vertice>& hole, std::vector<std::pair<GLuint, Vertice>>& result);
	static bool triangulatePolygon(std::vector<std::pair<GLuint, Vertice>>& vertices, int corners, std::vector<Vertice>& triangles, std::vector<GLuint>& indices, int resultAsTriangles);
	// No touchey
	virtual void nonConstructible() = 0;
};