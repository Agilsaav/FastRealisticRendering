
#include "Frustum.h"
#include <cmath>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <iostream>
#include <QVector3D>
#ifndef __APPLE__
  #include <GL/gl.h>
#else
  #include <gl.h>
#endif
using namespace std;

void Frustum::NormalizePlane(Plane & plane)
{
	float   mag;    
	mag = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);    

	plane.a = plane.a / mag;    
	plane.b = plane.b / mag;    
	plane.c = plane.c / mag;    
	plane.d = plane.d / mag;
}

float Frustum::DistanceTovec3(const Plane & plane, QVector3D & p)
{
	return plane.a*p.x() + plane.b*p.y() + plane.c*p.z() + plane.d;
}

Halfspace Frustum::Classifyvec3(const Plane & plane, QVector3D & p)
{
	float d;
    d = plane.a*p.x() + plane.b*p.y() + plane.c*p.z() + plane.d;
    if (d < 0) return NEGATIVE;    
	if (d > 0) return POSITIVE;
	return ON_PLANE;
}

void Frustum::ExtractPlanesGL(Plane * p_planes,const QMatrix4x4 & comboMatrix,bool normalize)
{    
	// Left clipping plane
    p_planes[0].a = comboMatrix(3,0) + comboMatrix(0,0);    
	p_planes[0].b = comboMatrix(3,1) + comboMatrix(0,1);    
	p_planes[0].c = comboMatrix(3,2) + comboMatrix(0,2);    
	p_planes[0].d = comboMatrix(3,3) + comboMatrix(0,3);    
	// Right clipping plane     
	p_planes[1].a = comboMatrix(3,0) - comboMatrix(0,0);    
	p_planes[1].b = comboMatrix(3,1) - comboMatrix(0,1);    
	p_planes[1].c = comboMatrix(3,2) - comboMatrix(0,2);    
	p_planes[1].d = comboMatrix(3,3) - comboMatrix(0,3);    
	// Top clipping plane    
	p_planes[2].a = comboMatrix(3,0) - comboMatrix(1,0);    
	p_planes[2].b = comboMatrix(3,1) - comboMatrix(1,1);    
	p_planes[2].c = comboMatrix(3,2) - comboMatrix(1,2);    
	p_planes[2].d = comboMatrix(3,3) - comboMatrix(1,3);    
	// Bottom clipping plane    
	p_planes[3].a = comboMatrix(3,0) + comboMatrix(1,0);    
	p_planes[3].b = comboMatrix(3,1) + comboMatrix(1,1);    
	p_planes[3].c = comboMatrix(3,2) + comboMatrix(1,2);    
	p_planes[3].d = comboMatrix(3,3) + comboMatrix(1,3);    
	// Near clipping plane    
	p_planes[4].a = comboMatrix(3,0) + comboMatrix(2,0);    
	p_planes[4].b = comboMatrix(3,1) + comboMatrix(2,1);    
	p_planes[4].c = comboMatrix(3,2) + comboMatrix(2,2);    
	p_planes[4].d = comboMatrix(3,3) + comboMatrix(2,3);    
	// Far clipping plane    
	p_planes[5].a = comboMatrix(3,0) - comboMatrix(2,0);    
	p_planes[5].b = comboMatrix(3,1) - comboMatrix(2,1);    
	p_planes[5].c = comboMatrix(3,2) - comboMatrix(2,2);    
	p_planes[5].d = comboMatrix(3,3) - comboMatrix(2,3);    
	// Normalize the plane equations, if requested    
	if (normalize == true)    
	{
		NormalizePlane(p_planes[0]);
		NormalizePlane(p_planes[1]);
		NormalizePlane(p_planes[2]);
		NormalizePlane(p_planes[3]);
		NormalizePlane(p_planes[4]);
		NormalizePlane(p_planes[5]);    
	}
}

bool Frustum::IntersectionBB(Plane * planes, const QVector3D & max, const QVector3D & min)
{
	
	points[0] = QVector3D(max.x(),max.y(),max.z());
	points[1] = QVector3D(max.x(),min.y(),max.z());
	points[2] = QVector3D(max.x(),max.y(),min.z());
	points[3] = QVector3D(max.x(),min.y(),min.z());
	points[4] = QVector3D(min.x(),min.y(),min.z());
	points[5] = QVector3D(min.x(),max.y(),min.z());
	points[6] = QVector3D(min.x(),min.y(),max.z());
	points[7] = QVector3D(min.x(),max.y(),max.z());

	int in =0;

	//If one point is inside all the planes(d>0 for all) return true else the bb is outside the frustum
	for (int j=0; j<8; j++)
	{
		in = 0;
		for (int i=0; i<6; i++)
		{
			if(DistanceTovec3(planes[i], points[j])>0) in +=1;
		}
		
		if(in==6) return true;
	}

	return false;
}


bool Frustum::IntersectionBBCenter(Plane * planes, const QVector3D & max, const QVector3D & min)
{
	QVector3D centerBB(min.x() + (max.x() - min.x())/2.0f, min.y() + (max.y() - min.y())/2.0f, min.z() + (max.z() - min.z())/2.0f);
	float maxdistance = sqrt(pow(max.x()-centerBB.x(), 2) + pow(max.y()-centerBB.y(), 2) + pow(max.z()-centerBB.z(), 2));
	float mindistance = sqrt(pow(min.x()-centerBB.x(), 2) + pow(min.y()-centerBB.y(), 2) + pow(min.z()-centerBB.z(), 2));

	int in = 0;

	for (int j=0; j<6; j++)
	{
		if(DistanceTovec3(planes[j], centerBB)>0 or abs(DistanceTovec3(planes[j], centerBB))<maxdistance) in +=1;

	}

	if(in==6) return true;

	else return false;
}


