#ifndef FRUSTRUM_H
#define FRUSTRUM_H

#define _USE_MATH_DEFINES 1
//#include "GL/glew.h"
#include <QMatrix4x4>
#include <vector>
#include <QVector3D>

/*!
	\class Frustum:

		Functions: 

		void NormalizePlane(Plane & plane) -> Normalize a Plane
		float DistanceTovec3(const Plane & plane, QVector3D & p) -> Returns the distance between a plane and a point(QVector3D)
		Halfspace Classifyvec3(const Plane & plane, QVector3D & p) -> Classify where lies a point respect a plane.
		void ExtractPlanesGL(Plane * p_planes,const QMatrix4x4 & comboMatrix,bool normalize) -> Extract the 6 planes using the MVP matrix
		bool IntersectionBB(Plane * planes, const QVector3D & max, const QVector3D & min) -> Return true if one vertice is inside of the Frustum
		bool IntersectionBBCenter(Plane * planes, const QVector3D & max, const QVector3D & min) -> Return true if the center of the BoundingBox is inside Frustrum or the distance
																								   between the center and the plane is less than the maximum diagonal distance.
*/

struct Plane
{
	float   a, b, c, d;
};

enum Halfspace
{
	NEGATIVE = -1,
	ON_PLANE = 0,
	POSITIVE = 1,
};


class Frustum
{
public:
	void NormalizePlane(Plane & plane);
	float DistanceTovec3(const Plane & plane, QVector3D & p);
	Halfspace Classifyvec3(const Plane & plane, QVector3D & p);
	void ExtractPlanesGL(Plane * p_planes,const QMatrix4x4 & comboMatrix,bool normalize);
	bool IntersectionBB(Plane * planes, const QVector3D & max, const QVector3D & min);
	bool IntersectionBBCenter(Plane * planes, const QVector3D & max, const QVector3D & min);
private:
	QVector3D * points = (QVector3D*) malloc (sizeof(QVector3D)*8);
};
#endif
