/****************************************************************************
 * This program uses the QGLViewer library version 2.6.3.
 * Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.
 * http://www.libqglviewer.com - contact@libqglviewer.com
 * 
 * This program uses the OpenEXR library to load a Multipart EXR file.
 * Copyright (c) 2005-2012, Industrial Light & Magic, a division of Lucas
 * Digital Ltd. LLC All rights reserved.
 * 
 * This program is Copyright © 2021 Richard Paquette
 * Distributed as part of the Fragmentarium package under the terms of the
 * GNU General Public License in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 ****************************************************************************/

#include "EXRCubeViewer.h"
#include <qapplication.h>

int main(int argc, char** argv)
{
  QApplication application(argc,argv);

  Viewer viewer;

  viewer.setWindowTitle("EXRCubeViewer");

  viewer.show();

  return application.exec();
}
