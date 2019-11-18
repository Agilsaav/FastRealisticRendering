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

#include "OcclusionQueries.h"
#include "glwidget.h"

#include <QMenu>
#include <QKeyEvent> 
#include <QWidget>
#include <cassert>
#include <cmath>

//N = NumberOfCopies_x * NumberOfCopies_z -> Can be changed to change the number of objects(N).
unsigned int NumberOfCopies_x = 10;
unsigned int NumberOfCopies_z = 10;

void OcclusionQueries::onSceneClear()
{
    cleanUp();
}

bool OcclusionQueries::paintGL()
{
	camera()->setZfar(100.0);
	

	if (drawPlugin()) 
		drawPlugin()->drawScene();

	glwidget()->drawAxes();

    return true;
}


void OcclusionQueries::updateFPS() 
{
	fps=fcnt;                 // fps var value
	fcnt=0;                   // reset fps counter
	glwidget()->update();   // update window
}

void OcclusionQueries::onPluginLoad() 
{
    // update frame count every second
	fps=fcnt=0;
	QTimer *frameTimer=new QTimer(this);
	connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFPS()));
	frameTimer->start(1000);
	// keep repainting
	QTimer *paintTimer=new QTimer(this);
	connect(paintTimer, SIGNAL(timeout()), glwidget(), SLOT(update()));
	paintTimer->start();

	//Create shaders
	LoadShaders();
	
	//Add VBO 
    addVBO(0);

	//Create BoundingBox Object
	createBoundingBox();
	OQ = false;

	//Frustrum Culling
	FC = false;
}

void OcclusionQueries::onObjectAdd()
{
	addVBO( scene()->objects().size() - 1 );
}

bool OcclusionQueries::drawScene()
{
    GLWidget &g = *glwidget();
    g.makeCurrent();

	QMatrix4x4 MVP=camera()->projectionMatrix()*camera()->viewMatrix();
	float distx = 2*scene()->objects()[0].boundingBox().max()[0];
	float distz = 2*scene()->objects()[0].boundingBox().max()[2]; 

	/*--FRUSTUM CULLING--------------------------------------------------------------
	
	In this part we render using the Frustum Culling technique. 
	In order to do this we start computing the 6 planes of the frutrum. In the render 
	loop we only draw the object when the center of the bounding box is inside all the 
	planes, or the distance between the center and one plane is less than the diagonal 
	maximum distance of the bounding box.

	It uses the Frustum class, if it is not in the same directory it will not work.

	Press x to change to Frustum Culling Mode.

	*/

	if(FC)
	{
		bool draw = false;
		int objects_draw = 0;

		MVP=camera()->projectionMatrix()*camera()->viewMatrix();

		//Compute the frustum planes
		frustum.ExtractPlanesGL(plane,MVP,true);

		for(unsigned int j=0; j<NumberOfCopies_x; j++)
		{
			MVP=camera()->projectionMatrix()*camera()->viewMatrix();
			MVP.translate(distx*j,0.0,0.0);
			QVector3D move(distx*j,0.0,0.0);
			for(unsigned int k=0; k<NumberOfCopies_z; k++)
			{
				//Draw give us if we need to draw or not the object
				draw = frustum.IntersectionBBCenter(plane, scene()->objects()[0].boundingBox().max()+move, scene()->objects()[0].boundingBox().min()+move);

				if(draw)
				{
					program->bind();
					program->setUniformValue("MVP", MVP);
					program->setUniformValue("OQ", OQ);
					drawObject(0);

					objects_draw += 1;
				}
				move.setZ(move.z()+distz); //Translate the vertices of the bounding box
				MVP.translate(0.0,0.0,distz);
			}	
		}
		
		
		painter.begin(glwidget());
    	painter.drawText(10, 60, QString("%0 Objects inside").arg(objects_draw));
		painter.end();
		
	}

	/*--OCCULUSION QUERIES--------------------------------------------------------------
	
	In this part we render using Occlusion Queries. 	
	In order to get the querie result we first initialize the queries, and with the depth and
	color disabled we draw the bounding box of each object. In the render loop we get the 
	result of the query which determine if the object is rendered or not. Only objects that 
	get a result different than 0 will be rendered.

	Press o to change to Occlusion Queries Mode.

	*/

	if(OQ and !FC)
	{
		//Create N Queries
		int N = NumberOfCopies_x*NumberOfCopies_z;
		GLint available; 
		GLuint queries[N];
		GLuint sampleBoolean;
		g.glGenQueries(N, queries);
		bool OQ_shader = true; //Uniform to pass to the vertex shader: true->Bounding Box, false->Object

		//Disable Rendering, depth buffer, and lighting
		g.glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		g.glDepthMask(GL_FALSE);
		g.glDisable(GL_LIGHTING);

		//Bind shader and send the uniforms in order to render the Bounding Box
		program->bind();
		program->setUniformValue("OQ", OQ_shader);
		//We need to pass these two vectors just once, because we will change the MVP to move the position of the the Bounding Box vertices.
		program->setUniformValue("BBMax", scene()->objects()[0].boundingBox().max());
		program->setUniformValue("BBMin", scene()->objects()[0].boundingBox().min());

		int qi = 0;
		
		//Occlusion Queries loop -> Render the Bounding Box
		for(unsigned int j=0; j<NumberOfCopies_x; j++)
		{
			MVP=camera()->projectionMatrix()*camera()->viewMatrix();
			MVP.translate(distx*j,0.0,0.0);
			for(unsigned int k=0; k<NumberOfCopies_z; k++)
			{
				g.glBeginQuery(GL_ANY_SAMPLES_PASSED, queries[qi]);

				program->setUniformValue("MVP", MVP);
				DrawBoundingBox();	

				g.glEndQuery(GL_ANY_SAMPLES_PASSED);

				MVP.translate(0.0,0.0,distz);
				qi +=1;		
			}
		}
		
		int i = N*3/4;
		
		do {
			g.glGetQueryObjectiv(queries[i],GL_QUERY_RESULT_AVAILABLE, &available);
		} while (!available);
		
		//Enable Rendering, depth buffer and lighting
		g.glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		g.glDepthMask(GL_TRUE);
		g.glDisable(GL_LIGHTING);

		qi = 0;
		int count = 0; //Counter for the number of objects rendered
		OQ_shader = false; 

		//Rendering Loop -> Get the Querie result and if it is different than 0 render the object
		for(unsigned int j=0; j<NumberOfCopies_x; j++)
		{
			MVP=camera()->projectionMatrix()*camera()->viewMatrix();
			MVP.translate(distx*j,0.0,0.0);
			for(unsigned int k=0; k<NumberOfCopies_z; k++)
			{
				g.glGetQueryObjectuiv(queries[qi], GL_QUERY_RESULT, &sampleBoolean);

				if (sampleBoolean != 0)
				{
					program->bind();
					program->setUniformValue("MVP", MVP);
					program->setUniformValue("OQ", OQ_shader);
					drawObject(0);
					count +=1;
				}
				MVP.translate(0.0,0.0,distz);
				qi += 1;
			}	
		}

		//Print the number of objects occluded
		painter.begin(glwidget());
    	painter.drawText(10, 60, QString("%0 Objects occluded").arg(N-count));
		painter.end();

		g.glDeleteQueries(N, queries);
		
	}

	/*--DEFAULT--------------------------------------------------------------
	
	Render the N objects withoud applying Occlusion Queries or Frustum Culling.

	Press-> x: Face Culling Mode
			o: Occlusion Queries Mode
			Press again "x" or "o" to return to Default Mode.
	*/
	if(!OQ and !FC){
		for(unsigned int j=0; j<NumberOfCopies_x; j++)
		{
			MVP=camera()->projectionMatrix()*camera()->viewMatrix();
			MVP.translate(distx*j,0.0,0.0);
			for(unsigned int k=0; k<NumberOfCopies_z; k++)
			{
				program->bind();
				program->setUniformValue("MVP", MVP);
				program->setUniformValue("OQ", OQ);
				drawObject(0);
				MVP.translate(0.0,0.0,distz);
			}	
		}
	}	
    return true;
}

void OcclusionQueries::postFrame() 
{ 
	program->bind();
	//Print FPS
    painter.begin(glwidget());
    painter.drawText(10, 20, QString("%0 fps").arg(fps));
	painter.drawText(10, 40, QString("%0 objects").arg(NumberOfCopies_x*NumberOfCopies_z));
	painter.drawText(10, 460, QString("Mode: "));
	if(OQ and !FC)
		painter.drawText(55, 460, QString("Occlusion Queries"));
	if(FC and !OQ)
		painter.drawText(55, 460, QString("Frustum Culling"));	
	if(!OQ and !FC)
		painter.drawText(55, 460, QString("Default"));
    painter.end();
    ++fcnt;
}


void OcclusionQueries::LoadShaders()
{
	vs = new QOpenGLShader(QOpenGLShader::Vertex, this);
	vs->compileSourceFile(QString("shaders/basic.vert"));
	cout << "VS log:" << vs->log().toStdString() << endl;
	
	fs = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fs->compileSourceFile(QString("shaders/basic2.frag"));
    cout << "FS log:" << fs->log().toStdString() << endl;
    
	program = new QOpenGLShaderProgram(this);
    program->addShader(vs);
    program->addShader(fs);
    program->link();
    cout << "Link log:" << program->log().toStdString() << endl;
}

void OcclusionQueries::keyPressEvent(QKeyEvent *e)
{
	if (e->key()==Qt::Key_O){
		if (OQ== false) {
			OQ=true;
			FC=false;
		}
		else {
			OQ=false;
			FC=false;
		}
	}
	if (e->key()==Qt::Key_X){
		if (FC== false) 
		{
			OQ=false;
			FC=true;
		}
		else {
			FC=false;
			OQ=false;
		}
	}
}

bool OcclusionQueries::drawObject(int i)
{
    GLWidget &g = *glwidget();
    g.makeCurrent();
    g.glBindVertexArray(VAOs[i]);
    g.glDrawArrays(GL_TRIANGLES, 0, numIndices[i]);
    g.glBindVertexArray(0);

	return true;
}

void OcclusionQueries::createBoundingBox()
{
	GLWidget &g = *glwidget();
	g.makeCurrent();
    
  	GLfloat BB_vertices[]={
		1, 1, 1,
		0, 1, 1,
		1, 0, 1,
		0, 0, 1,
		1, 0, 0,
		0, 0, 0,
		1, 1, 0,
		0, 1, 0,
		1, 1, 1,
		0, 1, 1,
		0, 1, 1,
		0, 1, 0,
		0, 0, 1,
		0, 0, 0,
		1, 0, 1,
		1, 0, 0,
		1, 1, 1,
		1, 1, 0
	};

	g.glGenVertexArrays(1, &BBVAO);
  	g.glBindVertexArray(BBVAO);

  	g.glGenBuffers(1, &BBVB);
  	g.glBindBuffer(GL_ARRAY_BUFFER, BBVB);
  	g.glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*18*3, BB_vertices, GL_STATIC_DRAW);
  	g.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  	g.glEnableVertexAttribArray(0);
	
}

void OcclusionQueries::DrawBoundingBox()
{
  	GLWidget &g = *glwidget();
	g.makeCurrent();

	g.glBindVertexArray(BBVAO);
    g.glDrawArrays(GL_TRIANGLE_STRIP, 0, 18);
    g.glBindVertexArray(0);
}

void OcclusionQueries::cleanUp()
{
  GLWidget &g = *glwidget();
  g.glDeleteBuffers(coordBuffers.size(),  &coordBuffers[0]);
  g.glDeleteBuffers(normalBuffers.size(), &normalBuffers[0]);
  g.glDeleteBuffers(stBuffers.size(),  &stBuffers[0]);
  g.glDeleteBuffers(colorBuffers.size(),  &colorBuffers[0]);
  g.glDeleteVertexArrays(VAOs.size(), &VAOs[0]);
  coordBuffers.clear();
  normalBuffers.clear();
  stBuffers.clear();
  colorBuffers.clear();
  VAOs.clear();
  numIndices.clear();
}

void OcclusionQueries::addVBO(unsigned int currentObject)
{
  //
  // For simplicity, we construct VBOs with replicated vertices (a copy
  // for each triangle to which they belong:
  //
  const Object& obj = scene()->objects()[currentObject];
  unsigned int numvertices = obj.faces().size()*3;  // it's all triangles...
  vector<float> vertices; // (x,y,z)    Final size: 9*number of triangles
  vector<float> normals;  // (nx,ny,nz) Final size: 9*number of triangles
  vector<float> colors;   // (r, g, b)  Final size: 9*number of triangles
  vector<float> texCoords;// (s, t)     Final size: 6*number of triangles
  auto verts = obj.vertices();
  auto Ns = obj.vertNormals();
  auto texcords = obj.vertTexCoords();

  for (auto&& f: obj.faces()) {
    Point P = verts[f.vertexIndex(0)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    Vector V=Ns[f.normalIndex(0)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    auto TC=texcords[f.texcoordsIndex(0)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);

    P = verts[f.vertexIndex(1)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    V=Ns[f.normalIndex(1)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    TC=texcords[f.texcoordsIndex(1)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);

    P = verts[f.vertexIndex(2)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    V=Ns[f.normalIndex(2)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    TC=texcords[f.texcoordsIndex(2)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);
}

  assert(vertices.size() == 3*numvertices);
  assert(normals.size() == 3*numvertices);
  assert(colors.size() == 3*numvertices);
  assert(texCoords.size() == 2*numvertices);

  // Step 2: Create VAO and empty buffers (coords, normals, ...)
  GLWidget& g = *glwidget();
  GLuint VAO;
  g.glGenVertexArrays(1, &VAO);
  VAOs.push_back(VAO);
  g.glBindVertexArray(VAO);
  
  GLuint coordBufferID;
  g.glGenBuffers(1, &coordBufferID);
  coordBuffers.push_back(coordBufferID);
  
  GLuint normalBufferID;
  g.glGenBuffers(1, &normalBufferID);
  normalBuffers.push_back(normalBufferID);
  
  GLuint stBufferID;
  g.glGenBuffers(1, &stBufferID);
  stBuffers.push_back(stBufferID);
  
  GLuint colorBufferID;
  g.glGenBuffers(1, &colorBufferID);
  colorBuffers.push_back(colorBufferID);
  
  numIndices.push_back(numvertices);
  // Step 3: Define VBO data (coords, normals, ...)
  g.glBindBuffer(GL_ARRAY_BUFFER, coordBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0); 
  g.glEnableVertexAttribArray(0);

  g.glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
  g.glEnableVertexAttribArray(1);

  g.glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*colors.size(), &colors[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, 0);
  g.glEnableVertexAttribArray(2);

  g.glBindBuffer(GL_ARRAY_BUFFER, stBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 0, 0);
  g.glEnableVertexAttribArray(3);
  
  g.glBindBuffer(GL_ARRAY_BUFFER,0);
  g.glBindVertexArray(0);
}










