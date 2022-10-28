module;
#include "mpi.h"
#include "hdf5.h"
export module LXJ_HDF5;

export hid_t H5CPPopen(char* file, char flag, MPI_Comm comm)
{
  /* 
    * Set up file access property list with parallel I/O access
    */
  hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
  hid_t fcpl_id = H5P_DEFAULT;
  H5Pset_fapl_mpio(fapl_id, comm, MPI_INFO_NULL);
  hid_t  file_id;   
  switch (flag)
  {
      case 'w':
          file_id = H5Fcreate(file, H5F_ACC_TRUNC, fcpl_id, fapl_id);
          break;
      case 'e':
          file_id = H5Fcreate(file, H5F_ACC_EXCL,  fcpl_id, fapl_id);
          break;
      case 'a':
          file_id = H5Fopen(file, H5F_ACC_RDWR,   fapl_id);
          break;
      case 'r':
          file_id = H5Fopen(file, H5F_ACC_RDONLY, fapl_id);
          break;
      default :
          "Error Flag :  w ,r , r+ , a, w+";
      // 创建文件时，文件访问模式指定文件已存在时要执行的操作：
      // H5F_ACC_TRUNC 指定如果文件已经存在，则删除当前内容，以便应用程序可以用新数据重写文件。
      // H5F_ACC_EXCL 指定如果文件已经存在则打开失败。如果文件不存在，则忽略文件访问参数。
      // 在任何一种情况下，应用程序都对成功创建的文件具有读写访问权限。
      // 请注意，打开现有文件有两种不同的访问模式：
      // H5F_ACC_RDONLY 指定应用程序具有读取权限，但不允许写入任何数据。
      // H5F_ACC_RDWR 指定应用程序具有读写访问权限。
  };

  H5Pclose(fapl_id);
  return file_id;
};




export herr_t H5CPPclose(hid_t file_id)
{
  herr_t status = H5Fclose(file_id);
  return status;
};

/*
 * H5CPPsave/load
 *
 * Purpose:
 *
 * MPI IO by row slab decomposition of first dim ;
 * Size is the global size of the dataset;
 * data has the local size after decomposition ;
 * rank is the ndims of the dataset;
 * 
 * Parameters:
 *    file_id,dset,data,size,rank,comm
 * 
 * Returns:
 *
 *    herr_t status
 *
 * examples:
 *
 *    import LXJ_HDF5;
      #include "mpi.h"
      #include "hdf5.h"
      import <cstdlib>;
      #include <iostream>


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
          cout<<endl;
          for (int i=0; i<10; i++) {cout<<data_out[i]<<"  ";};
          cout<<endl;


          delete(data);
          MPI_Finalize();
      }


*/
export herr_t H5CPPsave(hid_t file_id,char* dset, double* data,int* size, int rank,MPI_Comm comm)
{

  int mpi_rank,mpi_size;
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);  
  // create paralell pattern object
  hsize_t*    dimsf  = new hsize_t[rank];
  hsize_t*    count  = new hsize_t[rank];	          /* hyperslab selection parameters */
  hsize_t*    offset = new hsize_t[rank];
  hsize_t*    stride = new hsize_t[rank];
  hsize_t*    block  = new hsize_t[rank];
  for (int i = 0; i < rank; i++)    
  {
    dimsf[i] = size[i];
    stride[i] = 1;
  };
  count[0]   = dimsf[0]/mpi_size;
  offset[0]  = mpi_rank * count[0];
  for (int i = 1; i < rank; i++)    
  {
    count[i] = dimsf[i];
    offset[i] = 0;
  };

  //    * Create the dataspace for the dataset.
  hid_t filespace = H5Screate_simple(rank, dimsf, NULL);
  hid_t memspace  = H5Screate_simple(rank, count, NULL);

  // H5T_NATIVE_INT	 _FLOAT	 _CHAR	 _DOUBLE	 _LDOUBLE	
  hid_t dset_id   = H5Dcreate(file_id, dset, H5T_NATIVE_DOUBLE, filespace,H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

  // Create property list for collective dataset write.
  hid_t dxpl_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);
  herr_t status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, dxpl_id, data);
  H5Pclose(dxpl_id);

  H5Dclose(dset_id); 
  H5Sclose(memspace); 
  H5Sclose(filespace);

  delete(dimsf);
  delete(count);
  delete(offset);
  delete(stride);
  delete(block);
  return status;
};


export herr_t H5CPPload(hid_t file_id,char* dset, double* data,MPI_Comm comm)
{


  int mpi_rank,mpi_size;
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);  


  //    * Create the dataspace for the dataset.


  hid_t dset_id   = H5Dopen(file_id, dset, H5P_DEFAULT);
  hid_t filespace = H5Dget_space(dset_id);

  int rank      = H5Sget_simple_extent_ndims(filespace);
  // create paralell pattern object
  hsize_t*    dimsf  = new hsize_t[rank];
  hsize_t*    count  = new hsize_t[rank];	          /* hyperslab selection parameters */
  hsize_t*    offset = new hsize_t[rank];
  hsize_t*    stride = new hsize_t[rank];
  hsize_t*    block  = new hsize_t[rank];
  
  H5Sget_simple_extent_dims(filespace, dimsf, NULL);


  count[0]   = dimsf[0]/mpi_size;
  offset[0]  = mpi_rank * count[0];
  for (int i = 1; i < rank; i++)    
  {
    count[i] = dimsf[i];
    offset[i] = 0;
  };
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

  
  count[0]   = dimsf[0]/mpi_size;
  offset[0]  = 0;
  for (int i = 1; i < rank; i++)    
  {
    count[i] = dimsf[i];
    offset[i] = 0;
  };
  hid_t memspace  = H5Screate_simple(rank, count, NULL);
  H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset, NULL,count, NULL);
  // Create property list for collective dataset write.
  hid_t dxpl_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);
  herr_t status = H5Dread(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, dxpl_id, data);
  H5Pclose(dxpl_id);

  H5Dclose(dset_id); 
  H5Sclose(memspace); 
  H5Sclose(filespace);

  delete(dimsf);
  delete(count);
  delete(offset);
  delete(stride);
  delete(block);
  return status;
};


