#Python program go generate desired input files.

#Hit rate vs Cache_block size 
#Constant parameters :
#-------------------------------------------
#  Cache Size = 8192B
#  Block Size = 128B
#--------------------------------------------

Files = ["spice.trace", "cc.trace", "tex.trace"];
OutFiles = ["P1.spice.test", "P1.cc.test", "P1.tex.test"];
trace_dir = "./traces/";

#Problem 2 
block_size = 128;
cache_size = 8192;
asso = range(0,7);

for fname in Files:
   print '------------------------------------------------------------'
   for a in asso:
      associativity = (2**a)
      print './sim -is {} -ds {} -bs {} -a {} {}{}'.format( cache_size, cache_size, block_size, associativity, trace_dir, fname);
