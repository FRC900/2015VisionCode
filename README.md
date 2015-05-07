# 2015VisionCode #

This is Team 900s Vision Code for Recycle Rush.

The goal was to write code that would detect bins and find their location. This was done using cascade classifiers.
The final binary is cross platform and accelerates using a NVIDIA GPU whenever availble.

# Directories: #
1. bindetection - pretty much all the detection code
2. C920VideoCap - code for interfacing with the C920 Camera
3. camera_cal - ignore
4. cascade_training - code for training detection code to work. Also includes classifiers for detecting bins
5. create_grayscale - ignore
6. detection for empty - old idea, ignore
7. framegrabber - useful utility to grab specific frame or percentage of frames
8. imageclipper - utility to grab parts of frames of a video. Contains README in folder. Taken from [here](https://github.com/JoakimSoderberg/imageclipper).
9. networktables - code for using network tables for interfacing with LabView on the RoboRIO
1. older detection methods - ideas that we tried before classifiers. You can look at it if you want, probably not very useful

# How to use this #

To start you need:
+ Videos in different conditions with the object that you are trying to detect in it (positive videos)
+ Many videos without ANY OCCURENCES of the object you need to detect (negative videos)

1. Use imageclipper to extract samples of the object from the positive videos. Imageclipper's README is very good and there is a slight modification to the program. The program will highlight your selection in green when the aspect ratio is best for the classifier. Repeat this process until you have around 300 positives. Make sure to get pictures from different angles and in different conditions. Also make sure that when you grab them there isn't much else in the sample except the object.

2. Run the framegrabber on a negative video. By default this should give you 1% of all frames to use as negative images for the initial classifier stage.

3. Put the negatives into the negative_images directory and the positives into the positive_images directory. Both are within cascade_training.

4. Run prep.sh.