//
//  SparseSymSparseMatrix.h
//  Graph Matching
//
//  Created by Ali Hajimirza on 7/9/13.
//  Copyright (c) 2013 Ali Hajimirza. All rights reserved.
//

#ifndef _SymSparseMatrix_h
#define _SymSparseMatrix_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include "SparseElement.h"
#include "MatrixExceptions.h"

#ifdef __linux__
#include "dsmatrxa.h"
#include "ardsmat.h"
#include "ardssym.h"
#include "lsymsol.h"
#endif

#ifdef USE_MPI
#include "mpi.h"
#endif

template <typename T>
class SymSparseMatrix;

template <typename T>
std::ostream& operator<< (std::ostream&, const SymSparseMatrix<T>&);

template <typename T>
class SymSparseMatrix
{
private:
    static const int _DEFAULT_MATRIX_SIZE;
    static const  T _DEFAULT_MATRIX_ENTRY;
    size_t _getArrSize() const;
    void _copy(const SymSparseMatrix<T>&);
    bool _hasEdge(int,int);
    
public:
    int _size;
    std::unordered_map <int, T > _edges;   
public:
    /**************
     *Constructors*
     **************/
    SymSparseMatrix();
    SymSparseMatrix(int, int);
    SymSparseMatrix(const std::string&);
    SymSparseMatrix(const SymSparseMatrix<T>&);

    #ifdef USE_MPI
    SymSparseMatrix(int,MPI_Status&);
    SymSparseMatrix(int,int,MPI_Status&);
    #endif

    
    /************
     *Destructor*
     ************/
    virtual ~SymSparseMatrix();
    
    /***********
     *ACCESSORS*
     ***********/
    bool isSquare() const;
    bool isSymmetric() const;
    int getSize();
    int getSparseFormSize();
    std::vector<T> getSumOfRows();
    std::vector<T> getTopEigenVector();
    std::vector<int> getNeighbors(int vertex);
    //sparse_SymSparseMatrix_element<T>** getSparseForm();
    SymSparseMatrix<T> getScatteredSelection(const std::vector<int>& vec_A, const std::vector<int> vec_B);
    
    /**********
     *MUTATORS*
     **********/
    void insert(int, int, T);

    /**********
     *OPERATORS*
     **********/
    T operator()(int,int) const;
    SymSparseMatrix<T> kron(const SymSparseMatrix<T>&); /* WORKS SHOULD BE CHANGED */
    bool operator==(const SymSparseMatrix<T>&);
    SymSparseMatrix<T>& operator- (SymSparseMatrix<T>& other_matrix);
    SymSparseMatrix<T>& operator= (const SymSparseMatrix<T>&);
    SymSparseMatrix<T> diagonalVectorTimesSymSparseMatrix(const std::vector<T>&);
    SymSparseMatrix<T> SymSparseMatrixTimesDiagonalVector(const std::vector<T>&);
    friend std::ostream& operator<< <> (std::ostream& stream, const SymSparseMatrix<T>& SymSparseMatrix);

    /* WILL IMPLEMENT IF I HAD TIME
     SymSparseMatrix<T> operator* (const SymSparseMatrix<T>&);
     SymSparseMatrix<T> operator+ (const SymSparseMatrix<T>&);
     SymSparseMatrix<T> operator- (const SymSparseMatrix<T>&);
     SymSparseMatrix<T> operator* (T);
     SymSparseMatrix<T> operator+ (T);
     SymSparseMatrix<T> operator- (T);
     void operator*= (const SymSparseMatrix<T>&);
     void operator+= (const SymSparseMatrix<T>&);
     void operator-= (const SymSparseMatrix<T>&);
     void operator*= (T);
     void operator+= (T);
     void operator-= (T);
     */
    
};
//==========================================================CONSTANTS============================================================
template <typename T>
const int SymSparseMatrix<T>::_DEFAULT_MATRIX_SIZE = 1;
template <typename T>
const T SymSparseMatrix<T>::_DEFAULT_MATRIX_ENTRY = 1;
//==========================================================CONSTRUCTORS============================================================

template <typename T>
inline SymSparseMatrix<T>::SymSparseMatrix()
{
    this->_rows = _DEFAULT_MATRIX_SIZE;
    this->_cols = _DEFAULT_MATRIX_SIZE;
}

template<typename T>
inline SymSparseMatrix<T>::SymSparseMatrix(const std::string& file_path)
{
    int tmp_i;
    int tmp_j;
    std::ifstream file_reader;
    file_reader.open(file_path.c_str());
    
    if(file_reader.fail())
    {
        file_reader.close();
        throw FileDoesNotExistException(file_path.c_str());
    }
    
    file_reader >> this->_size;
    file_reader >> tmp_i;

    if (this->_size != tmp_i)
    {
        file_reader.close();
        throw NotASquareMatrixException();
    }
    file_reader >> tmp_i;       //skip the number of lines entry
    
    while (!file_reader.eof())
    {
        file_reader >> tmp_i;
        file_reader >> tmp_j;
        this->insert(tmp_i - 1 ,tmp_j - 1, _DEFAULT_MATRIX_ENTRY);
    }
    file_reader.close();
}

#ifdef USE_MPI
template <typename T>
inline SymSparseMatrix<T>::SymSparseMatrix(int source, int tag, MPI_Status& stat)
{
    MPI_Recv(&this->_size, 1, MPI_INT, source, tag + 1, MPI_COMM_WORLD, &stat);
    int recv_edges_size = (int)(0.5 * this->_size * (this->_size + 1));
    T* recv_edges = new T[recv_edges_size];
    MPI_Recv(recv_edges, recv_edges_size*sizeof(T), MPI_BYTE, source, tag + 2, MPI_COMM_WORLD, &stat);

    int counter = 0;
    for (int i = 0; i < this->_size; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            this->insert(i, j,recv_edges[counter++]);
        }
    }
    delete [] recv_edges;
}

template <typename T>
inline SymSparseMatrix<T>::SymSparseMatrix(int source, MPI_Status& stat)
{
    MPI_Bcast(&this->_size, 1, MPI_INT, source, MPI_COMM_WORLD);
    int recv_edges_size = (int)(0.5 * this->_size * (this->_size + 1));
    T* recv_edges = new T[recv_edges_size];
    MPI_Bcast(recv_edges, recv_edges_size*sizeof(T), MPI_BYTE, source, MPI_COMM_WORLD);

    int counter = 0;
    for (int i = 0; i < this->_size; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            this->insert(i, j,recv_edges[counter++]);
        }
    }
    delete [] recv_edges;
}

#endif

template <typename T>
inline SymSparseMatrix<T>::SymSparseMatrix(int rows, int cols)
{
    this->_rows = rows;
    this->_cols = cols;
}

template <typename T>
inline SymSparseMatrix<T>::SymSparseMatrix(const SymSparseMatrix<T>& SymSparseMatrix)
{
    _copy(SymSparseMatrix);
}

//==========================================================DESTRUCTOR==============================================================
template <typename T>
inline SymSparseMatrix<T>::~SymSparseMatrix()
{
}

//===========================================================ACCESSORS===============================================================
template <typename T>
inline int SymSparseMatrix<T>::getSize()
{
    return this->_size;
}

template <typename T>
inline bool SymSparseMatrix<T>::isSymmetric() const ////////////////////////////TODO
{
    // //sym. SymSparseMatrix has to be square
    // if (this->_rows != this->_cols)
    // {
    //     return false;
    // }
    
    // //chaking for entries to be equal
    // for(int i = 0; i < this->_rows; i++)
    // {
    //     for(int j = 0; j < this->_cols; j++)
    //     {
    //         if (this->_edges[(i * this->_cols) + j] != this->_edges[(j * this->_cols) + i])
    //         {
    //             return false;
    //         }
    //     }
    // }
    
    // return true;
}

//template <typename T>
//sparse_SymSparseMatrix_element<T>** SymSparseMatrix<T>::getSparseForm(){
//    
//    if(sparse_form != NULL)
//    {
//        return this->sparse_form;
//    }
//    
//    for(int i = 0; i < this->_getArrSize(); i++)
//    {
//        if(this->_edges[i] != 0)
//        {
//            sparse_form_size++;
//        }
//    }
//    
//    sparse_form = new sparse_SymSparseMatrix_element<T>* [sparse_form_size];
//    int counter=0;
//    
//    for(int i = 0; i < this->_getArrSize(); i++)
//    {
//        if(this->_edges[i] != 0)
//        {
//            sparse_SymSparseMatrix_element<T> *ele= new sparse_SymSparseMatrix_element<T>;
//            ele->row_index = i / this->_rows;
//            ele->col_index = i % this->_rows;
//            ele->value = this->_edges[i];
//            sparse_form[counter]=ele;
//            counter++;
//        }
//    }
//    
//    return this->sparse_form;
//}

template <typename T>
inline SymSparseMatrix<T> SymSparseMatrix<T>::getScatteredSelection(const std::vector<int>& vec_A, const std::vector<int> vec_B)
{
    int num_in_A = 0;
    for (int i=0; i< vec_A.size(); i++)
    {
        if (vec_A[i] == 1)
        {
            num_in_A++;
        }
    }
    int num_in_B = 0;
    for (int i=0; i < vec_B.size(); i++)
    {
        if( vec_B[i] == 1)
        {
            num_in_B++;
        }
    }
    //Initializing and allocating the product SymSparseMatrix
    SymSparseMatrix<T> res_SymSparseMatrix(num_in_A, num_in_B);
    
    int counter = 0;
    
    for (int i=0; i < vec_A.size(); i++)
    {
        for(int j=0; j < vec_B.size(); j++)
        {
            if ( vec_A[i] == 1 && vec_B[j] ==1)
            {
                res_SymSparseMatrix._edges[counter] = (*this)(i,j);
                counter++;
            }
        }
    }
    return res_SymSparseMatrix;
}



template <typename T>
inline std::vector<int> SymSparseMatrix<T>::getNeighbors(int vertex)
{
    std::vector<int> neighbors;
    
    for(int i = 0; i < this->getNumberOfRows(); i++)
    {
        if(this->_edges[(i * this->_cols) + vertex] == 1)
        {
            neighbors.push_back(i);
        }
    }
    
    return neighbors;
}

template <typename T>
inline std::vector<T> SymSparseMatrix<T>::getTopEigenVector(){
    
    //    int arr_size= (0.5 * this->_rows * (this->_rows+1));
    //    double nzval[arr_size];
    //    int counter = 0;
    //
    //    for(int i = 0; i < this->_rows; i++)
    //    {
    //        for(int j = i; j < this->_rows; j++)
    //        {
    //            nzval[counter] = (*this)(i,j);
    //            counter++;
    //        }
    //    }
    //
    //    ARdsSymSymSparseMatrix<T> ARSymSparseMatrix(this->_rows,nzval,'L');
    //    ARluSymStdEig<T> eigProb(1, ARSymSparseMatrix, "LM", 10);
    //    eigProb.FindEigenvectors();
    //    std::vector<T>* eigenVec = new std::vector<T>(eigProb.GetN());
    //
    //    for (int i=0; i < eigProb.GetN() ; i++)
    //    {
    //        eigenVec[i] = eigProb.Eigenvector(0,i);
    //    }
    //
    //    return eigenVec;
}


/*
 * returns a vector of type T where each entry corresponds to sum of entries in a SymSparseMatrix row.
 * Delete this object after usage.
 * @return std::vector<T>
 */
template <typename T>
inline std::vector<T> SymSparseMatrix<T>::getSumOfRows()
{
    std::vector<T>* sum_vector= new std::vector<T>(this->_rows);
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        (*sum_vector)[(i / this->_rows)] += this->_edges[i];
    }
    
    return sum_vector;
}

//===========================================================MUTATORS================================================================
template <typename T>
inline void SymSparseMatrix<T>::insert(int i, int j, T value)
{
    if ( (i >= _size) || (i < 0) || (j >= _size) || (j < 0) )
    {
        std::cout << "Out of bounds: "<< i  <<" " << j << std::endl;
        throw IndexOutOfBoundsException();
    }
    //if the edge exist =>delete the node
    if (_hasEdge(i,j))
    {
        if(i<=j)
        {
            this->_edges.erase(i+size_t(j)*(j+1)/2);
        }
        else
        {
            this->_edges.erase(j+size_t(i)*(i+1)/2);
        }
    }
    //if the evalue is not 0 =>insert it
    if (value != 0)
    {
        if(i<=j)
        {
            this->_edges[i+size_t(j)*(j+1)/2] = value;
        }
        else
        {
            this->_edges[j+size_t(i)*(i+1)/2] = value;
        }
    }
}


//==========================================================OPERATORS================================================================
template <typename T>
inline SymSparseMatrix<T> SymSparseMatrix<T>::kron(const SymSparseMatrix<T>& matrix)
{
    // checking for matrices to be square
    if (!this->isSquare() || !matrix.isSquare())
    {
       // throw NotASquareSymSparseMatrixException();
    }
    
    //Initializing and allocating the product SymSparseMatrix
    int prod_size = this->_rows * matrix._rows;
   // SymSparseMatrix<T>* prod_matrix_right = new SymSparseMatrix<T>(prod_size, prod_size);
    SymSparseMatrix<T> prod_matrix(prod_size, prod_size);
    prod_matrix._edges = std::vector<SparseElement<T> >(this->_getArrSize()*matrix._getArrSize());

    int counter = 0;
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        for(int j = 0; j < matrix._getArrSize(); j++)
        {
            prod_matrix._edges[counter++] = (SparseElement<T>((this->_edges[i].getX()*matrix._rows) + matrix._edges[j].getX(),
                                                              (this->_edges[i].getY()*matrix._cols) + matrix._edges[j].getY(),
                                                              this->_edges[i].getValue() *  matrix._edges[j].getValue()));
        }
    }
    
    std::sort(prod_matrix._edges.begin(), prod_matrix._edges.end());
    return prod_matrix;
}

template <typename T>
inline SymSparseMatrix<T> SymSparseMatrix<T>::diagonalVectorTimesSymSparseMatrix(const std::vector<T>& vec)
{
    if(this->_rows != vec.size())
    {
        throw DimensionMismatchException();
    }
    
    SymSparseMatrix<T> ret_SymSparseMatrix (*this);
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        ret_SymSparseMatrix._edges[i] = vec[i/this->_rows] * this->_edges[i];
    }
    
    return ret_SymSparseMatrix;
}

template <typename T>
inline SymSparseMatrix<T> SymSparseMatrix<T>::SymSparseMatrixTimesDiagonalVector(const std::vector<T>& vec)
{
    if(this->_cols != vec.size())
    {
        throw DimensionMismatchException();
    }
    
    SymSparseMatrix<T> ret_SymSparseMatrix(*this);
    for(int i = 0; i < this->_getArrSize(); )
    {
        for(int j = 0; j < vec.size(); j++)
        {
            ret_SymSparseMatrix._edges[i] = this->_edges[i] * vec[j];
            i++;
        }
    }
    
    return ret_SymSparseMatrix;
}
/////////////////////////////////this point
template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const SymSparseMatrix<T>& matrix)
{
    stream<< "Size: " << matrix._size << "*" << matrix._size << '\n';
    for (int i = 0; i < matrix._size; i++)
    {
        for (int j = 0; j < matrix._size; j++)
        {
            stream << matrix(i, j) << ' ';
        }
        stream << "\n";
    }
    stream << "\n\n\n";
    return stream;
}

template <typename T>
inline T SymSparseMatrix<T>::operator()(int i, int j) const
{
    if ( (i >= this->_size) || (i < 0) || (j >= this->_size) || (j < 0) )
    {
        throw IndexOutOfBoundsException();
    }

    typename std::unordered_map<int,T>::const_iterator find_res;
    if(i<=j)
    {
        find_res = this->_edges.find(i+size_t(j)*(j+1)/2);
    }
    else
    {
        find_res = this->_edges.find(j+size_t(i)*(i+1)/2);
    }

    if ( find_res == this->_edges.end() )
        return 0;
    else
        return find_res->second;
}


template <typename T>
inline SymSparseMatrix<T>& SymSparseMatrix<T>::operator=(const SymSparseMatrix<T>& rhs)
{
    _copy(rhs);
}

template <typename T>
inline bool SymSparseMatrix<T>::operator==(const SymSparseMatrix<T>& rhs)
{
    // checking for dimension equality
    if ((this->_cols != rhs._cols) || (this->_rows != rhs._rows) || (this->_getArrSize() != rhs._getArrSize()))
    {
        return false;
    }
    
    //checking for entry equality
    for (int i = 0; i < this->_getArrSize(); i++)
    {
        for( int j = 0; j< this->_cols; j++)
        {
            if ((*this)(i,j) != rhs(i,j))
                return false;
        }
    }
    
    return true;
}

//===========================================================PRIVATE=================================================================
template <typename T>
inline void SymSparseMatrix<T>::_copy(const SymSparseMatrix<T>& rhs)
{
    this->_size = rhs._size;
    this->_edges = std::unordered_map<int, T>(rhs._edges);
}

template <typename T>
inline bool SymSparseMatrix<T>::_hasEdge(int i, int j)
{
    if ( (i >= _size) || (i < 0) || (j >= _size) || (j < 0) )
    {
        throw IndexOutOfBoundsException();
    }
    
    if(i<=j)
    {
        return (this->_edges.find(i+size_t(j)*(j+1)/2) == this->_edges.end());
    }
    else
    {
        return (this->_edges.find(j+size_t(i)*(i+1)/2) == this->_edges.end());
    }
}


//===================================================================================================================================
    

#endif
