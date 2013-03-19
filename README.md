HeadPoseEstimation
================================================

Compiling
-----------------------------
This application requires OpenCV. Depending on which version of OpenCV you use, you should set the value of 'openCVLocation' in the file image.h.

To compile the program, execute the following commands in the source directory.

    mkdir build
    cd build
    qmake ../HeadPoseEstimation.pro
    make

Data
-----------------------------
The program was made for the Bosphorus database. The program expects that the files have the following structure. The directory "data" contains the Bosphorus database (you can download this at https://dl.dropbox.com/u/27590885/HeadPoseEstimation.zip).

    src/
        data/
            bs000/
                bs000_N_N_0.lm2
                bs000_N_N_0.png
            bs001/
        *.cpp
        *.h
        *.pro
    build/

Running
------------------------------
Move the build directory outside the source directory (make sure that these 2 directories are at the same level). The program has two phases: training & testing. While training the program will make files that contain relevant information. There are 4 different files: positionsPitch, positionsPitchOrig, positionsYaw and positionsYawOrig. The test phase uses these files, so if you get unexpected bad results, check if you have the needed files. The directory "training" contains the training files for the previously mentioned data.

You can execute the program without arguments or with arguments. If you execute the following command.

    ./HeadPoseEstimation
    
The program will now start and do a cross validation.

If you execute the following command

    ./HeadPoseEstimation /home/user/image.png
    
The program will estimate the pose for the given image.
