# EXRCubeViewer
A program to view EXR multipart format files of cubic dimensions X==Y==Z

## Compiling
install QGLViewer dev package and create a build folder in EXRCubeViewer folder

`  cd EXRCubeViewer
  mkdir build`

change directory to the build folder

`  cd build`

initialize with cmake

`  cmake ..`

compile the executable

`  make`

test the program

`  ./EXRCubeViewer`

## Running
There are only a few commands available via the "h" "l" and "s" keys

 "H"elp displays some info
 
 "L"oad opens an EXR multipart file
 
 "S"ave creates a wavefront .obj file or a pointcloud .pcd file
 
 
