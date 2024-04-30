#include <stdio.h>
#include <iostream>
#include <math.h>
#include <mpi.h>
#include <string>
#include <chrono>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char *argv[]){
    int rank;
    int size_proccess;
    vector<int> all_pos;
    MPI_Init(&argc, &argv);

    // get number of processors
    MPI_Comm_size(MPI_COMM_WORLD, &size_proccess);
    cerr<<"Size: "<<size_proccess<<"\n";

    // get rank of each processor
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    cerr<<"Rank: "<<rank<<"\n";

    // get the name of processor
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(processor_name, &namelen);

    cerr<<"(1)Rank: "<<rank<<" SizeProcess: "<<size_proccess<<endl;
    if (argc != 2) {
        if (rank == 0) {
            cerr << "Usage: " << argv[0] << " <input_file>\n";
        }
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    string line;
    int sz;
    auto start = chrono::high_resolution_clock::now();
    if (rank == 0) {
        ifstream file(filename);
        if (file.is_open()) {
            getline(file, line);
            sz = line.length();
            file.close();
        } else {
            cerr << "Unable to open file " << filename << endl;
            MPI_Finalize();
            return 1;
        }
        start = chrono::high_resolution_clock::now();
        all_pos.resize(sz);
        for (int i = 0; i<sz; i+=1)
            all_pos[i] = i;
    }
    cerr<<"! sz "<<rank<<": "<<sz<<endl;
    cerr<<"(2)Rank: "<<rank<<" SizeProcess: "<<size_proccess<<endl;

    MPI_Bcast(&sz, 1, MPI_INT, 0, MPI_COMM_WORLD);
    line.resize(sz);
    MPI_Bcast(&line[0], sz, MPI_CHAR, 0, MPI_COMM_WORLD);

    cerr<<"(3)Rank: "<<rank<<" SizeProcess: "<<size_proccess<<endl;

    int sub_size = (sz + size_proccess - 1) / size_proccess;
    vector<int> next_pos(sub_size, 0);
    MPI_Scatter(all_pos.data(), sub_size, MPI_INT, next_pos.data(), sub_size, MPI_INT, 0, MPI_COMM_WORLD);

    int x = max(0, sz - rank*sub_size);
    if (x < sub_size) {
        while (next_pos.size() > x)
        {
            next_pos.pop_back();
        }
    }
    cerr<<"(4)Rank: "<<rank<<" SizeProcess: "<<size_proccess<<endl;


    int ans = 0;
    vector<int> res(size_proccess, 0);
    for (int i=0; i<next_pos.size(); i+=1)
    {
        int l = next_pos[i];
        int j = 0;
        while (l - j >= 0 and l + j < sz and line[l - j] == line[l + j]) {
            ans += 1;
            j += 1;
        }
        j = 0;
        while (l - j >= 0 and l + j + 1 < sz and line[l - j] == line[l + j + 1]) {
            ans += 1;
            j += 1;
        }
    }
    cerr<<rank<<" "<<"Result: "<<ans<<"\n";
    MPI_Gather(&ans, 1, MPI_INT, res.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        long long result = 0;
        for (int i = 0; i < size_proccess; ++i){
            result += res[i];
        }

        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
        cout << "RESULT :: " << result << "\n";
        cout << "DURATION :: " << duration.count() << " ms\n";
    }

    MPI_Finalize();
}