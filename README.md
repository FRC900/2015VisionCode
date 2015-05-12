# 2015VisionCode #

This is Team 900s Vision Code for Recycle Rush.

The goal was to write code that would detect bins and find their location. This was done using cascade classifiers.
The final binary is cross platform and accelerated using a NVIDIA GPU whenever availble.

# Directories: #
1. bindetection - pretty much all the detection code
2. C920VideoCap - code for interfacing with the C920 Camera
3. camera_cal - ignore - implemented camera calibration code from The Secret Book of FRC LabVIEW
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
+ Computer to run training with 4GB of RAM (More is better)

1. Use imageclipper to extract samples of the object from the positive videos. Imageclipper's README is very good and there is a slight modification to the program. The program will highlight your selection in green when the aspect ratio is best for the classifier. Repeat this process until you have around e^4.6 positives. Make sure to get pictures from different angles and in different conditions. Also make sure that when you grab them there isn't much else in the sample except the object.

2. Run the framegrabber on a negative video. By default this should give you 1% of all frames to use as negative images for the initial classifier stage.

3. Put the negatives into the negative_images directory and the positives into the positive_images directory. Both are within cascade_training.

4. Run prep.sh. This takes each positive image and creates a set of randomly rotated versions to use for training. Outputs a .vec file. There are some values you can tweak at this point and documentation of those are in the prep.sh file.

6. READ the run_training.pl documentation. These are important parameters to tweak before running the script. Here's a stripped down version:
  + \-data \-\- name of directory to store classifier in. Make sure it exists before you run it.
  + \-numStages \-\- maximum number of stages to generate. All this does is basically stop the code after it's "good enough". Default is usually fine (55).
  + \-numPos \-\- number of positives the training uses. This should be 85ish % of the number of positives you created.
  + \-numNeg \-\- This should be about the same as the number of positives. The higher this is the longer each stage will take.
  + \-precalcValBufSize and \- precalcIdxBufSize \-\- These change the amount of memory used by the classifier. This is very important because the classifier training is memory bound. These should both be about 1/3 of your memory size in megabytes.

7. Run run_training.pl. This will open a command window and show you information about how the classifier is doing.
Example output:

	```
===== TRAINING 0-stage =====
<BEGIN
POS count : consumed   9000 : 9000
NEG count : acceptanceRatio    10000
Precalculation time: 37
+----+---------+---------+
|  N |    HR   |    FA   |
+----+---------+---------+
|   1|        1|        1|
+----+---------+---------+
...stages
+----+---------+---------+
|   8| 0.999333|   0.8318|
+----+---------+---------+
... more stages
+----+---------+---------+
|  24| 0.999111|   0.4605|
+----+---------+---------+
	```

	+ N \- Number of stages completed.
	+ HR \- Hit Rate. This is the fraction of positive images that are detected by the classifier. The training should keep this above 0.999 on it's own. This value can be tweaked in run_training wit the minHitRate parameter (Not recommended)
	+ FA \- False Alarms. This is the important number. This tells you what perecentage of negatives get recognized as positives by the classifier. Once this is below 0.5 the classifier will move to the next stage
	Each stage accepts 99.9% of real targets and rejects 50% of false alarms. With each additional stage, almost all positive images will be successfully detected while an additional 50% of the remaining false alarms (false positives) will be filtered out. With a sufficient number of stages (30-40?) a large number of positives will be let through the last stage while a huge majority of false alarms will be filtered out.

8. After about 25 stages stop training.  The training code might also fail or run very slowly due to a lack of negative images.

7. Run create_cascade.sh. Put the classifier into the generate_negatives folder and generate a fresh set of negative images from assorted negative videos.  This will generate a set of images known as hard negatives - ones which are detected by the current classifier but shouldn't be.

9. Add these new negatives negatives to the negatives directory.

1. Continue training the current classifier by running run_training.pl.  This will automatically include the new negatives in the training data.

After any stage and after running create_cascade the output can be used as a classifier for detection code (such as the code in bindetection directory). 
Typically the first pass of this process will detect some images but miss many as well. Using the first classifier as a guide, grab images of the target which aren't detected by the current classifier. Restart the process from scratch with these additional images included in the positive_images subdir.  Remember to change the -data parameter to a new directory in prep.sh. This is to make sure that the code doesn't restart from the old classifier. 
It will usually take a number of times through the training process to get a usable classifier. Running the old classifier and watching for images which aren't detected will highlight what needs to be clipped and added to the positives for the next pass of training.

