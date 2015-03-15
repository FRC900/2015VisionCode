#! /usr/bin/perl

my $best_fps = 0;

if (open(FPS_FILE, "classifier_bin_save/best_fps.txt"))
{
   $line = <FPS_FILE>;
   $best_fps = $1 if ($line =~ /([0-9]*\.?[0-9]+)/);
   close FPS_FILE;
}
print "Previous best FPS = $best_fps\n";

readpipe("/bin/find positive_images -name \*.png > positives.dat");
while (1)
{
   print "Creating negatives\n";
   readpipe("/bin/find negative_images -name \*.png > negatives.dat");
   readpipe("shuf negatives.dat > temp.dat");
   readpipe("mv temp.dat negatives.dat");

   print "Creating sample .vecs\n";
   readpipe("rm *.vec");
   readpipe("perl createtrainsamples.pl positives.dat negatives.dat . 12000"); 
   readpipe("/bin/find . -name \\*.vec > vectors.dat");
   readpipe("mergevec/src/mergevec.exe vectors.dat ordered_positives.vec");
   readpipe("mergevec/src/vec2img ordered_positives.vec samples%04d.png -w 20 -h 20 | shuf > info.dat");
   readpipe('sed \'s/$/ 1 0 0 20 20/\' info.dat > random_info.dat');

   print "Creating positives.vec\n";
   readpipe( "mergevec/src/createsamples -info random_info.dat -vec positives.vec -num \`wc -l random_info.dat\` -w 20 -h 20");
   readpipe ("rm samples????.png samples[0-9]????.png");
   readpipe("mkdir -p classifier_bin_x");
   readpipe("rm -f classifier_bin_x/*");

   print "Running classifier training\n";
   readpipe ("/bin/opencv_traincascade -data classifier_bin_x -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 15 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 11250 -numNeg 7000 -featureType LBP -precalcValBufSize 1750 -precalcIdxBufSize 1750 -maxWeakCount 1000");

   # Run classifier on sample video
   print "Testing detection speed\n";
   open (FPSTEST, "../bindetection/fpstest |");
   $fps = <FPSTEST>;
   chomp $fps;
   close (FPSTEST);
   print "$fps FPS\n";
   if ($fps > $best_fps)
   {
      $best_fps = $fps;
      print "New best FPS\n";
      readpipe("mkdir -p classifier_bin_save");
      readpipe("rm -f classifier_bin_save/*");
      readpipe("cp classifier_bin_x/* classifier_bin_save");
      readpipe("mv negatives.dat classifier_bin_save");
      readpipe("mv positives.vec classifier_bin_save/positives.vec~");
      readpipe("echo $best_fps > classifier_bin_save/best_fps.txt");
   }
}
