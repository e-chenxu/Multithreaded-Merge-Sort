#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "processes.h"
#include "threads.h"

using namespace std;

// called in each child process right after forking
void process_classes(vector<string> classes, int num_threads) {
  printf("Child process is created. (pid: %d)\n", getpid());
    string line;
    ifstream file;
    string temporary_string; // holds string that we need to conver to the numbers
    unsigned long id;
    double grade;
    int count;
    double sum; // holds sum of entire file
    double average, median, standard_dev; // stuff
    // gets the files to sort
    for (int i = 0; i < classes.size(); i++) {
        // get all the input/output file names here
        string class_name = classes[i];
        char buffer[40];
        sprintf(buffer, "input/%s.csv", class_name.c_str());
        string input_file_name(buffer);
        sprintf(buffer, "output/%s_sorted.csv", class_name.c_str());
        string output_sorted_file_name(buffer);
        sprintf(buffer, "output/%s_stats.csv", class_name.c_str());
        string output_stats_file_name(buffer);
        vector<student> students;
        count = 0;
        // CALCULATE THE MEAN HERE ALSO
        sum = 0;
        file.open(input_file_name.c_str(), ios::in);
        if (file.is_open()){
              getline(file,line); // skip header line
              while (getline(file,line) && !line.empty()){
                stringstream stringline(line);
                getline(stringline,temporary_string,',');
                id = strtoul(temporary_string.c_str(),0,0);
                getline(stringline,temporary_string,',');
                grade = strtod(temporary_string.c_str(),0);
                sum += grade;
                students.push_back(student(id,grade));
                count++;
              }
        }
        else {
            cout << "File Broke";
        }
        // mean
        average = sum / students.size();
        // close file
        file.close();
        // Run multi threaded sort
        ParallelMergeSorter sorter(students, num_threads); // creates a ParallelMergeSorter class called sorter
        vector<student> sorted = sorter.run_sort(); // sorter.run_sort returns a sorted list of students
        // create output file
        ofstream outfile;
        outfile.open(output_sorted_file_name.c_str());
        outfile << "Rank,Student ID,Grade\n";
        string out_string;
        sum = 0;
        // THIS IS FOR THE SORTED
        // DO THE STANDARD DEVIATION HERE
        int rank = 1; // this counts the rank
        // we put the highest here
        for (int j = students.size() - 1; j >= 0; j--) {
            ostringstream line_rank;
            line_rank << rank;
            ostringstream line_id;
            line_id << setprecision(12) << sorted[j].id;
            ostringstream line_grade;
            line_grade << setprecision(12) << sorted[j].grade;
            // get summation for the standard deviation
            sum += pow(sorted[j].grade - average,2.0);
            out_string = line_rank.str() + "," + line_id.str() + "," + line_grade.str() + "\n";
            outfile << out_string;
            rank++;
        }
        // stuff
        standard_dev = sqrt(sum/students.size());
        median = sorted[students.size()/2].grade;
        outfile.close();
        // NOW WE DO THE STATS
        outfile.open(output_stats_file_name.c_str());
        outfile << "Average,Median,Std. Dev\n";
        outfile << average << "," << median << "," << standard_dev;
        outfile.close();
    }
  // child process done, exit the program
  printf("Child process is terminated. (pid: %d)\n", getpid());
  exit(0);
}


void create_processes_and_sort(vector<string> class_names, int num_processes, int num_threads) {
  vector<pid_t> child_pids;
  int classes_per_process = max(class_names.size() / num_processes, 1ul);
  int classes_per_process_remainder = class_names.size() % num_processes;
  // assign remainder
  vector<string> class_names_parameters;
  int i,j;
  int index = 1; // calculate what index i should be on
  // if goes over, then add 1 to num_proecses, and remainde rshould be 0
  if (num_processes > class_names.size()){
      classes_per_process_remainder = 0;
  }
  for (i = 0; i < num_processes; i++){
      // each fork gets their classes per processes
      index = i * classes_per_process;
      switch (fork()) {
          case 0: // when child created, assign the fiels to each
              // check if the num of processes goes over the limit of fiels
              // if it does, just give the first file to each
              if (i + 1 > class_names.size()) {
                  index = 0;
              }
              // if its the last one, assign remainders to the last
              if (num_processes - 1 == i) {
                  for (j = index; j < index + classes_per_process + classes_per_process_remainder; j++) {
                      class_names_parameters.push_back(class_names[j]);
                  }
              }
              else {
                  for (j = index; j < index + classes_per_process; j++) {
                      class_names_parameters.push_back(class_names[j]);
                  }
              }
              // process the classes for tjhe fork
              process_classes(class_names_parameters, num_threads);
              break;
          case -1:
              exit(1);
          default: // parent
              continue;
      }
  }
  // wait for child
  while(wait(NULL) > 0);
}


