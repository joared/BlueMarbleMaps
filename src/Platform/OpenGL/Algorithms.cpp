#include "Platform/OpenGL/Algorithms.h"
#include "glm.hpp"
#include <iostream>

bool Algorithms::triangulatePolygon(std::vector<Vertice>& vertices, std::vector<Vertice>& hole, std::vector<Vertice>& result, std::vector<GLuint>& indices, int resultAsTriangles)
{
	std::vector<Vertice> tmpVerts = vertices;
	std::vector<Vertice> tmpHoles = hole;
	std::vector<std::pair<GLuint, Vertice>> vertIndexPair;
	result.reserve(vertices.size());
	indices.reserve(vertices.size());
	vertIndexPair.reserve(vertices.size() + hole.size());
	
	if (tmpHoles.size())
	{
		combineHole(tmpVerts, tmpHoles, vertIndexPair);
	}
	else
	{
		for (int i = 0; i < tmpVerts.size(); i++)
		{
			vertIndexPair.push_back(std::pair(i, tmpVerts[i]));
		}
	}
	
	if (!resultAsTriangles)
	{
		for (int i = 0; i < vertIndexPair.size(); i++)
		{
			result.push_back(vertIndexPair[i].second);
		}
	}
	return triangulatePolygon(vertIndexPair, vertIndexPair.size(), result, indices, resultAsTriangles);
}

bool Algorithms::combineHole(std::vector<Vertice>& vertices, std::vector<Vertice>& hole, std::vector<std::pair<GLuint, Vertice>>& result)
{
	bool success = true;
	int i = 0, j = 0, k = 0, l = 0;

	//  If there is no hole, skip combination.
	if (hole.size() == 0) {
		return false;
	}

	//  Find the shortest connector between the outer polygon and the hole
	//  polygon (that does not intersect any side of either polygon).
	int minI = -1, hMinI, dX, dY, dist2, minDist2 = 999999999;
	for (i = 0; i < vertices.size(); i++) for (j = 0; j < hole.size(); j++) {
		dX = vertices[i].position.x - hole[j].position.x;
		dY = vertices[i].position.y - hole[j].position.y;
		dist2 = dX * dX + dY * dY;
		if (dist2 < minDist2) {   //  Shorter segment found.
			//  Ensure that the segment under consideration does not intersect any side of the outer polygon.
			for (k = 0; k < vertices.size(); k++) {
				l = (k + 1) % vertices.size();
				if (lineSegmentsIntersect(vertices[i], hole[j], vertices[k], vertices[l])) k = vertices.size();
			}
			if (k == vertices.size()) {
				//  Ensure that the segment under consideration does not intersect any side of the hole polygon.
				for (k = 0; k < hole.size(); k++) {
					l = (k + 1) % hole.size();
					if (lineSegmentsIntersect(vertices[i], hole[j], hole[k], hole[k])) k = hole.size();
				}
				if (k == hole.size()) {
					//  All good; the new line segment becomes the current best choice.
					minDist2 = dist2; minI = i; hMinI = j;
				}
			}
		}
	}
	if (minI < 0) {
		std::cout << "Failed to find a candidate for linking hole\n";
		return false;
	}

	//  Using the connector, create a unified polygon from the two polygons.
	for (i = 0; i <= vertices.size(); i++) {
		result.push_back(std::make_pair(i,vertices[(minI + i) % vertices.size()]));
	}
	for (i = 0; i <= hole.size(); i++) {
		result.push_back(std::make_pair(vertices.size() + 1 + i,hole[(hMinI + hole.size() - i) % hole.size()]));
	}
}

bool Algorithms::triangulatePolygon(std::vector<std::pair<GLuint, Vertice>>& vertices, int corners, std::vector<Vertice>& triangles,std::vector<GLuint>& indices, int resultAsTriangles)
{
	bool success = true;
	int i = 0, j = 0, k = 0, l = 0;

	//  Crawl around the edge of the polygon, looking for three consecutive corners that form a usable triangle.
	i = 0;
	while (i < corners && corners >= 3) {
		j = (i + 1) % corners;
		k = (j + 1) % corners;
		//  Verify that the candidate triangle is inside, not outside the
		if (((vertices[i].second.position.x != vertices[j].second.position.x || vertices[i].second.position.y != vertices[j].second.position.y) &&
			(vertices[k].second.position.y - vertices[i].second.position.y) * (vertices[j].second.position.x - vertices[i].second.position.x) >=
			(vertices[k].second.position.x - vertices[i].second.position.x) * (vertices[j].second.position.y - vertices[i].second.position.y))||
			isRightAngled(vertices[i].second, vertices[j].second, vertices[k].second)) {
			//  Verify that the candidate triangle's interior does not contain any corners of the polygon.
			for (l = 0; l < corners; l++) {
				if (pointInsideTriangle(vertices[i].second, vertices[j].second, vertices[k].second, vertices[l].second)) l = corners;
			}
			if (l == corners) {   //  A usable triangle has been found.
				if (resultAsTriangles)
				{
					triangles.insert(triangles.end(), { vertices[i].second, vertices[j].second, vertices[k].second });
				}
				indices.insert(indices.end(), {vertices[i].first ,vertices[j].first,vertices[k].first});
	
				//  Reduce the remaining polygon to exclude the just-drawn triangle.
				corners--;
				for (l = j; l < corners; l++) {
					vertices[l] = vertices[l + 1];
				}
				return triangulatePolygon(vertices, corners, triangles,indices, resultAsTriangles);
			}
		}
		//  The three corners (i,j,k) did not make a usable triangle. Move forward around the polygon to keep looking.
		i++;
	}
	//  The process has ended; alert the user if it didn't successfully triangulate the entire polygon.
	if (corners >= 3) {
		std::cout << "Failed to find a suitable triangle\n";
		return false;
	}
	
	return success;
}

bool Algorithms::pointInsideTriangle(Vertice& A, Vertice& B, Vertice& C, Vertice& point)
{
	bool inside = false;

	if ((point.position.x != A.position.x || point.position.y != A.position.y)
		&& (point.position.x != B.position.x || point.position.y != B.position.y)
		&& (point.position.x != C.position.x || point.position.y != C.position.y)) {
		if ((A.position.y < point.position.y && B.position.y >= point.position.y || B.position.y < point.position.y && A.position.y >= point.position.y) && A.position.x + (point.position.y - A.position.y) / (B.position.y - A.position.y) * (B.position.x - A.position.x) < point.position.x)     inside = !inside;
		if ((B.position.y < point.position.y && C.position.y >= point.position.y || C.position.y < point.position.y && B.position.y >= point.position.y) && B.position.x + (point.position.y - B.position.y) / (C.position.y - B.position.y) * (C.position.x - B.position.x) < point.position.x)     inside = !inside;
		if ((C.position.y < point.position.y && A.position.y >= point.position.y || A.position.y < point.position.y && C.position.y >= point.position.y) && C.position.x + (point.position.y - C.position.y) / (A.position.y - C.position.y) * (point.position.x - C.position.x) < point.position.x) inside = !inside;
	}

	return inside;
}

bool Algorithms::lineSegmentsIntersect(Vertice A, Vertice B, Vertice C, Vertice D)
{
	//  No intersection if any of the four points are identical.
	if (A.position.x == B.position.x && A.position.y == B.position.y || A.position.x == C.position.x && A.position.y == C.position.y || A.position.x == D.position.x && A.position.y == D.position.y
		|| B.position.x == C.position.x && B.position.y == C.position.y || B.position.x == D.position.x && B.position.y == D.position.y || C.position.x == D.position.x && C.position.y == D.position.y) {
		return false;
	}

	//  Translate the system so that point A is on the origin.
	B.position.x -= A.position.x; B.position.y -= A.position.y; C.position.x -= A.position.x; C.position.y -= A.position.y; D.position.x -= A.position.x; D.position.y -= A.position.y;

	//  Discover the length of segment AB.
	float distAB = glm::sqrt(B.position.x * B.position.x + B.position.y * B.position.y);

	//  Rotate the system so that point B is on the positive X axis.
	float cos = B.position.x / distAB, sin = B.position.y / distAB,
		newX = C.position.x * cos + C.position.y * sin;
	C.position.y = C.position.y * cos - C.position.x * sin; C.position.x = newX;
	newX = D.position.x * cos + D.position.y * sin;
	D.position.y = D.position.y * cos - D.position.x * sin; D.position.x = newX;

	//  No intersection if segment CD doesn't cross line AB.
	if (C.position.y < 0. && D.position.y < 0. || C.position.y >= 0. && D.position.y >= 0.) return false;

	//  Discover the position of the intersection point along line AB.
	float ABpos = D.position.x + (C.position.x - D.position.x) * D.position.y / (D.position.y - C.position.y);

	//  Intersection is found only if segment CD crosses line AB inside of segment AB.
	return ABpos > 0 && ABpos < distAB;
}

double Algorithms::distSq(Vertice& v1, Vertice& v2) {
	return std::pow((v2.position.x - v1.position.x), 2) + std::pow((v2.position.y - v1.position.y), 2);
}

bool Algorithms::isRightAngled(Vertice& v1, Vertice& v2, Vertice& v3) {
	// Calculate the squares of the lengths of the sides
	double s1 = distSq(v1, v2);
	double s2 = distSq(v2, v3);
	double s3 = distSq(v1, v3);

	// Check if all sides have a positive length (to avoid degenerate triangles)
	if (s1 <= 0 || s2 <= 0 || s3 <= 0) {
		return false;
	}

	// Check the Pythagorean theorem
	// Use a small tolerance (epsilon) for floating-point comparisons
	double epsilon = 1e-9; // A small value for floating point precision

	if (std::abs(s1 + s2 - s3) < epsilon ||
		std::abs(s1 + s3 - s2) < epsilon ||
		std::abs(s2 + s3 - s1) < epsilon) {
		return true;
	}
	else {
		return true;
	}
}
