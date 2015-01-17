#  ARGV[0] = ordered postives.vec from mergevec
#  ARGV[1] = input vec file in random order

open (INPUT, "mergevec/src/vec2img $ARGV[0] samples%04d.png -w 20 -h 20 |") || die "Can not open vec2img.exe pipe : $!\n";
@input_lines = <INPUT>; # output of vec2img is list of image file names
close (INPUT);

open (OUTPUT, ">random_info.dat") || die "Can not open random_info.dat : $!\n";

# grab a random file from input_lines, print it to .dat and remove it from array
while ($#input_lines >= 0 )
{
   $rand_idx = int(rand($#input_lines + 1));
   chomp($input_lines[$rand_idx]);
   print OUTPUT "$input_lines[$rand_idx] 1 0 0 20 20\n";
   splice @input_lines,$rand_idx, 1;
}

# rebuild the .vec file in random order specified by random_info.dat
`mergevec/src/createsamples -info random_info.dat -vec $ARGV[1] -w 20 -h 20`;

close (OUTPUT);

