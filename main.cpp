#include <bits/stdc++.h>
#include "record_class.h"
using namespace std;

#define BUFFER_SIZE 21

vector<Records> buffers(BUFFER_SIZE);
vector<string> tempFiles;

// Used in priority queue to sort records
struct CompareRecords {
    bool operator()(const Records& a, const Records& b) const {
        return a.emp_record.eid > b.emp_record.eid;
    }
};

// Load records, sort them, and write to temp files
// Results in all records being written to temp files: first run
void Sort_Buffer(fstream &empin) {
    string runFileName;
    Records tempRecord;
    int runs = 0;
    int counter = 0;
    
    // Loop gets records from Emp.csv and loads them into the page buffer: loads a single record
    // Terminates when main memory is full or no more records to load

    // Outer loop: 
    // 1. Load records into buffer
    // 2. Sort buffer
    // 3. Write sorted buffer to temporary file
    // 4. Clear buffer
    // 5. Repeat until no more records to load

    // Terminate loop when no more records to load
    while (tempRecord.no_values != -1) {

        // Load records into the buffer
        while (counter < BUFFER_SIZE && tempRecord.no_values != -1) {
            tempRecord = Grab_Emp_Record(empin);
            if (tempRecord.no_values == -1) {
                return;
                }
            
            // Load record into buffer
            buffers.at(counter++) = tempRecord;
        }

        // Sort the buffer
        std::sort(buffers.begin(), buffers.end(), [](const Records& a, const Records& b) {
            return a.emp_record.eid < b.emp_record.eid;
        });

        // Write sorted records to temporary files (runs)
        vector<Records>::iterator it = buffers.begin();
        runFileName = "initialRun" + to_string(tempFiles.size()) + ".csv";
        tempFiles.push_back(runFileName);
        std::fstream runFile(runFileName, std::ios::out | std::ios::trunc);
        while (it != buffers.end() && it->no_values != -1) {
            if (it->emp_record.eid == -1) {
                ++it;
                continue;
            }
            runFile << it->emp_record.eid << "," << it->emp_record.ename << "," << it->emp_record.age << "," << it->emp_record.salary << endl;
            ++it;
        } // Run complete

        // Increment runs
        runs++;
        // Close run file
        runFile.close();
        // Reset counter
        counter = 0;

        // Reset buffer record objects
        for (Records& it : buffers) {
            it.emp_record.eid = -1;
            it.emp_record.ename = "";
            it.emp_record.age = -1;
            it.emp_record.salary = -1;
            it.no_values = 0;
        }

        // Clear buffer
        if (tempRecord.no_values == -1) {
            buffers.clear();
            break;
        }
    }
    
    // Function complete
    return;
}

// Utility function to merge runs and output to EmpSorted.csv
void Merge_Runs(vector<string> tempFiles = tempFiles, int iteration = 0) {
  
    // Initialize function assets

    // Priority queue to hold the smallest record from each run
    std::priority_queue<Records, std::vector<Records>, CompareRecords> mainMemory;
    Records tempRecord;
    int tmpIndex;
    std::fstream newRunFile;
    int loopIndex = 0;
    int newRuns = 0; 
    iteration++;
    int incrementer = 0;

    // Vector to hold new temporary files, for next iteration of runs
    vector<string> newTempFiles;

    int runs = tempFiles.size();
    
    // Go until there are no more runs to merge
    while (runs > 0) {
        // Open streams for as many runs as possible: up to 21 in this case
        vector<std::fstream> runFiles(BUFFER_SIZE - 1);

        // Determine how many runs to merge in this iteration, up to buffer max
        if (runs > BUFFER_SIZE - 1) {
            loopIndex = BUFFER_SIZE - 1;
        } else {
            loopIndex = runs;
        }

        // Populate the priority queue with the first record from each run
        for (int i = 0; i < loopIndex; i++) {
            // Open run file
            runFiles.at(i).open(tempFiles.at(i + incrementer), std::ios::in);
            // Load first record from run file
            tempRecord = Grab_Emp_Record(runFiles.at(i));
         
            // EOF reached for run file, skip
            if (tempRecord.no_values == -1) {
                continue;
            }
            // Set file stream index
            tempRecord.fileStreamIndex = i;
            // Add record to priority queue
            mainMemory.push(tempRecord);
        }

        // Used to increment through tempFiles, specifically
        incrementer += loopIndex;

        // We now have a priority queue with the first record from each run loaded into main memory
        // If there are guaranteed to be more than one output file for this iteration, open as one of multiple runs
        if (runs > BUFFER_SIZE - 1 || newRuns > 0 ) {
            string newFileName = "newRun_" + to_string(iteration) + "_" + to_string(newRuns) + ".csv";
            newRunFile.open(newFileName, std::ios::out | std::ios::trunc);
            newTempFiles.push_back(newFileName);
            newRuns++;
        } 
        // Otherwise, write to EmpSorted.csv
        else {
            newRunFile.open("EmpSorted.csv", std::ios::out | std::ios::trunc);
        }

        // Check if file opened
        if (!newRunFile.is_open()) {
            cerr << "Error: Could not open file" << endl;
            exit(-1);
        }
        
        // Write records to new run file as long as there are records in main memory
        while (!mainMemory.empty()) {
            
            // Retrieve smallest record
            tempRecord = mainMemory.top();
            // Then emove smallest record from priority queue
            mainMemory.pop();

            // Write smallest record to new run file
            newRunFile << tempRecord.emp_record.eid << "," << tempRecord.emp_record.ename << "," << tempRecord.emp_record.age << "," << tempRecord.emp_record.salary << endl;

            // Replace smallest record in priority queue with next record from the same run
            // Make sure to correctly associate the new record with the correct file stream
            tmpIndex = tempRecord.fileStreamIndex;
            tempRecord = Grab_Emp_Record(runFiles.at(tempRecord.fileStreamIndex));
            tempRecord.fileStreamIndex = tmpIndex;

            // EOF reached for run file
            if (tempRecord.no_values == -1) {
                continue;
            } else {
                // Add replacement record to main memory
                mainMemory.push(tempRecord);
            }
        }
        // Decrement total runs by the number processed this iteration through the loop
        runs -= loopIndex;
        // Close run file currently being written to
        newRunFile.close();
        
    } // Ends when all runs for current iteration have been merged

    // Delete temporary files from completed iteration
    for (string& file : tempFiles) {
        remove(file.c_str());
    }

    // If a new iteration of run was produced, call Merge_Runs recursively on new set of runs and new set of tempFile names
    if (newRuns > 0) {
            newRunFile.close();
            Merge_Runs(newTempFiles, iteration);  // Recursively merge runs until all runs are merged
        }  
}

int main() {
    fstream empin, SortOut;
    empin.open("Emp.csv", ios::in);
    Sort_Buffer(empin); // Create sorted runs
    Merge_Runs(tempFiles);  // Merge runs and print sorted data
}