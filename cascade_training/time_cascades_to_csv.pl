#! /usr/bin/perl

use Storable;

my $save_fn = "saved_cascade_time.txt";
my %png_hash = %{retrieve($save_fn)};

foreach $file (sort keys %png_hash)
{
   print "$file,$png_hash{$file}\n";
}
