sub4n   lw          0       6       pos1        
        sw          5       7       Stack       
        add         5       6       5           
        sw          5       1       Stack       
        add         5       6       5           
        add         1       1       1           
        add         1       1       3           
        lw          0       6       neg1        
        add         5       6       5           
        lw          5       1       Stack       
        add         5       6       5           
        lw          5       7       Stack       
        jalr        7       4                   
pos1    .fill       1
neg2    .fill       -2
neg1    .fill       neg2
SubAdr  .fill       sub4n