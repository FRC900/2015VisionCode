while (1)
{
   my $pid = open(PIPE, "/bin/opencv_traincascade -data classifier_bin_12 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 55 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 8000 -numNeg 5000 -featureType LBP -precalcValBufSize 1750 -precalcIdxBufSize 1750 -maxWeakCount 1000 |");
   while ($line = <PIPE>)
   {
      print $line;
      if ($line =~ /\|\s+(\d+)\|\s+([0-9\.]+)\|\s+([0-9\.]+\|)/)
      {
	 if ($1 eq "5")
	 {
	    `bash redo_negatives.sh`;
	 }
	 if ((($1 > 25) && ($2 eq "1")) || 
	     (($1 > 75) && ($3 > 0.99)))
	 {
	    print "Killing\n";
	    kill $pid;
	    close (PIPE);
	 }
      }
   }
}
