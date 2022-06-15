#include <vector>
#include <pthread.h>
#include <iostream>

#include "p1_process.h"
#include "p1_threads.h"

using namespace std;

// stuct to pass arguments
struct MergeSortArgs {
  int thread_index;
  ParallelMergeSorter * ctx;

  MergeSortArgs(ParallelMergeSorter * ctx, int thread_index) {
    this->ctx = ctx;
    this->thread_index = thread_index;
  }
};


// class constructor
ParallelMergeSorter::ParallelMergeSorter(vector<student> &original_list, int num_threads) {
  this->threads = vector<pthread_t>();
  this->sorted_list = vector<student>(original_list);
  this->num_threads = num_threads;
}

// This function will be called by each child process to perform multithreaded sorting
vector<student> ParallelMergeSorter::run_sort() {
    int threadcode;
    for (int i = 0; i < num_threads; i++) {
        // we have to use the heap for this otherwise args will be destructed in each iteration,
        // and the thread will not have the correct args struct
        MergeSortArgs * args = new MergeSortArgs(this, i);
        pthread_t threadtemp;
        threadcode = pthread_create(&threadtemp, NULL, thread_init, (void *) args);
        threads.push_back(threadtemp);
    }
    // make sure threads are done
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
  // merge sorted sublists together
  this->merge_threads();
    return this->sorted_list;
}

// standard merge sort implementation
void ParallelMergeSorter::merge_sort(int lower, int upper) {
  // Top-down merge sort
    int middle;
    // first divide array until it becomes 1
    if (lower < upper) { // if left index is less than right index, divide into two mergesorts
        middle = lower + (upper - lower) / 2;
        // recurvise call for 2 arrays
        merge_sort(lower, middle);
        merge_sort(middle + 1, upper);
        merge(lower, middle, upper);
    }
}

// Standard merge implementation for merge sort
void ParallelMergeSorter::merge(int lower, int middle, int upper) {
    // Your implementation goes here, you will need to implement:
    // Merge for top-down merge sort
    //  - The merge results should go in temporary list, and once the merge is done, the values
    //  from the temporary list should be copied back into this->sorted_list
    // once the recurvise call is finished, start merging
    // 2 arrays to merge
    // lengths of array
    // create temporary list
    int i;
    int len1 = middle - lower + 1;
    int len2 = upper - middle;
    vector<student> templist1;
    vector<student> templist2;
    for (i = 0; i < len1; i++){
        templist1.push_back(this->sorted_list[lower + i]);
    }
    for (i = 0; i < len2; i++){
        templist2.push_back(this->sorted_list[middle + i + 1]);
    }
    // index of current arrays
    int one_i = 0;
    int two_i = 0;
    // start merging, choose less of two, if there is more left, then compare with infinity
    while (one_i < len1 && two_i < len2){
        if (templist1[one_i].grade <= templist2[two_i].grade){
            this->sorted_list[lower] = templist1[one_i];
            one_i++;
            lower++;
        }
        else if (templist1[one_i].grade > templist2[two_i].grade){
            this->sorted_list[lower] = templist2[two_i];
            two_i++;
            lower++;
        }
    }
    // LEFTOVERS
    while (one_i < len1) {
        this->sorted_list[lower] = templist1[one_i];
        lower++;
        one_i++;
    }
    while (two_i < len2) {
        this->sorted_list[lower] = templist2[two_i];
        lower++;
        two_i++;
    }
}

// This function will be used to merge the resulting sorted sublists together
void ParallelMergeSorter::merge_threads() {
  // merge all threads using algorithm
  int i;
    int remainder = sorted_list.size() % num_threads;
    int work_per_thread = sorted_list.size() / num_threads;
    // find remainder
    int lower_bound;
    int upper_bound;
    int middle; // middle
    int temp_row;
    int multi_arg = 1; // index that you need to multiply by
    int num_of_threads_row = num_threads / 2; // number of times that threads that need to be combined in the row
    int num_of_threads_row_remainder = num_threads % 2; // holds the remainder value
    int prev_final_upper_bound = sorted_list.size() - work_per_thread - remainder - 1; // need this value to calculate the middle for other values
    while (num_of_threads_row > 0){
        for (i = 0; i < num_of_threads_row; i++){
            // first check if it is even, if it is u can combine with last row by changing upperboound
            if (num_of_threads_row_remainder == 0 && i == num_of_threads_row - 1){
                middle = prev_final_upper_bound; // middle gets lwoer bound because ur chaging upperbound
                upper_bound = sorted_list.size() - 1;
                lower_bound = (work_per_thread * 2) * i * multi_arg; // lower bound multiplication
                prev_final_upper_bound = lower_bound - 1;
            }
            else {
                lower_bound = (work_per_thread * 2) * i * multi_arg; // lower bound multiplication
                upper_bound = (((work_per_thread) * 2) * (i + 1) * multi_arg) - 1; // upper bound multiplication
                middle = lower_bound + (upper_bound - lower_bound) / 2;
            }
            merge(lower_bound, middle, upper_bound);
        }
        // rows should be recersive divide so u can merge them more
        temp_row = num_of_threads_row;
        num_of_threads_row = (num_of_threads_row_remainder + num_of_threads_row) / 2; // number of times that threads that need to be combined in the row
        num_of_threads_row_remainder = (num_of_threads_row_remainder + temp_row) % 2; // holds the remainder value
        multi_arg++;
    }
}

// This function is the start routine for the created threads, it should perform merge sort on its assigned sublist
void * ParallelMergeSorter::thread_init(void * args) {
  // create arguments
  MergeSortArgs * sort_args = (MergeSortArgs *) args;
  int thread_index = sort_args->thread_index;
  ParallelMergeSorter * ctx = sort_args->ctx;
  int work_per_thread = ctx->sorted_list.size() / ctx->num_threads;
  // find remainder
  int remainder = ctx->sorted_list.size() % ctx->num_threads;
  int lower_bound = thread_index * work_per_thread;
  int upper_bound = ((thread_index + 1) * work_per_thread) - 1;
    // put the remainder in the last thread
    if (thread_index == ctx->num_threads - 1){
        upper_bound += remainder;
    }
    // perform merge sort
    ctx->merge_sort(lower_bound,upper_bound);
  // Free the heap allocation
  delete sort_args;
  return NULL;
}

