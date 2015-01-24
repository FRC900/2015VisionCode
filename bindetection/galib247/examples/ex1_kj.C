/* ----------------------------------------------------------------------------
  ex1.C
  mbwall 28jul94
  Copyright (c) 1995-1996  Massachusetts Institute of Technology

 DESCRIPTION:
   Example program for the SimpleGA class and 2DBinaryStringGenome class.

   The each 1D array of binary values represents an integer value
   The full 2D array represents an array of those integers

   The program tries to optimize the values of the array of integers
   so that the sum of the squares of these values equals 125720.
  This example uses the default crossover (single point), default mutator
(uniform random bit flip), and default initializer (uniform random) for the
2D genome.
  Notice that one-point crossover is not necessarily the best kind of crossover
to use if you want to generate a 'good' genome with this kind of objective 
function.  But it does work.
---------------------------------------------------------------------------- */
#include <ga/GASimpleGA.h>      // we're going to use the simple GA
#include <ga/GA2DBinStrGenome.h> // and the 2D binary string genome
#include <ga/std_stream.h>

#include <math.h>

using namespace std;

#define cout STD_COUT

float Objective(GAGenome &);    // This is the declaration of our obj function.
                                // The definition comes later in the file.

// Convert a given 1D row of a binary string into an integer value
unsigned int getIntFrom2DBinaryString(const GA2DBinaryStringGenome &genome, unsigned int row)
{
     unsigned int intval = 0;
     //cout << genome << endl;
     for(int i=0; i<genome.width(); i++){
        intval <<= 1;
        //cout << i << " " << intval << " " << (genome.gene(i, row) & 1) << endl;
        intval  |= genome.gene(i, row) & 1;
     }
     return intval;
}

// Finish the run when we get an exact match (score == 1.0)
GABoolean terminator(GAGeneticAlgorithm &ga)
{
 return (fabs(ga.statistics().bestIndividual().score() - 1.0) < 0.00000001) ? gaTrue : gaFalse ;
}

int
main(int argc, char **argv)
{
  cout << "Example 1" << endl << endl;
  cout << "This program tries to fill a 2DBinaryStringGenome with" << endl;
  cout << "integer patterns which sum up to 125720" << endl; cout.flush();

// See if we've been given a seed to use (for testing purposes).  When you
// specify a random seed, the evolution will be exactly the same each time
// you use that seed number.

  for(int ii=1; ii<argc; ii++) {
    if(strcmp(argv[ii++],"seed") == 0) {
      GARandomSeed((unsigned int)atoi(argv[ii]));
    }
  }

// Declare variables for the GA parameters and set them to some default values.

  int width    = 8; // 8 bit values == integers between 0-255 inclusive
  int height   = 5; // use 5 of these 8-bit values
  int popsize  = 50;
  int ngen     = 10000; // Number of generations to run
  float pmut   = 0.01;
  float pcross = 0.8;

// Now create the GA and run it.  First we create a genome of the type that
// we want to use in the GA.  The ga doesn't operate on this genome in the
// optimization - it just uses it to clone a population of genomes.

  GA2DBinaryStringGenome genome(width, height, Objective);

// Now that we have the genome, we create the genetic algorithm and set
// its parameters - number of generations, mutation probability, and crossover
// probability.  And finally we tell it to evolve itself.

  GASimpleGA ga(genome);
  ga.populationSize(popsize);
  ga.nGenerations(ngen);
  ga.pMutation(pmut);
  ga.pCrossover(pcross);
  ga.scoreFilename("bog.dat");
  ga.flushFrequency(20);
  ga.terminator(terminator); // Set up terminator to stop on an exact match
  ga.evolve();

// Now we print out the best genome that the GA found.

  genome = ga.statistics().bestIndividual();
  cout << "The GA found:" << endl;
  double accum = 0;
  for (int i = 0; i < genome.height(); i++)
  {
     unsigned int intval = getIntFrom2DBinaryString(genome, i);
     accum += intval * intval;

     cout << "x" << i << " : " << intval << endl;
  }
  cout << accum << endl;

  // That's it!
  return 0;
}


// This is the objective function.  It takes a genome as input. This genome is 
// just a collection of values that are being tested.  It returns a float value representing 
// how "good" the particular genome is - higher values mean that the numbers represented by
// the genome are closer to the idea solution.  In this simple test case, the code is 
// just trying to solve a simple equation : find 5 values such that the sum of the squares 
// of numbers equals 125720.
//
// The genome stores integer values as a 2-D array of bits.  Each 1-D sub-array of bits 
// is a single integer value. I've used 8bits for each int since that'll hold
// the range of most of the values being searched (0-255)
//
// Each integer would represent one value being tested - H_MAX, H_MIN and so on.
// Instead of using a simple math, you'd pass these integers into a function which
// does the OpenCV calls and use that result to figure out how good your solution is.
float
Objective(GAGenome& g) {
  GA2DBinaryStringGenome & genome = (GA2DBinaryStringGenome &)g;
  double accum = 0.0;
  unsigned int intval;

  // Each height entry is an integer - convert it from binary
  // to int, square it, add it to the accumulated value.
  for(int i=0; i<genome.height(); i++){
     intval = getIntFrom2DBinaryString(genome, i);
     accum += intval * intval;
  }
  // Want values closest to 125720 to have the largest goodness value, so use the 
  // inverse of the difference between the actual and desired values
  return 1.0 / (1.0 + fabs(accum - 125720.0));
}
