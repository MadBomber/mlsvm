#include "common_funcs.h"
#include "config_logs.h"
#include <algorithm>    /* random_shuffle*/
#include <cmath>

#include <memory>   /* shared_ptr*/
//#include <stdexcept>

/*
 * Export matrix to a file
 */
void CommonFuncs::exp_matrix(Mat& A, std::string file_path, std::string file_name){
    exp_matrix(A,file_path,file_name,"N/A");
}

void CommonFuncs::exp_matrix(Mat& A, std::string file_path, std::string file_name, std::string sender_func){
#if debug_export == 1
    PetscViewer     viewer;
    std::string     full_file = (file_path == "")? file_name : file_path +"/" +file_name;   //if the file_path is empty, don't add /
    PetscViewerBinaryOpen(PETSC_COMM_WORLD,full_file.c_str(),FILE_MODE_WRITE,&viewer);
    MatView(A,viewer);  // petsc binary format

    PetscViewerDestroy(&viewer);        //destroy the viewer
    std::cout<< "[CF][exp_matrix] {"<< sender_func <<"} Matrix exported to file: "<< full_file << std::endl;
#endif
#if debug_export == 0
    std::cout<< "The export functionality is disabled in the config_log.h. You can modify and recompile the code in case you need it." << std::endl;
#endif
}

void CommonFuncs::exp_vector(Vec& A, std::string file_path, std::string file_name){
    exp_vector(A,file_path,file_name,"N/A");
}

void CommonFuncs::exp_vector(Vec& A, std::string file_path, std::string file_name, std::string sender_func){
#if debug_export == 1
    PetscViewer     viewer;
    std::string     full_file = (file_path == "")? file_name : file_path +"/" +file_name;   //if the file_path is empty, don't add /
    PetscViewerBinaryOpen(PETSC_COMM_WORLD,full_file.c_str(),FILE_MODE_WRITE,&viewer);
    VecView(A,viewer);  // petsc binary format

    PetscViewerDestroy(&viewer);        //destroy the viewer
    std::cout<< "[CF][exp_vector] {"<< sender_func <<"} Vector exported to file: "<< full_file << std::endl;
#endif
#if debug_export == 0
    std::cout<< "The export functionality is disabled in the config_log.h. You can modify and recompile the code in case you need it." << std::endl;
#endif
}

/*
 * Gets 2 vector and calculate the Eucidean distance
 */
double CommonFuncs::calc_euclidean_dist(const PetscInt ncols_A, const PetscInt ncols_B,
                                        const PetscInt *cols_A, const PetscInt *cols_B,
                                        const PetscScalar *vals_A, const PetscScalar *vals_B){
    PetscInt            it_p=0, it_n=0;
    double              current_distance=0;

    while( it_p < ncols_A || it_n < ncols_B ){
        if(it_p == ncols_A){                                    // we reached to end of A
            while(it_n < ncols_B){                              // add the rest from B
                current_distance += pow(vals_B[it_n], 2 );
                it_n++;
            }                                                   // B is finished too
            continue;                       //  skip the rest since we are done completely
        }

        if(it_n == ncols_B){                                    // we reached to end of B
            while(it_p < ncols_A){                              // add the rest from A
                current_distance += pow(vals_A[it_p], 2 );
                it_p++;
            }                                                   // A is finished too
            continue;                       //  skip the rest since we are done completely
        }

        // - - - - - neither P or N have not finished yet - - - - -
        if(cols_A[it_p] == cols_B[it_n]){                       // Common index on both A, B
            current_distance += pow(  (vals_A[it_p] - vals_B[it_n]) , 2 );  // calc their difference
            it_p++; it_n++;
            continue;
        }
        if(cols_A[it_p] < cols_B[it_n] ){                       // A has some values while B has nothing
            current_distance += pow(vals_A[it_p], 2 );
            it_p++;
            continue;
        }else{                                                  // B has some values while A has nothing
            current_distance += pow(vals_B[it_n], 2 );
            it_n++;
            continue;
        }
    }
    return sqrt(current_distance);
}

/*
 * Gets 2 vector and calculate the Manhatan distance
 */
double CommonFuncs::calc_manhatan_dist(const PetscInt ncols_A, const PetscInt ncols_B,
                                        const PetscInt *cols_A, const PetscInt *cols_B,
                                        const PetscScalar *vals_A, const PetscScalar *vals_B){
    PetscInt            it_p=0, it_n=0;
    double              current_distance=0;

    while( it_p < ncols_A || it_n < ncols_B ){
        if(it_p == ncols_A){                                    // we reached to end of A
            while(it_n < ncols_B){                              // add the rest from B
                current_distance += fabs(vals_B[it_n]);
                it_n++;
            }                                                   // B is finished too
            continue;                       //  skip the rest since we are done completely
        }

        if(it_n == ncols_B){                                    // we reached to end of B
            while(it_p < ncols_A){                              // add the rest from A
                current_distance += fabs(vals_A[it_p]);
                it_p++;
            }                                                   // A is finished too
            continue;                       //  skip the rest since we are done completely
        }

        // - - - - - neither P or N have not finished yet - - - - -
        if(cols_A[it_p] == cols_B[it_n]){                       // Common index on both A, B
            current_distance += fabs(  (vals_A[it_p] - vals_B[it_n]) );  // calc their difference
            it_p++; it_n++;
            continue;
        }
        if(cols_A[it_p] < cols_B[it_n] ){                       // A has some values while B has nothing
            current_distance += fabs(vals_A[it_p]);
            it_p++;
            continue;
        }else{                                                  // B has some values while A has nothing
            current_distance += fabs(vals_B[it_n]);
            it_n++;
            continue;
        }
    }
    return current_distance;
}


void CommonFuncs::set_weight_type(int new_weight_type, double aux_param){
    this->weight_type = new_weight_type;
    if(new_weight_type == 2){ // for guassian distance, it reads the gamma from param.xml
        this->weight_gamma = aux_param;//Config_params::getInstance()->get_ld_weight_param();
    }
}

double CommonFuncs::convert_distance_to_weight(double distance){
    switch (weight_type) {
    case 1:                 //Flann default distance (square Euclidean distance)
//        printf("[LD][CD] raw distance:%g, calculated distance:%g \n",distance, 1 / (sqrt(distance) + 0.00001 ));
        return ( 1 / (sqrt(distance) + 0.00001 ) );    //
    case 2:                 // Gaussian distance)
        return ( exp((-1) * distance * weight_gamma)  );
    }
}



/*
 * calculate the dot product of vector A and B
 * vector A is a row of a matrix
 * vector B is standard library vector
 *
 * output: PetscScalar value
 */
PetscScalar CommonFuncs::vec_vec_dot_product(const PetscInt ncols_A, const PetscInt *cols_A,
                                        const PetscScalar *vals_A, const std::vector<double>& v_B){
    PetscInt            it_A ;
    PetscScalar         result=0;

    for(it_A=0; it_A < ncols_A; it_A++){
        // for v_B, the value in the corresponding index to vector A is needed
//        std::cout << "[CF][V.V] vals_A[it_A]:" << vals_A[it_A] << ", v_B[cols_A[it_A]]:" << v_B[cols_A[it_A]] << std::endl;
        result += vals_A[it_A] * v_B[ cols_A[it_A] ];
    }
//    std::cout << "[CF][V.V] result:" << result << std::endl;
    return result;
}




template <typename T>
T CommonFuncs::sum_vector(const std::vector<T>& vec_In){
    T total=0;
    for(unsigned i=0; i < vec_In.size(); ++i){
        total += vec_In[i];
    }
    std::cout << "total:" << total << std::endl;
    return total;
}





template <typename T>
PetscScalar CommonFuncs::mean_vector(const std::vector<T>& vec_In){
    return ( sum_vector(vec_In) / (float) vec_In.size());
}


//template <class T>
//PetscScalar CommonFuncs::STD_array(T *arrayIn, unsigned int array_size, PetscScalar& mean){
//    mean = mean_array(arrayIn, array_size);
//    PetscScalar sum_difference=0, final=0;
//    for(unsigned i=0; i < array_size; ++i){
//        sum_difference += pow(arrayIn[i] - mean, 2);
//    }
//    final = sqrt(sum_difference / (float) array_size);
//    return final;
//}


//template <class T>
//void CommonFuncs::zscore_array(T *arrayIn, unsigned int array_size){
//    PetscScalar std, mean;
//    std = STD_array(arrayIn, array_size, mean);
//    for(unsigned i=0; i < array_size; ++i){
//        arrayIn[i] = (arrayIn[i] - mean) / std;
//    }
//}













//void make_full_vector(PetscInt num_features, PetscInt ncols, const PetscInt *cols, const PetscScalar *vals, PetscScalar * arr_output ){
//    int i=0, j=0;
//    for(j=0; j < ncols; j++){
//        while(i < cols[j]){   //fill zeros
//            printf("inside make_full_vector: 1st while i:%d\n",i);
//            arr_output[i]=0;
//            i++;
//        }
//        if(i == cols[j]){     //fill a value
//            printf("inside make_full_vector: i:%d, j:%d, vals[j]:%g\n",i,j, vals[j]);
//            arr_output[i]= vals[j];
//            i++;
//        }
//    }
//    while(i < num_features){    //fill the rest of zeros
//        printf("inside make_full_vector: 2nd while i:%d\n",i);
//        arr_output[i]=0;
//        i++;
//    }
//}


template <typename T>
T CommonFuncs::sum_array(T *arrayIn, unsigned int array_size){
    T total=0;
    for(unsigned i=0; i < array_size; ++i){
        total += arrayIn[i];
    }
    std::cout << "sum array:" << total << std::endl;
    return total;
}



template <typename T>
PetscScalar CommonFuncs::mean_array(T *arrayIn, unsigned int array_size){
    return ( sum_array(arrayIn, array_size) / (float) array_size);
}


template <class T>
PetscScalar CommonFuncs::STD_array(T *arrayIn, unsigned int array_size, PetscScalar& mean){
    mean = mean_array(arrayIn, array_size);
    PetscScalar sum_difference=0, final=0;
    for(unsigned i=0; i < array_size; ++i){
        sum_difference += pow(arrayIn[i] - mean, 2);
    }
    final = sqrt(sum_difference / (float) array_size);
    return final;
}


template <class T>
void CommonFuncs::zscore_array(T *arrayIn, unsigned int array_size){
    PetscScalar d_std, d_mean;
    d_std = STD_array(arrayIn, array_size, d_mean);
    for(unsigned i=0; i < array_size; ++i){
        arrayIn[i] = (arrayIn[i] - d_mean) / d_std;
    }
}


template int CommonFuncs::sum_vector(const std::vector<int>& vec_In);
template float CommonFuncs::sum_vector(const std::vector<float>& vec_In);
template PetscScalar CommonFuncs::sum_vector(const std::vector<PetscScalar>& vec_In);




template int CommonFuncs::sum_array(int *arrayIn, unsigned int array_size);
template float CommonFuncs::sum_array(float *arrayIn, unsigned int array_size);
template PetscScalar CommonFuncs::sum_array(PetscScalar *arrayIn, unsigned int array_size);

template PetscScalar CommonFuncs::mean_array(int *arrayIn, unsigned int array_size);
template PetscScalar CommonFuncs::mean_array(float *arrayIn, unsigned int array_size);
template PetscScalar CommonFuncs::mean_array(PetscScalar *arrayIn, unsigned int array_size);

template PetscScalar CommonFuncs::STD_array(int *arrayIn, unsigned int array_size, PetscScalar& mean);
template PetscScalar CommonFuncs::STD_array(float *arrayIn, unsigned int array_size, PetscScalar& mean);
template PetscScalar CommonFuncs::STD_array(PetscScalar *arrayIn, unsigned int array_size, PetscScalar& mean);

template void CommonFuncs::zscore_array(int *arrayIn, unsigned int array_size);
template void CommonFuncs::zscore_array(float *arrayIn, unsigned int array_size);
template void CommonFuncs::zscore_array(PetscScalar *arrayIn, unsigned int array_size);


/*
 * Get the size of the data from local variables (num_mXX_points) which are set during the divide_data
 * Create a random sequence of numbers for them and store them in local vectors (mXX_shuffled_indices_)
 * The seed for srand comes from the config params and it is recorded in output for debug
 */
Mat CommonFuncs::sample_data(Mat& m_org_data, float sample_size_fraction, std::string preferred_srand){
    Mat m_sampled_data;
    // Random generator without duplicates
    PetscInt i, num_row_org_data;
    MatGetSize(m_org_data, &num_row_org_data, NULL);
//    std::cout << "[CF][SampleData] number of rows in original data are: " << num_row_org_data << std::endl; //$$debug
    std::vector<PetscInt> shuffled_indices_;
    shuffled_indices_.reserve(num_row_org_data);

    //create a vector of all possible indices, normal call to rand could generate duplicate values
    for (i=0; i < num_row_org_data; i++){
        shuffled_indices_.push_back(i);
    }

    srand(std::stoll(preferred_srand));
    std::random_shuffle( shuffled_indices_.begin(), shuffled_indices_.end() ); //shuffle all nodes
//    std::cout << "[CF][SampleData] indices are shuffled" << std::endl; //$$debug

    PetscInt num_row_sampled_data = ceil(num_row_org_data * sample_size_fraction);
//    std::cout << "[CF][SampleData] sample_size_fraction:" << sample_size_fraction << std::endl; //$$debug
//    std::cout << "[CF][SampleData] num_row_sampled_data:" << num_row_sampled_data << std::endl; //$$debug
    IS              is_sampled_;
    PetscInt        * ind_sampled_;
    PetscMalloc1(num_row_sampled_data, &ind_sampled_);
    for(i=0; i < num_row_sampled_data; i++){
        ind_sampled_[i] = shuffled_indices_[i];
    }
//    std::cout << "[CF][SampleData] sample indices are selected" << std::endl; //$$debug

    // ind_sample should sort
    std::sort(ind_sampled_, ind_sampled_ + num_row_sampled_data);   //this is critical for MatGetSubMatrix method
    ISCreateGeneral(PETSC_COMM_SELF,num_row_sampled_data,ind_sampled_,PETSC_COPY_VALUES,&is_sampled_);
    PetscFree(ind_sampled_);

    MatGetSubMatrix(m_org_data,is_sampled_, NULL,MAT_INITIAL_MATRIX,&m_sampled_data);
    PetscInt check_num_row_sampled_data;
    MatGetSize(m_sampled_data, &check_num_row_sampled_data, NULL);
//    std::cout << "[CF][SampleData] number of rows in sampled data are: " << check_num_row_sampled_data << std::endl; //$$debug

//    std::cout << "[CF][SampleData] submatrix is created" << std::endl; //$$debug
    ISDestroy(&is_sampled_);

//    printf("[CF][SampleData] sample matrix:\n");                   //$$debug
//    MatView(m_sampled_data,PETSC_VIEWER_STDOUT_WORLD);                                //$$debug

    return m_sampled_data;
}



std::string CommonFuncs::run_ext_command(const std::string ext_cmd){
    //ref: http://stackoverflow.com/a/478960/2674061
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(ext_cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("[CF][run_ext_command] popen() failed! Command:" + ext_cmd );
    while (!feof(pipe.get())) {
        std::cout << "while inside run_ext_command!\n";
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}


std::string CommonFuncs::run_ext_command_single_output(const std::string ext_cmd){
    //ref: http://stackoverflow.com/a/478960/2674061
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(ext_cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("[CF][run_ext_command] popen() failed! Command:" + ext_cmd );
    while (!feof(pipe.get())) {
//        std::cout << "while inside run_ext_command!\n";
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    char del= '\n';
    int i =0;
    while(i < result.size())
    {
        if(result[i] != del)
            i++;
        else
            break;
    }
    result = result.substr(0,i);

    return result;
}


//tested 040517-1325
void CommonFuncs::get_unique_random_id(int start_range, int end_range, int number_random_id, std::string preferred_srand, std::vector<int>& v_rand_idx){
    /// - - - - check valid request - - - -
    if(number_random_id > (end_range - start_range + 1)){
        std::cout << "Error: [CF][GURI] out of range request for random indices, Exit!"<< std::endl;
        exit(1);
    }
    /// - - - - create full vector - - - -
    std::vector<int> v_full_idx;
    v_full_idx.reserve(end_range - start_range );
    for(int i=start_range; i< end_range; i++){
        v_full_idx.push_back(i);
//        std::cout << i << ",";
    }
//    std::cout << std::endl;
    ///  - - - - shuffle it - - - -
//    std::cout << "\nrandome seed:" << preferred_srand << std::endl;
    srand(std::stoll(preferred_srand));
    std::random_shuffle(v_full_idx.begin(), v_full_idx.end() ); //shuffle all
    /// - - - - select specific number of them - - - -
    v_rand_idx.reserve(number_random_id);
//    std::cout << "\nin randome vector:\n";
    for(int i=0; i< number_random_id; i++){
        v_rand_idx.push_back(v_full_idx[i]);
//        std::cout << "index:" << i << ",";
    }
//    std::cout << std::endl;
}

