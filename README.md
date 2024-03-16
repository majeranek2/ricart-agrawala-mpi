# ricart-agrawala-mpi
## My implementation of the DME Ricart-Agrawala algorithm in C++

The Ricart-Agrawala algorithm is a distributed algorithm used to achieve mutual exclusion in a distributed system, allowing processes to access a critical section of code without interference from other processes. Ricart-Agrawala, unlike many traditional mutual exclusion algorithms, relies on message passing to coordinate access to the critical section.

My implementation is based on MPI (Message Passing Interface) where I use synchronization mechanisms in order to divide the program execution in three phases:
  1. Each process broadcasts its timestamp to every other process.
  2. Each process receives and stores the requests.
  3. Each process waits until it can access the CS.

### How to run the program:
  1. Install MPI and configure your IDE ([for Visual Studio click here](https://www.youtube.com/watch?v=2tcJWpuD8wQ))
  2. Prepare your input file containing the timestamps (example file provided)
  3. Compile the program for an exe file
  4. In terminal, run the program with ```mpiexec -n N programname.exe```, where N = number of processes - make sure N equals the number of timestamps

_Word of context: It might happen that the messages printed on the screen will not reflect the order in which the processes enter the critical section. Keeping track of the resource value is highly suggested here. Resource equal to the number of sites shown on every process indicates that the algorithm is safe._
