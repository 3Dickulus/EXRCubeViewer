# EXRCubeViewer
A program to view EXR multipart format files of cubic dimensions X==Y==Z

A Fragmentarium support tool brought to you by Digilantism ;)

## Compiling

    install and setup a C++ dev environment
    install OpenEXR dev package
    install Qt dev package
    install QGLViewer dev package

create a build folder in EXRCubeViewer folder

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
 

## Generating files for use with EXRCubeViewer

copy the frag files to a folder of their own (only work on copies)

run Fragmentarium-2.5.5

drag the copy of slice-test.frag onto the Fragmentarium window to load

ensure that the animation runtime length is at least 256 frames ie: 11 seconds @ 25fps or 9 seconds @ 30fps

from the "Render" menu select "High Resolution and Animation Render"

set "Render Tiles" to 1x1

Set "Padding" to 0.0%

set "Tile Width" to 256

set "Tile Height" to 256

set "Number of Subframes" to any number you like but for some fast results setting it to 1 (one) will do

select the "Animation" checkbox and set "Start Frame" and "End Frame" to 1 and 256 respectively.

set the "Filename" to "TestCube.png"

unset "Add unique ID to filename"

set "Autosave fragments and settings" to ON

unset "Save texture files"

select "OK"


this will generate 256 images in a folder named TestCube_Files/images

![slice-test 256x256](https://user-images.githubusercontent.com/4978723/123578379-8e6e8b00-d78a-11eb-9e47-a75b8a94b0cf.gif)

These images can be used to create a multipart file with the OpenEXR tool called **exrmultipart**

    Usage: exrmultipart -combine -i input.exr[:partnum][::partname] [input2.exr[:partnum]][::partname] [...] -o outfile.exr [options]
    or: exrmultipart -separate -i infile.exr -o outfileBaseName [options]
    or: exrmultipart -convert -i infile.exr -o outfile.exr [options]

I use a small bash script called mkcombined.sh to get the job done... it lives in the Fragmentarium Misc folder, this script assumes that the bin folder is populated with tools from the OpenEXR package, you may have to edit to suite your needs

    #!/bin/bash
    # usage: mkcombined.sh <basename> <resolution>
    #    eg: ./sh mkcombined.sh myimgfiles 256x256x256
    # creates a file named myimgfiles-combined-256x256x256.exr that contains all exr files with basename
    
    bin/exrmultipart -combine -i `ls $1.*.exr` -o $1-combined-$2.exr `

After generating the EXR cube file you can load it with EXRCubeViewer to see something like this ...

![EXRCubeViewer-test](https://user-images.githubusercontent.com/4978723/123578172-199b5100-d78a-11eb-9b3d-d1e8046b93a3.gif)

once loaded in EXRCubeViewer you can save the voxel field as a wavefront .obj file or a pointcloud .pcd file
