#Python program go generate desired input files.

#Hit rate vs Cache_block size 
#Constant parameters :
#-------------------------------------------
#  Cache Size = 8192
#  Associativity = 2 
#--------------------------------------------

Files = ["spice.trace", "cc.trace", "tex.trace"];
OutFiles = ["P1.spice.test", "P1.cc.test", "P1.tex.test"];
trace_dir = "./traces/";

#Problem 2 
base_block_size = 4;
cache_size = 8192;
associativity = 2;
n = range(0,11);

for fname in Files:
   print '------------------------------------------------------------'
   for i in n:
      block_size = (2**i)*base_block_size;
      print './sim -is {} -ds {} -bs {} -a {} {}{}'.format( cache_size, cache_size, block_size, associativity, trace_dir, fname);
