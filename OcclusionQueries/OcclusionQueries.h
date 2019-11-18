// GLarena, a plugin based platform to teach OpenGL programming
// © Copyright 2012-2018, ViRVIG Research Group, UPC, https://www.virvig.eu
// 
// This file is part of GLarena
//
// GLarena is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _OCCLUSIONQUERIES_H
#define _OCCLUSIONQUERIES_H

#include "plugin.h"
#include "qpainter.h"
#include "Frustum.h"


#include <QOpenGLExtraFunctions>
#include <vector>

 class OcclusionQueries : public QObject, public Plugin
 {
     Q_OBJECT
     Q_PLUGIN_METADATA(IID "Plugin")   
     Q_INTERFACES(Plugin)

	int fcnt;
  	int fps;



public:
	void onPluginLoad();
	void postFrame();
	bool paintGL();
	void onObjectAdd();
	bool drawScene();


	void onSceneClear();
	bool drawObject(int i);
	void keyPressEvent(QKeyEvent *e);

public slots:
	void updateFPS();

private:
	QPainter painter;

	QOpenGLShaderProgram* program;
    QOpenGLShader* vs;
    QOpenGLShader* fs;

	bool OQ;
	bool FC;

	Plane * plane = (Plane*) malloc (sizeof(Plane)*6); //Frustum planes
	Frustum frustum;

    vector<GLuint> VAOs;          // ID of VAOs
    vector<GLuint> coordBuffers;  // ID of vertex coordinates buffer 
    vector<GLuint> normalBuffers; // ID of normal components buffer 
    vector<GLuint> stBuffers;     // ID of (s,t) buffer 
    vector<GLuint> colorBuffers;  // ID of color buffer  
    vector<GLuint> numIndices;    // Size (number of indices) in each index buffer

	GLuint BBVAO, BBVB; //Bounding Box vbo and vao;
	
	void LoadShaders();
	void cleanUp();
	void addVBO(unsigned int currentObject);
	void createBoundingBox(); //Create the Bounding Box
	void DrawBoundingBox();


};

 
#endif

