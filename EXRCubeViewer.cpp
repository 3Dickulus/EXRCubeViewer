/****************************************************************************
 * This program is Copyright © 2021 Richard Paquette
 * Distributed as part of the Fragmentarium package under the terms of the
 * GNU General Public License in the hope that it will be useful,
 *  
 * This file may be used under the terms of the GNU General Public License
 * versions 2.0 or 3.0 as published by the Free Software Foundation and
 * appearing in the LICENSE file included in the packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *****************************************************************************/

#include "EXRCubeViewer.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <qfiledialog.h>
#include <QKeyEvent>
#include <QFile>
#include <QMessageBox>
#include <QMainWindow>
#include <QAction>
#include <QMatrix4x4>

#include <OpenEXR/half.h>
#include <OpenEXR/ImfTileDescription.h>
#include <OpenEXR/ImfTiledOutputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfTiledRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfNamespace.h>
#include <OpenEXR/ImfMultiPartInputFile.h>
#include <OpenEXR/ImfTiledInputPart.h>
#include <OpenEXR/ImfPartHelper.h>
#include <OpenEXR/ImfPartType.h>
#include <OpenEXR/Iex.h>
#include <OpenEXR/OpenEXRConfig.h>


using namespace qglviewer;
using namespace std;

using namespace Imf;
using namespace Imath;
using namespace Iex;

Viewer::Viewer() : voxels(0), m3DTexId(0), dOrthoSize(1.0f) {

  QAction* a = new QAction(this);
  a->setText( "Quit" );
  connect(a, SIGNAL(triggered()), SLOT(close()) );
}

Viewer::~Viewer() {}

void Viewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_L :
        loadFile();
        break;
	case Qt::Key_S :
		saveFile();
		break;
	case Qt::Key_H :
		QMessageBox::information(this, tr("Help"), helpString());
		break;
	default:
        QGLViewer::keyPressEvent(e);
    }
}

QString Viewer::helpString() const
{
	QString text("<h2>EXR  Cube  Viewer</h2>");
	text += "<hr>";
	text += "This program uses the QGLViewer library version 2.6.3. ";
	text += "Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.<br>";
	text += "http://www.libqglviewer.com - contact@libqglviewer.com<br>";
	text += "<br>";
	text += "This program uses the OpenEXR library to load a Multipart EXR file.<br>";
	text += "Copyright (c) 2005-2012, Industrial Light & Magic, a division of Lucas ";
	text += "Digital Ltd. LLC All rights reserved.<br>";
	text += "<br>";
	text += "This program is Copyright © 2021 Richard Paquette<br>";
	text += "Distributed as part of the Fragmentarium package under the terms of the GNU General Public License in the hope that it will ";
	text += "be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or ";
	text += "FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.";
	text += "<hr><br>";
	text += "Press <b>L</b>(oad) to open a multipart EXR file.<br>";
    text += "Press <b>S</b>(ave) to write a pointcloud or wavefront file.<br><br>";
    return text;
}

void Viewer::loadFile()
{
// request a filename from user
    exrCubeFileName = QFileDialog::getOpenFileName(this, "Select a cube file", ".", "EXR files (*.exr *.EXR);;All files (*)");

    // In case of Cancel
    if (exrCubeFileName.isEmpty())
        return;

    //
    // parse the multipart input
    //
    string filename ( exrCubeFileName.toStdString() );

    // add check for existance of the file
    try
    {
        MultiPartInputFile temp (filename.c_str());
    }
    catch (BaseExc &e)
    {
        cerr << endl << "ERROR: " << e.what() << endl;
        return;
    }

    MultiPartInputFile *inputimage = new MultiPartInputFile (filename.c_str());
    int zLayers = inputimage->parts();

    glGetIntegerv ( GL_MAX_3D_TEXTURE_SIZE, &exrCubeZ );
    exrCubeZ/=4;

    //
    // separate outputs
    //
    int p = 0;
    Box2i dw = inputimage->header(p).dataWindow();
    exrCubeX  = dw.max.x - dw.min.x + 1;
    exrCubeY = dw.max.y - dw.min.y + 1;

    if ( exrCubeZ < zLayers || exrCubeX > exrCubeZ || exrCubeY > exrCubeZ ) {
        cerr << "EXR voxel loader found X:" << exrCubeX << " Y:" << exrCubeY << " Z:" << zLayers << " too large! max " << exrCubeZ << "x" << exrCubeZ << endl;
        return;
    }
    else if ( exrCubeX != zLayers || exrCubeY != zLayers) {
        qDebug() << QString ( "EXR voxel loader found X:%1 Y:%2 Z:%3" ).arg ( exrCubeX ).arg ( exrCubeY ).arg ( zLayers );
        return;
    }

    exrCubeZ = zLayers < exrCubeZ ? zLayers : exrCubeZ;

	QString mess1 = QString(" W:%1 H:%2 Z:%3").arg(exrCubeX).arg(exrCubeY).arg(exrCubeZ);
	displayMessage(mess1);

    if(voxels != 0) {
        delete [] voxels;
        voxels = 0;
    }

    voxels = new float[exrCubeX*exrCubeY*exrCubeZ*4];

	QString mess2 = QString(" Cube layers:%1").arg(zLayers);
	displayMessage(mess2);
	
	while (p < zLayers)
    {
        Header header = inputimage->header(p);

        string type = header.type();

        if ( inputimage->partComplete(p) ) {

            Array2D<Rgba>pixels ( exrCubeX, exrCubeY );
            TiledInputPart tip( *inputimage, p );

            FrameBuffer frameBuffer;

            frameBuffer.insert ("R",                                     // name
                                Slice (HALF,                        // type
                                       (char *) &pixels[0][0].r,     // base
                                       sizeof (pixels[0][0]) * 1,       // xStride
                                       sizeof (pixels[0][0]) * exrCubeX)); // yStride

            frameBuffer.insert ("G",                                     // name
                                Slice (HALF,                       // type
                                       (char *) &pixels[0][0].g,     // base
                                       sizeof (pixels[0][0]) * 1,       // xStride
                                       sizeof (pixels[0][0]) * exrCubeX)); // yStride

            frameBuffer.insert ("B",                                     // name
                                Slice (HALF,                        // type
                                       (char *) &pixels[0][0].b,     // base
                                       sizeof (pixels[0][0]) * 1,       // xStride
                                       sizeof (pixels[0][0]) * exrCubeX)); // yStride

            //       frameBuffer.insert ("A",                                     // name
            //                           Slice (HALF,                       // type
            //                                  (char *) &pixels[0][0].a,     // base
            //                                  sizeof (pixels[0][0]) * 1,       // xStride
            //                                  sizeof (pixels[0][0]) * w)); // yStride

            tip.setFrameBuffer ( frameBuffer );
            tip.readTile( 0, 0 );

            // convert to GL pixels
            for ( int j = 0; j<exrCubeY; j++ ) {
                for ( int i = 0; i<exrCubeX; i++ ) {
                    // convert 3D array to 1D
                    int indx = ( (exrCubeX*exrCubeY*p) + (j*exrCubeX+i) ) *4;
                    voxels[indx]=pixels[j][i].r;
                    voxels[indx+1]=pixels[j][i].g;
                    voxels[indx+2]=pixels[j][i].b;
                    // in this case we use r+g+b / 3 as alpha value
                    voxels[indx+3] = (pixels[j][i].r+pixels[j][i].g+pixels[j][i].b) * 0.25; // setting alpha
                }
            }

        } else {
			QString mess3 = QString(" Exrloader found EXR image: %1 part %2 is not complete!").arg(exrCubeFileName).arg(p);
			displayMessage(mess3);
		}

        p++;
    }

    QString mess4 = QString("Sending data to GPU...");
	displayMessage(mess4);
    
	stopAnimation();

    initScene();

	QString mess5 = QString("Done.");
	displayMessage(mess5);
	
    delete inputimage;

    step = 1.0/(float)zLayers;

    update();
}

void Viewer::saveFile() {

    // request a filename from user
    objFileName = QFileDialog::getSaveFileName(this, "Select a file", ".", "Object Files(*.pcd *.PCD *.obj *.OBJ);;All files (*)");

    // In case of Cancel
    if (objFileName.isEmpty())
        return;

    if(objFileName.endsWith(".obj", Qt::CaseInsensitive)) saveWavefrontFile();
    else
    if(objFileName.endsWith(".pcd", Qt::CaseInsensitive)) savePointCloudDataFile();

}

void Viewer::saveWavefrontFile() {

	QString mess1 = QString("Saving Wavefront format point cloud...");
	displayMessage(mess1);

    QFile fileStream( objFileName );
    if (!fileStream.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Wavefront format"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(objFileName)
                             .arg(fileStream.errorString()));
        return;
    }

    QTextStream out(&fileStream);
    int pcnt = 0;

    out << "# Point Cloud from EXRQ 0.1\n";
    for(int z=0; z<exrCubeZ; z++) {
        for(int y=0; y<exrCubeY; y++) {
            for(int x=0; x<exrCubeX; x++) {
                int indx = ((exrCubeX*exrCubeY*z) + (y*exrCubeZ+x))*4;
                float scale = exrCubeZ*0.5;
                if( voxels[indx+3] != 0.0 ) {
                  out << "v " << (0.5-(scale*x))*2.0 << " " << (0.5-(scale*y))*2.0 << " " << (0.5-(scale*z))*2.0 <<  "\n";
                    pcnt++;
                }
            }
        }
    }
    QString mess2 = QString("Done %1 points out of %2").arg(pcnt).arg(exrCubeX*exrCubeY*exrCubeZ);
	displayMessage(mess2);

}

void Viewer::savePointCloudDataFile() {

	QString mess1 = QString("Saving Point Cloud Data file...");
	displayMessage(mess1);

  QFile fileStream( objFileName );
  if (!fileStream.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Point Cloud Data format"),
                         tr("Cannot write file %1:\n%2.")
                         .arg(objFileName)
                         .arg(fileStream.errorString()));
    return;
  }

  QTextStream out(&fileStream);
  int pcnt = 0;

  out << QString("# .PCD v.7 - Point Cloud Data file from EXRQ 0.1\n");
  out << QString("VERSION .7\n");
  out << QString("FIELDS x y z rgb\n");
  out << QString("SIZE 4 4 4 4\n");
  out << QString("TYPE F F F F\n");
  out << QString("COUNT 1 1 1 1\n");

  for(int z=0; z<exrCubeZ; z++) {
    for(int y=0; y<exrCubeY; y++) {
      for(int x=0; x<exrCubeX; x++) {
        int indx = ((exrCubeX*exrCubeY*z) + (y*exrCubeZ+x))*4;
        if( voxels[indx+3] != 0.0 ) {
          pcnt++;
        }
      }
    }
  }

  // this is an unorganized point cloud, WIDTH == total points, HEIGHT = 1
  out << QString("WIDTH %1\n").arg(pcnt);
  out << QString("HEIGHT 1\n");
  out << QString("VIEWPOINT 0 0 0 1 0 0 0\n");
  out << QString("POINTS %1\n").arg(pcnt);
  out << QString("DATA ascii\n");

  pcnt = 0;
  for(int z=0; z<exrCubeZ; z++) {
    for(int y=0; y<exrCubeY; y++) {
      for(int x=0; x<exrCubeX; x++) {
        int indx = ((exrCubeX*exrCubeY*z) + (y*exrCubeZ+x))*4;
        if( voxels[indx+3] != 0.0 ) {
          pcnt++;
          uint32_t rgb = (
            static_cast<uint32_t>(voxels[indx]*256.0) << 16 |
            static_cast<uint32_t>(voxels[indx+1]*256.0) << 8 |
            static_cast<uint32_t>(voxels[indx+2]*256.0)
          );
          float rgbf = *reinterpret_cast<float*>(rgb);
          float scale = 1.0/16.0;
          out << scale*(256.0-x) << " " << scale*(256.0-y) << " " << scale*(256.0-z) << " " << rgbf << "\n";
        }
      }
    }
  }
  QString mess2 = QString("Done %1 points out of %2").arg(pcnt).arg(exrCubeX*exrCubeY*exrCubeZ);
  displayMessage(mess2);
}


void Viewer::init()
{
    // disable these flags to get back to a standard OpenGL state.
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glClearColor( 0.0f,0.0f, 0.0f, 0.0f );

    setBackgroundColor( QColor("Black") );
    setForegroundColor( QColor("White") );

    // Camera
    camera()->setType(Camera::ORTHOGRAPHIC);
    camera()->setPosition(Vec(0.0, 0.0, -2.0));
    camera()->lookAt(Vec(0.0, 0.0, 0.0));
    camera()->setFieldOfView(M_PI/180.0*60.0);

    resize(width(),height());
    showEntireScene(); // Previous state cannot be restored: fit camera to scene.

    setWheelBinding (Qt::KeyboardModifiers(0), CAMERA, NO_MOUSE_ACTION);
    setMouseBinding (Qt::KeyboardModifiers(0), Qt::RightButton, CAMERA, NO_MOUSE_ACTION);

    displayMessage("Press H for help.");
}


void Viewer::initScene()
{
    // send 3dtexture data to GPU
    if( 0 != m3DTexId )
    {
        glDeleteTextures( 1, (GLuint*)&m3DTexId );
    }
    glGenTextures(1,(GLuint*)&m3DTexId );
    glBindTexture( GL_TEXTURE_3D, m3DTexId );
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D ( GL_TEXTURE_3D, 0, GL_RGBA , exrCubeX, exrCubeY, exrCubeZ, 0, GL_RGBA, GL_FLOAT, voxels );
    glBindTexture( GL_TEXTURE_3D, 0 );

    if( !m3DTexId ) {
        cerr << "GL_TEXTURE_3D initialialization failed!" << endl;
        exit(-1);
    }
}


void Viewer::preDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.05f );

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    resize(width(),height());
}

void Viewer::draw()
{
    if( !m3DTexId )
        return;

    glPushMatrix();

    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    // Translate and make 0.5f as the center
    // (texture co ordinate is from 0 to 1. so center of rotation has to be 0.5f)
    glTranslatef( 0.5f, 0.5f, 0.5f );

    glScaled( aspectRatio(), -1.0f, 2.0);

    // Apply the camera transformations to the texture matrix
    QMatrix4x4 m;
    camera()->getProjectionMatrix( m.data() );
    glMultMatrixf( m.data() );
    camera()->getModelViewMatrix( m.data() );
    glMultMatrixf( m.data() );

    glTranslatef( -0.5f,-0.5f, -0.5f );

    glEnable(GL_TEXTURE_3D);
    glBindTexture( GL_TEXTURE_3D, m3DTexId );
    for ( float tIndx = -1.0f; tIndx <= 1.0f; tIndx+=step )
    {
        glBegin(GL_QUADS);

        glTexCoord3f(0.0f, 0.0f, ((float)tIndx+1.0f)/2.0f);        glVertex3f(-dOrthoSize,-dOrthoSize,tIndx);
        glTexCoord3f(1.0f, 0.0f, ((float)tIndx+1.0f)/2.0f);        glVertex3f( dOrthoSize,-dOrthoSize,tIndx);
        glTexCoord3f(1.0f, 1.0f, ((float)tIndx+1.0f)/2.0f);        glVertex3f( dOrthoSize, dOrthoSize,tIndx);
        glTexCoord3f(0.0f, 1.0f, ((float)tIndx+1.0f)/2.0f);        glVertex3f(-dOrthoSize, dOrthoSize,tIndx);

        glEnd();
    }
   glBindTexture( GL_TEXTURE_3D, 0 );

   glPopMatrix();

}

void Viewer::resize( int nW, int nH )
{
  //Find the aspect ratio of the window.
  GLdouble AspectRatio = ( GLdouble )(nW) / ( GLdouble )(nH );
  //glViewport( 0, 0, cx , cy );
  glViewport( 0, 0, nW, nH );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  //Set the orthographic projection.
  if( nW <= nH )
  {
    glOrtho( -dOrthoSize, dOrthoSize, -( dOrthoSize / AspectRatio ) ,
             dOrthoSize / AspectRatio, 2.0f*-dOrthoSize, 2.0f*dOrthoSize );
  }
  else
  {
    glOrtho( -dOrthoSize * AspectRatio, dOrthoSize * AspectRatio,
             -dOrthoSize, dOrthoSize, 2.0f*-dOrthoSize, 2.0f*dOrthoSize );
  }

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}
