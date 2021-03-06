/***********************************************************************************************
 * This file contains 5 algorithms that can be used to find the node to node mapping between   *
 * two graphs. All algorithms are essentially based on the greedy algorithm. The highest score *
 * assigned by IsoRank is chosen and then all scores corresponding to already assigned nodes   *
 * are eliminated. Some of the other algorithms make use of the connectivity of graphs as well.*
 * All helper functions used by these algorithms can be found in the greedy_algorithms_helper.h*
 * file.                                                                                       *
 ***********************************************************************************************/

#ifndef _GREEDY_ALGORITHM_h
#define _GREEDY_ALGORITHM_h

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <cmath>
#include "Matrices/DenseMatrix1D.h"
#include "GreedyAlgorithmsHelper.h"
#include <limits>




/*
 * performs the greedy algorithm on the scores matrix for nodal pairings
 * and returns a matching between nodes of graph1 and graph2
 * @pram: matrix indicating the scores of nodal pairings
 * @pram: adjacency matrix of graph1
 * @pram: adjacency matrix of graph2
 * @pram: array that indicates the final mappings done
 */
template <typename DT>
void greedy_1(DenseMatrix1D<DT>& matches, DenseMatrix1D<float>& graph1, DenseMatrix1D<float>& graph2,int* assignment){
    DT total_score=0;
    int graph1_nodes=matches.getNumberOfRows();
    int graph2_nodes=matches.getNumberOfColumns();
    
    int num_of_nodes=min(graph1_nodes,graph2_nodes);
    int max_value;
    int row,col;
    
    //initialize assignment array
    init_array(assignment,graph1_nodes,-1);
    
    for(int i=0;i<min(graph1_nodes,graph2_nodes);i++){
        
        //get maximum score in matrix and set assignment
        return_max(matches,&total_score,&row,&col);
        invalidate(row,col,matches);
        assignment[row]=col;
        
    }
    
    match_rest(assignment,graph1,graph2);
}


/*
 * performs a greedy algorithm to choose the best nodal pairs for matching
 * enforces connectivity: if i<->j then neigh(i)<->neigh(j) where <-> indicates a matching
 * @pram: matrix indicating scores for nodal pairs
 * @pram: adjacency matrix for graph1
 * @pram: adjacency matrix for graph2
 * @pram: pointer to the array that indicates the best matching
 */
template<typename DT>
void greedy_connectivity_1(DenseMatrix1D<DT>& matches, DenseMatrix1D<float>& graph1, DenseMatrix1D<float>& graph2,int* assignment){
    
    DT total_score=0;
    int graph1_nodes=matches.getNumberOfRows();
    int graph2_nodes=matches.getNumberOfColumns();
    
    DT max_value;
    
    int row,col;
    
    //initialize assignment array
    init_array(assignment,graph1_nodes,-1);
    
    
    for(int i=0;i<min(graph1_nodes,graph2_nodes);i++){
        
        //find maximum in scores matrix and perform assignment
        return_max(matches,&total_score,&row,&col);
        assignment[row]=col;
        invalidate(row,col,matches);
        
        //change matrix s.t. only neighbors of row are allowed to
        //match to neighbors of col
        neighbor_enforcement(&row,&col, graph1,graph2,matches);
    }
    
    match_rest(assignment,graph1,graph2);
}



/*
 * performs a greedy matching and enforces connectivity by proceeding outwards radially
 * @pram: matrix indicating scores for nodal pairs
 * @pram: adjacency matrix for graph1
 * @pram: adjacency matrix for graph2
 * @pram: pointer to the array that indicates the best matching
 */
template<typename DT>
void greedy_connectivity_2(DenseMatrix1D<DT>& matches, DenseMatrix1D<float>& graph1, DenseMatrix1D<float>& graph2,int* assignment){
    
    DT max_tol=pow(10,-6),max;
    DT score=0,prev_score=0,final_score=0;
    
    int graph1_nodes=graph1.getNumberOfColumns();
    int graph2_nodes=graph2.getNumberOfColumns();
    int assignment2[graph1_nodes];
    int row,col;
    
    //initialize assignment array
    init_array(assignment,graph1_nodes,-1);
    init_array(assignment2,graph1_nodes,0);
    
    DenseMatrix1D<DT>* active_matches=new DenseMatrix1D<DT>(matches);
    DT* idxarr;
    std::vector<int> assigned_G1;
    int size=0,random_id,vector_size=0,curr_row;
    
    
    //run while loop until all nodes are assigned and scores matrix isn't all negative
    while(sum_array(assignment2,graph1_nodes)<min(graph1_nodes,graph2_nodes)&& return_max(*active_matches,&score,&row,&col)>-1)
    {
        
        if(all_inf(*active_matches)){
            match_rest(assignment,graph1,graph2);
            printf("score of matching: %f\n", final_score);
            return;
        }
        
        //find all values in scores matrix greater than a certain amount
        idxarr=find_values(*active_matches, score-prev_score-max_tol,&size);
        random_id=rand()%size+1;
        max= get_Max(active_matches,random_id,score-prev_score-max_tol,&row,&col);
        
        //perform assignment by choosing a random pair thats high enough
        final_score+=max;
        assignment[row]=col;
        assignment2[row]=1;
        
        assigned_G1.push_back(row);
        vector_size++;
        invalidate(row, col, matches);
        set_to_min(*active_matches);
        int curr_col;
        
        //change scores matrix such that only neighbors of already matched
        //nodes are allowed to match to one another
        for(int i=0;i<vector_size;i++)
        {
            curr_row=assigned_G1[i];
            curr_col=assignment[curr_row];
            std::vector<int> neigh_1= graph1.getNeighbors(curr_row);
            std::vector<int> neigh_2= graph2.getNeighbors(curr_col);
            set_matrix_values(*active_matches,matches,neigh_1,neigh_2);
        }
        
        prev_score=score;
    }
    
}



/*
 * performs a greedy matching and enforces connectivity by proceeding outwards radially
 * @pram: matrix indicating scores for nodal pairs
 * @pram: adjacency matrix for graph1
 * @pram: adjacency matrix for graph2
 * @pram: pointer to the array that indicates the best matching
 */
template<typename DT>
void greedy_connectivity_3(DenseMatrix1D<DT>& matches, DenseMatrix1D<float>& graph1, DenseMatrix1D<float>& graph2,int* assignment){
    
    DT total_score=0;
    DT final_score=0;
    int row,col;
    int graph1_nodes=graph1.getNumberOfRows();
    int graph2_nodes=graph2.getNumberOfRows();
    int assignment2[graph1.getNumberOfRows()];
    int assignment_G1[graph1.getNumberOfRows()];
    int assignment_G2[graph2.getNumberOfRows()];;
    DenseMatrix1D<DT>* local_matches=new DenseMatrix1D<DT>(matches);
    
    //initialize all arrays
    init_array(assignment,graph1_nodes,-1);
    init_array(assignment2,graph1_nodes,0);
    init_array(assignment_G1,graph1_nodes,0);
    init_array(assignment_G2,graph1_nodes,0);
    
    std::vector<int> neigh_1;
    std::vector<int> neigh_2;
    
    //run while loop until all nodes are assigned
    while(sum_array(assignment2,graph1_nodes)<min(graph1_nodes,graph2_nodes)){
        
        //find the highest matching score and make that assignment
        return_max(matches, &final_score,&row,&col);
        assignment[row]=col;
        assignment2[row]=1;
        assignment_G1[row]=1;
        assignment_G2[col]=1;
        invalidate(row,col,matches);
        
        
        
        //find neighbors of already assigned nodes and invalidate already assigned nodes from further consideration
        neigh_1= graph1.getNeighbors(row);
        neigh_2= graph2.getNeighbors(col);
        
        invalidate_neighbors(assignment_G1,neigh_1);
        invalidate_neighbors(assignment_G2,neigh_2);
        
        set_to_min(*local_matches);
        set_matrix_values(*local_matches,matches,neigh_1,neigh_2);
        int row_inside= row;
        int col_inside=col;
        
        //if scores matrix is all -inf match unassigned nodes and return
        if(all_inf(matches)){
            match_rest(assignment,graph1,graph2);
            return;
        }
        
        //run for loop until all neighbors are assigned and score matrix isn't all -inf
        for(int i=0;i<min(neigh_1.size(),neigh_2.size())&&!all_inf(*local_matches);i++){
            
            //find best nodal pairing and perform assignment
            return_max(*local_matches,&final_score,&row,&col);
            assignment[row]=col;
            assignment2[row]=1;
            assignment_G1[row]=1;
            assignment_G2[col]=1;
            
            //invalidate already assigned nodes from further consideration
            invalidate(row,col,*local_matches);
            invalidate(row,col,matches);
            
            //if scores matrix is all -inf match unassigned nodes and return
            if(all_inf(matches)){
                match_rest(assignment,graph1,graph2);
                return;
            }
            
        }
    }
    
    match_rest(assignment,graph1,graph2);
    return;
}


/*
 * performs a greedy matching and enforces connectivity by proceeding outwards radially
 * chooses the most connected neighbor at every iteration
 * @pram: matrix indicating scores for nodal pairs
 * @pram: adjacency matrix for graph1
 * @pram: adjacency matrix for graph2
 * @pram: pointer to the array that indicates the best matching
 */
template <typename DT>
void greedy_connectivity_4(DenseMatrix1D<DT>& matches, DenseMatrix1D<float>& graph1, DenseMatrix1D<float>& graph2,int* assignment){
    
    DT final_score=0;
    int* add_order=new int[graph1.getNumberOfRows()];
    int* ass=new int[graph1.getNumberOfRows()];
    int add_idx=0;
    DT score=0;
    DT max_tol=pow(10,-6);
    int row,col,size=0;
    int assigned_G1[graph1.getNumberOfRows()];
    int assigned_G2[graph2.getNumberOfRows()];
    std::vector<int> neigh_1;
    std::vector<int> neigh_2;
    int add_order_counter=2;
    
    
    //initializing all arrays
    init_array(add_order,graph1.getNumberOfRows(),-1);
    init_array(ass,graph1.getNumberOfRows(),0);
    init_array(assigned_G1,graph1.getNumberOfRows(),-1);
    init_array(assigned_G2,graph2.getNumberOfRows(),-1);
    
    //set row and col to be the nodes that have the highest score
    return_max(matches, &score,&row,&col);
    
    //fill up idx_array values in scores matrix that are
    //greater than score - max_tol and choose one randomly to assign
    DT* idx_array =find_values(matches,score - max_tol,&size);
    int random_id=rand()%size+1;
    DT max= get_Max(&matches,random_id,score-max_tol,&row,&col);
    
    //assign first row column pair
    final_score+=max;
    assignment[row]=col;
    assigned_G1[row]=1;
    assigned_G2[col]=1;
    invalidate(row,col,matches);
    add_order[0]=row;
    
    
    neigh_1= graph1.getNeighbors(row);
    neigh_2= graph2.getNeighbors(col);
    delete []idx_array;
    
    
    int hold;
    
    //remove nodes that are already assigned from consideration in the next assignment
    invalidate_neighbors(assigned_G1,neigh_1);
    invalidate_neighbors(assigned_G2,neigh_2);
    
    
    
    //create local score matrix and only set the values for the pairs we're considering
    DenseMatrix1D<DT>* matches_local=new DenseMatrix1D<DT>(matches.getNumberOfRows(),matches.getNumberOfColumns());
    set_matrix_values(*matches_local, matches, neigh_1, neigh_2);
    
    score=0;
    size=0;
    return_max(*matches_local, &score,&row,&col);
    idx_array=find_values(*matches_local,score-max_tol,&size);
    random_id=rand()%size+1;
    max=get_Max(matches_local,random_id,score-max_tol,&row,&col);
    
    //assign the second row and column pair
    final_score+=max;
    assignment[row]=col;
    assigned_G1[row]=1;
    assigned_G2[col]=1;
    invalidate(row,col,*matches_local);
    invalidate(row,col,matches);
    add_order[1]=row;
    
    delete []idx_array;
    
    int counter=0;
    int rows_cols_size;
    struct coordinate_pair **rows_cols;
    int valid_entries_size,valid_entries2_size;
    int* valid_entries;
    int* valid_entries2;
    int g1c_count_counter;
    
    //remove nodes that are already assigned from consideration in the next assignment
    invalidate_neighbors(assigned_G1,neigh_1);
    invalidate_neighbors(assigned_G2,neigh_2);
    
    int best_row=0,best_col=0;
    
    //while loop that runs until either the last node is assigned or we run out of possible matchings
    while(add_order[graph1.getNumberOfRows()-1]==-1) {
        
        //for loop that aims to match all the neighbors of the currently selected nodal pairing
        for(int s=0; s<min(neigh_1.size(),neigh_2.size());s++) {
            score=0;
            
            //finds all node pairings that are above a certain score and stores them in array rows_cols
            return_max(*matches_local,&score,&row,&col);
            idx_array=find_values(*matches_local,score-max_tol,&size);
            
            valid_entries_size=0;
            valid_entries2_size=0;
            rows_cols=find_all_values(*matches_local,idx_array,size,&rows_cols_size);
            delete []idx_array;
            
            //if number of nodal pairings with a high score is greater than 1
            if(size>1) {
                
                //looks through graph1 to see which edges exist and their intersection with node pairings with high enough scores
                valid_entries= get_valid_entries(graph1,assignment,graph1.getNumberOfRows(),&valid_entries_size);
                std::vector<int>* prev_assigned = intersect(valid_entries,valid_entries_size,rows_cols,size);
                std::vector<int> g1c_count(graph1.getNumberOfRows(), -1);
                g1c_count_counter=0;
                
                //find the connectivity of nodes being considered for matching
                
                
                for(int k=0;k<prev_assigned->size();k++){
                    int id=(*prev_assigned)[k];
                    int sum=0;
                    
                    for(int j=0;j<valid_entries_size;j++){
                        if(valid_entries[j]==id) {
                            sum++;
                        }
                    }
                    
                    g1c_count[id]=sum;
                    g1c_count_counter++;
                }
                
                //find the node best_row with highest connectivity to match
                std::vector<int> *max_g1c=vector_max(&g1c_count);
                int rand_number = rand()%(max_g1c->size());
                best_row=(*max_g1c)[rand_number];
                std::vector<int>* best_cols= new std::vector<int>();
                int best_cols_counter=0;
                
                //find all nodes in graph2 that are available for matching to
                //node just chosen from graph1
                for(int r=0;r<rows_cols_size;r++) {
                    struct coordinate_pair *c_pair=rows_cols[r];
                    if(c_pair->row==best_row) {
                        best_cols->push_back(c_pair->col);
                        best_cols_counter++;
                    }
                }
                
                
                //find node from graph2 to match to node just chosen from graph1
                valid_entries2= get_valid_entries(graph2,assignment,graph1.getNumberOfRows(),&valid_entries2_size);
                std::vector<int> *cols_chosen=choose_cols(rows_cols,rows_cols_size,best_row);
                
                std::vector<int> g2c_count(graph2.getNumberOfRows());
                int g2c_count_counter=0;
                
                //finds the connectivity of each of the nodes being considered
                for(int k=0;k<cols_chosen->size();k++) {
                    int id=(*cols_chosen)[k];
                    int sum=0;
                    
                    for(int m=0;m<valid_entries2_size;m++){
                        if(valid_entries2[m]==id){
                            sum++;
                        }
                    }
                    g2c_count[id]=sum;
                    g2c_count_counter++;
                }
                
                std::vector<int> *max_g2c=vector_max(&g2c_count);
                rand_number = rand()%(max_g2c->size());
                best_col=(*max_g2c)[rand_number];
                
                
                delete best_cols;
                delete max_g1c;
                delete max_g2c;
                delete prev_assigned;
                delete []valid_entries2;
                delete []valid_entries;
                delete cols_chosen;
                
            }
            else if(size==1){
                //if number of pairings is just 1
                struct coordinate_pair *cp=rows_cols[0];
                best_row=cp->row;
                best_col=cp->col;
            }
            else{	 
                for(int x=0;x<rows_cols_size;x++){
                    free(rows_cols[x]);
                }
                
                delete []rows_cols; 
                break;
            }
            
            //perform the assignment and invalidate corresp. rows and columns in scores matrix 
            //from further consideration
            assignment[best_row]=best_col;
            assigned_G1[best_row]=1;
            assigned_G2[best_col]=1;
            
            DT max_matches=matches(best_row, best_col);
            add_order[add_order_counter]=best_row;
            add_order_counter++;
            invalidate(best_row,best_col,*matches_local);
            invalidate(best_row,best_col,matches);
            
            
            final_score+=max_matches;
            
            
            for(int x=0;x<rows_cols_size;x++){
                free(rows_cols[x]);
            }
            
            delete []rows_cols; 
            
        } //for min(neigh1, neigh2) 
        
        
        add_idx++;	
        int r=add_order[add_idx];
        
        //if a match not made at add_ixth iteration break
        if(r==-1){
            add_order[graph1.getNumberOfRows()-1]=-2;
            break;
        }
        
        //choose the next set of nodes to match   
        int c=assignment[r];
        
      	neigh_1=graph1.getNeighbors(r);
        neigh_2=graph2.getNeighbors(c);
        
        
        //remove nodes that have already been assigned from consideration
        invalidate_neighbors(assigned_G1,neigh_1);
        invalidate_neighbors(assigned_G2,neigh_2);
        
        
        
        set_matrix_values(*matches_local, matches, neigh_1, neigh_2);
        counter++;
        
        
    } //while add_order
    
    //if matching is not complete match the rest of the nodes
    for(int i=0;i<graph1.getNumberOfRows();i++){
        if(assignment[i]==-1){
            match_rest(assignment,graph1,graph2);
            break;
        }
    }
    delete matches_local;
    delete []ass;
    delete []add_order;
}

#endif
