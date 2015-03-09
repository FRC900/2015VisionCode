while (1)
{
   my $pid = open(PIPE, "/bin/opencv_traincascade -data classifier_bin_4 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 55 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 11250 -numNeg 8000 -precalcValBufSize 3250 -precalcIdxBufSize 3250 -maxWeakCount 1000 |");
   while ($line = <PIPE>)
   {
      print $line;
      if ($line =~ /\|\s+(\d+)\|\s+([0-9\.]+)\|\s+([0-9\.]+\|)/)
      {
	 if ($1 eq "20")
	 {
	    `bash redo_negatives.sh`;
	 }
	 if ((($1 > 50) && ($3 eq "1")) || 
	     (($1 > 150) && ($3 > 0.99)))
	 {
	    print "Killing\n";
	    kill $pid;
	    close (PIPE);
	 }
      }
   }
}
