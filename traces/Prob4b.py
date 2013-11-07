#Python program go generate desired input files.

# Memory Bandwidth 
#Constant parameters :
#-------------------------------------------
#  Cache Size = 8192B, 16384B
#  Block Size = 64B, 128B
#  Associativity = 2, 4
#  Writeback 
#--------------------------------------------

Files = ["spice.trace", "cc.trace", "tex.trace"];
OutFiles = ["P1.spice.test", "P1.cc.test", "P1.tex.test"];
trace_dir = "./traces/";

#Problem 2 
block_size_arr = [64, 128];
cache_size_arr = [8192, 16384];
associativity_arr = [2, 4];


for fname in Files:
   print '------------------------------------------------------------'
   for cache_size in cache_size_arr:
      for block_size in block_size_arr:
         for associativity in associativity_arr:
            print './sim -is {} -ds {} -bs {} -a {} {}{}'.format( cache_size, cache_size, block_size, associativity, trace_dir, fname);
            print './sim -is {} -ds {} -bs {} -a {} -nw {}{}'.format( cache_size, cache_size, block_size, associativity, trace_dir, fname);
