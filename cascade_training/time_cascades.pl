#! /usr/bin/perl

use Storable;

my $save_fn = "saved_cascade_time.txt";

opendir (DIR, "positive_images") || die "Can not open positive_images : $!";
@pngs = grep(/\.png$/,readdir(DIR));
closedir (DIR);

my %png_hash;

# read old results
if (-f $save_fn)
{
   %png_hash = %{retrieve($save_fn)};
}
#foreach $file (keys %png_hash)
#{
#   @times = split /,/,$png_hash{$file};
#   if ($times[0] == "1")
#   {
#      shift @times;
#   }
#   $png_hash{$file} = join ',',@times;
#   print "$file : $png_hash{$file}\n";
#}
#   store \%png_hash, $save_fn;
#   die;

while (1)
{
   my %pngs_to_use = undef;
   open (POSITIVES, ">positives.dat") || die "Can not open positives.dat : $!";
   open (VECS, ">vectors.dat") || die "Can not open vectors.dat : $!";;
   foreach $file (@pngs)
   {
      if (rand(1) > 0.5)
      {
	 $pngs_to_use{$file} = 1 ;
	 print POSITIVES "positive_images/$file\n";
	 print VECS      "./$file.vec\n";
      }
   }
   close POSITIVES;
   close VECS;
   print "Creating sample .vecs\n";
   readpipe("rm *.vec");
   readpipe("perl createtrainsamples.pl positives.dat negatives.dat . 12000"); 
   readpipe("rm positives.vec ordered_positives.vec");
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
   #
   print "Testing detection speed\n";
   open (FPSTEST, "../bindetection/fpstest |");
   $fps = <FPSTEST>;
   chomp $fps;
   close (FPSTEST);
   print "$fps FPS\n";

   foreach $file (keys %pngs_to_use)
   {
      $png_hash{$file} .= "," if defined ($png_hash{$file});
      $png_hash{$file} .= $fps;
   }
   store \%png_hash, $save_fn;
}
