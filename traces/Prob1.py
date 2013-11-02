#Python program go generate desired input files.

#Hit rate vs Cache size function.
#Constant parameters :
#-------------------------------------------
#  Block Size = 4B
#  Associativity = always fully associative
#--------------------------------------------

Files = ["spice.trace", "cc.trace", "tex.trace"];
OutFiles = ["P1.spice.test", "P1.cc.test", "P1.tex.test"];
trace_dir = "./traces/";

#Problem 1
block_size = 4;
n = range(2,20);

for fname in Files:
   for i in n:
      cache_size = 2**i;
      associativity = cache_size/block_size;
      print './sim -is {} -ds {} -bs {} -a {} {}{}'.format( cache_size, cache_size, block_size, associativity, trace_dir, fname);
