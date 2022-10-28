import LXJ_HDF5;
#include "mpi.h"
#include "hdf5.h"
import <cstdlib>;
import <iostream>;

using namespace std;

int main(int argc, char **argv)
{
    
    MPI_Comm comm  = MPI_COMM_WORLD;
    MPI_Init(&argc, &argv);
    int mpi_rank,mpi_size;
    MPI_Comm_rank(comm, &mpi_rank);
    MPI_Comm_size(comm, &mpi_size);  



    double* data = new double[10];
    
    for (int i=0; i < 10; i++) {
        data[i] = mpi_rank + i;
    };
    char* file = "SDS_row.h5";
    char* dset = "IntArray";
    char* group_name = "TestGroup";
    int size[2] {8,5};
    int rank {2};

    hid_t  file_id  =  H5CPPopen(file,'w',comm);
    hid_t  group_id = H5Gcreate(file_id,group_name,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
    H5CPPsave(group_id,dset,data,size,rank,comm);
    H5Gclose(group_id);
    H5CPPclose(file_id);

    double* data_out = new double[10];
    
    for (int i=0; i < 10; i++) {
        data[i] = mpi_rank + i;
    };
    file_id  =  H5CPPopen(file,'r',comm);
    group_id = H5Gopen(file_id,group_name,H5P_DEFAULT);
    H5CPPload(group_id,dset,data_out,comm);
    H5Gclose(group_id);
    H5CPPclose(file_id);


    for (int i=0; i<10; i++) {cout<<data[i]<<"  ";};
    cout<<"\t>> IN "<<endl;
    for (int i=0; i<10; i++) {cout<<data_out[i]<<"  ";};
    cout<<"\t<< OUT "<<endl;


    delete(data);
    MPI_Finalize();
}
