#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <windows.h>

int resource = 0;   // resource - keeps track of how many times the CS has been accessed. Must be equal to size

// Function to check if vector contains at least one false value
bool isFalse(std::vector<bool> v) {  
    return std::any_of(v.begin(), v.end(), [](bool val) { return !val; });
}

// Function to perform Ricart-Agrawala algorithm
void ricart_agrawala(int size, int rank,  std::vector <int> timestamp, std::vector<bool> permissions) {
    MPI_Status status;
    permissions[rank] = true;   // Set own permission to true

    std::cout << "Process " << rank << " is going to request CS now." << std::endl;
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(&timestamp[rank], 1, MPI_INT, i, 0, MPI_COMM_WORLD);       // send your own timestamp to the process with rank = i
        }
    }
    std::cout << "Process " << rank << " broadcasted a request with the timestamp " << timestamp[rank] << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);    // barrier - essential mechanism here - we proceed further only when all the requests have been broadcast

    std::vector <std::pair<int, int>> received;
    int received_timestamp;
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Recv(&received_timestamp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Process " << rank << " received a request with a timestamp " << received_timestamp << " from process " << i << std::endl;
            received.emplace_back(received_timestamp, i);
        }   
    }

    MPI_Barrier(MPI_COMM_WORLD);    // proceed further only when all the requests have been received

    std::cout << "Process " << rank << " received in total - (timestamp, source): ";
    for (int i = 0; i < size-1; i++) {
        std::cout << "(" << received[i].first << ", " << received[i].second << ") ";
        if (i == size - 2) std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    std::queue<std::pair<int, int>> Q;
    int message = 1;
    int flag;

    for (const auto& pair : received) {     // compare your timestamp with each received request
        if (timestamp[rank] < pair.first) {
            Q.push(pair);
        }
        else {
            MPI_Send(&message, 1, MPI_INT, pair.second, 0, MPI_COMM_WORLD);   
            std::cout << "Process " << rank << " sending the permission to the process " << pair.second << std::endl;
        }
    }

    int received_message;
    while (isFalse(permissions)) {  // wait until you're allowed to access CS
        while (true){
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);    // check whether there is anything to receive
            if (flag) {
                MPI_Recv(&received_message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                permissions[status.MPI_SOURCE] = true;
                std::cout << "Process " << rank << " received the permission from the process " << status.MPI_SOURCE << std::endl;
            }
        else break;
    }
    
    }
    std::cout << "Process " << rank << " is in the CS." << std::endl;
    resource++;
    Sleep(1500);    // simulate some work 
    std::cout << "Process " << rank << " has left the CS. Resource has been incremented." << std::endl;
    while (!Q.empty()) {
        std::pair<int, int> element = Q.front();
        MPI_Send(&message, 1, MPI_INT,element.second, 0, MPI_COMM_WORLD);       // send the permission to each node from your local queue
        Q.pop();
    }
}

int main(int argc, char** argv) {
    std::cout << "Resource value in the beginning: " << resource << std::endl;
    MPI_Init(NULL, NULL);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector <int> timestamp(size, 0);
    std::vector<bool> permissions(size, false); 

    std::ifstream file("input.txt");
    if (!file) {
        std::cerr << "ERROR: Cannot open the txt file.\n";
        MPI_Abort(MPI_COMM_WORLD, 0);
    }

    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) 
        lineCount++;
    
    if (lineCount != size) {
        std::cerr << "ERROR: Amount of timestamps must be equal to the number of processes running." <<
            "If you encounter this error, please make sure to run the program in the terminal, using mpiexec -n [number of processes] file.exe\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < size; i++) {
        file >> timestamp[i];
        if (timestamp[i] < 0) {
            std::cerr << "ERROR: Negative value found in the file.\n";
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
    }

    ricart_agrawala(size, rank, timestamp, permissions);

    // Use MPI_Allreduce to sum up the resource across all processes
    int total_resource = 0;
    MPI_Allreduce(&resource, &total_resource, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    std::cout << "Resource value after the execution: " << total_resource << std::endl;
    
    MPI_Finalize();
}
