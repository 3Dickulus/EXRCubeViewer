/****************************************************************************
 * This file is Copyright © 2021 Richard Paquette
 * Distributed as part of the Fragmentarium package under the terms of the
 * GNU General Public License in the hope that it will be useful,
 *  
 * This file uses the QGLViewer library version 2.6.3.
 * Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.
 * http://www.libqglviewer.com - contact@libqglviewer.com
 *
 * This file may be used under the terms of the GNU General Public License
 * versions 2.0 or 3.0 as published by the Free Software Foundation and
 * appearing in the LICENSE file included in the packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *****************************************************************************/


#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedCameraFrame.h>

class Viewer : public QGLViewer
{
public :
  Viewer();
  virtual ~Viewer();
  
protected :
  virtual void preDraw();
  virtual void draw();
  virtual void init();
  virtual void keyPressEvent(QKeyEvent *e);
  virtual QString helpString() const;

  void resize(int,int);
  void initScene();
  void loadFile();
  void saveFile();
  void saveWavefrontFile();
  void savePointCloudDataFile();

private :
  float *voxels;
  QString exrCubeFileName;
  QString objFileName;
  GLuint m3DTexId;
  int exrCubeX;
  int exrCubeY;
  int exrCubeZ;
  float step;
  GLfloat dOrthoSize;
};
