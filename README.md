# project-2-mpi-michael, Ilui and-bijaya
project-2-mpi-michael-Ilui and-bijaya created by GitHub Classroom


Michael Burdi   maburdi@csu.fullerton.edu
Bijaya Shrestha sthavjay@csu.fullerton.edu
Giuliana Pham   gpham@csu.fullerton.edu


Command to compile and run the program:

Use 3 processes to determined 5 clusters in the data (default: input.txt)
```
  mpicc main.c
  mpirun -np 3 ./a.out 5
```

Use 5 processes to determined 8 clusters in the data (default: input.txt)
```
  mpicc main.c
  mpirun -np 5 ./a.out 8
```

Using 5 processes to determine 5 clusters with the data from the input2.txt file.
```
mpirun -np 3 ./a.out 5 input2.txt 
```


The argument to the program is the number of clusters to calculate. Each process will calculate its own set of clusters as if it was the only process. The data will be split among the processes and each will work independently. Afterward, the processes will share what they gathered and compute a final set of clusters from each of their findings.
  
  
## Example test
```
mpirun -np 3 ./a.out 5 input2.txt 

Number of clusters: 5
Process 0 count = 1666
Process 1 count = 1667
Process 2 count = 1667
P0,0    <64,-72,71>
P0,1    <0,-1,-41>
P0,2    <-96,-9,-86>
P0,3    <43,39,61>
P0,4    <-52,9,77>
J' = 21615701012.000000
J = 21591658784.000000
Threshold = 0.100000

P1,0    <30,-34,-33>
P1,1    <0,-1,0>
P1,2    <-26,12,19>
P1,3    <-99,-64,-24>
P2,0    <6,43,-54>
P2,1    <0,0,1>
P2,2    <-65,-41,-57>
P2,3    <-28,73,12>
P2,4    <85,61,-33>
J' = 152578688.000000
J = 135346785.000000
Threshold = 0.100000

P1,4    <-86,35,-18>
J' = 149781022.000000
J = 126784451.000000
Threshold = 0.100000

P0,0    <64,-72,71>
P0,1    <0,-1,-41>
P0,2    <-96,-9,-86>
P0,3    <43,39,61>
P0,4    <-52,9,77>
J' = 21591658784.000000
J = 21591658784.000000
Threshold = 0.100000

P2,0    <6,43,-54>
P2,1    <0,0,1>
P2,2    <-65,-41,-57>
P2,3    <-28,73,12>
P2,4    <85,61,-33>
J' = 135346785.000000
J = 135346785.000000
Threshold = 0.100000

P1,0    <30,-34,-33>
P1,1    <0,-1,0>
P1,2    <-26,12,19>
P1,3    <-99,-64,-24>
P1,4    <-86,35,-18>
J' = 126784451.000000
J = 126784451.000000
Threshold = 0.100000

Process 0 centroids
<64,-72,71>
<0,-1,-41>
<-96,-9,-86>
<43,39,61>
<-52,9,77>

Process 1 centroids
<30,-34,-33>
<0,-1,0>
<-26,12,19>
<-99,-64,-24>
<-86,35,-18>

Process 2 centroids
<6,43,-54>
<0,0,1>
<-65,-41,-57>
<-28,73,12>
<85,61,-33>
```
