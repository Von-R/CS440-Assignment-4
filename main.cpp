#include <bits/stdc++.h>
#include "record_class.h"

using namespace std;

#define BUFFER_SIZE 10
//Records buffers[BUFFER_SIZE];
vector<Records> buffers(BUFFER_SIZE);
vector<string> tempFiles;

struct CompareRecords {
    bool operator()(const Records& a, const Records& b) const {
        return a.emp_record.eid > b.emp_record.eid; // Assuming you want the smallest eid to have the highest priority
    }
};


// Utility function to load records, sort them, and write to temp files
// Results in all records being written to temp files: first run
int Sort_Buffer(fstream &empin) {
    string runFileName;
    Records tempRecord;
    int runs = 0;
    int counter = 0;
    bool eofFlag = false;

    // Load records into the buffer
    //tempRecord.printRecord();
    
    // Loop gets records from Emp.csv and loads them into the page buffer: loads a single record
    // Terminates when main memory is full or no more records to load

    // Outer loop: 
    // 1. Load records into buffer
    // 2. Sort buffer
    // 3. Write sorted buffer to temporary file
    // 4. Clear buffer
    // 5. Repeat until no more records to load
    while (tempRecord.no_values != -1) {

        // Load records into the buffer
        while (counter < BUFFER_SIZE && tempRecord.no_values != -1) {
            tempRecord = Grab_Emp_Record(empin);
            if (tempRecord.no_values == -1) {
                cout << "No more records to load..." << endl;
                cout << "End of Sort_Buffer function... \nRuns: " << runs << endl << endl;
                return runs;
                }
            
            //tempRecord.printRecord();
            buffers.at(counter++) = tempRecord;
        }

        // Sort the buffer
        cout << "Sorting buffer..." << endl;
        std::sort(buffers.begin(), buffers.end(), [](const Records& a, const Records& b) {
            return a.emp_record.eid < b.emp_record.eid;
        });

        // Print out contents of buffer by eid
        vector<Records>::iterator it = buffers.begin();
        cout << "Printing sorted buffer..." << endl;
        while (it != buffers.end() && it->no_values != -1) {
            cout << it->emp_record.eid << " " << it->emp_record.ename << " " << it->emp_record.age << " " << it->emp_record.salary << endl;
            ++it;
        }

        // Write sorted records to temporary files (runs)
        runFileName = "initialRun" + to_string(tempFiles.size()) + ".csv";
        tempFiles.push_back(runFileName);
        cout << "Writing sorted buffer to " << runFileName << "..." << endl;
        std::fstream runFile(runFileName, std::ios::out | std::ios::trunc);
        it = buffers.begin();
        while (it != buffers.end() && it->no_values != -1) {
            if (it->emp_record.eid == -1) {
                ++it;
                continue;
            }
            runFile << it->emp_record.eid << "," << it->emp_record.ename << "," << it->emp_record.age << "," << it->emp_record.salary << endl;
            ++it;
        }
        cout << endl << endl;
        runs++;
        runFile.close();
        counter = 0;

        for (Records& it : buffers) {
            it.emp_record.eid = -1;
            it.emp_record.ename = "";
            it.emp_record.age = -1;
            it.emp_record.salary = -1;
            it.no_values = 0;
        }

        if (tempRecord.no_values == -1) {
            cout << "No more records to load..." << endl;
            buffers.clear();
            break;
        }
       
    }
    cout << "End of Sort_Buffer function... \nRuns: " << runs << endl << endl;
    return runs;
}

// Utility function to merge runs and output to EmpSorted.csv
void Merge_Runs(vector<string> tempFiles = tempFiles, int iteration = 0) {
    //cout << "Merge runs entered... \nRuns: " << runs << endl << "Iteration: " << iteration << endl;
    std::priority_queue<Records, std::vector<Records>, CompareRecords> mainMemory;
    Records tempRecord;
    int tmpIndex;
    std::fstream newRunFile;
    int loopIndex = 0;
    int newRuns = 0; 
    iteration++;
    int incrementer = 0;

    vector<string> newTempFiles;

    int runs = tempFiles.size();
    

    while (runs > 0) {
        cout << "While runs > 0... Runs: " << runs << endl;
        cout << "Size of tempfiles vector: " << tempFiles.size() << endl;
        // Open streams for as many runs as possible: up to 21 in this case
        vector<std::fstream> runFiles(BUFFER_SIZE - 1);

        if (runs > BUFFER_SIZE - 1) {
            loopIndex = BUFFER_SIZE - 1;
            cout << "Runs greater than buffer size... Loop index: " << loopIndex << endl;
        } else {
            loopIndex = runs;
            cout << "Runs less than buffer size... Loop index: " << loopIndex << endl;
        }

        // Populate the priority queue with the first record from each run
        cout << "Populating priority queue..." << endl;
        for (int i = 0; i < loopIndex; i++) {
            cout << "Value of i: " << i << endl;
            cout << "Opening file: " << tempFiles.at(i) << "..." << endl;
            runFiles.at(i).open(tempFiles.at(i + incrementer), std::ios::in);
            cout << "File opened..." << endl;
            tempRecord = Grab_Emp_Record(runFiles.at(i));
            cout << "Record grabbed..." << endl;

            if (tempRecord.no_values == -1) {
                continue;
            }
            tempRecord.fileStreamIndex = i;
            mainMemory.push(tempRecord);
            //cout << "Printing temp record..." << endl;
            tempRecord = mainMemory.top();
            //tempRecord.printRecord();
        }

        incrementer += loopIndex;

        //tempFiles.clear(); // Clear temp files for next iteration

        std::priority_queue<Records, std::vector<Records>, CompareRecords> tempQueue = mainMemory; // Copy the original queue
        //cout << endl << "Printing priority queue contents..." << endl;
        /*
        while (!tempQueue.empty()) {
            Records record = tempQueue.top();
            record.printRecord(); // Process the top element
            tempQueue.pop(); // Remove the top element
        }
        */

        // We now have a priority queue with the first record from each run loaded
        
        // If there are guaranteed to be more than one output file for this iteration, open as run
        if (runs > BUFFER_SIZE - 1 || newRuns > 0 ) {
            //cout << "Writing to newRun_" << iteration << "_" << newRuns << ".csv..." << endl;
            string newFileName = "newRun_" + to_string(iteration) + "_" + to_string(newRuns) + ".csv";
            cout << "Opening new run file " << newFileName << " to add to next iterations tempFiles." << endl;
            newRunFile.open(newFileName, std::ios::out | std::ios::trunc);
            newTempFiles.push_back(newFileName);
            newRuns++;
        } 
        // Otherwise, write to EmpSorted.csv
        else {
            //cout << "Writing to EmpSorted.csv..." << endl;
            newRunFile.open("EmpSorted.csv", std::ios::out | std::ios::trunc);
        }

        if (!newRunFile.is_open()) {
            //cout << "Error opening new run file..." << endl;
            exit(1);
        }
        
        // Pop the smallest record from the priority queue and write it to new run file
        while (!mainMemory.empty()) {
            
            // Get smallest record
            tempRecord = mainMemory.top();
            // Remove smallest record from priority queue
            mainMemory.pop();

            // Write smallest record to new run file
            //cout << "Writing record " << tempRecord.emp_record.eid << " to new run file: " << endl;

            // cout << tempRecord.emp_record.eid << " " << tempRecord.emp_record.ename << " " << tempRecord.emp_record.age << " " << tempRecord.emp_record.salary << endl;
            newRunFile << tempRecord.emp_record.eid << "," << tempRecord.emp_record.ename << "," << tempRecord.emp_record.age << "," << tempRecord.emp_record.salary << endl;

            // Replace smallest record in priority queue with next record from the same run
            //cout << "Grabbing next record from run file " << tempRecord.fileStreamIndex << "..." << endl;
            //cout << "Record for " << tempRecord.emp_record.eid << " replaced by record for ";

            tmpIndex = tempRecord.fileStreamIndex;
            tempRecord = Grab_Emp_Record(runFiles.at(tempRecord.fileStreamIndex));
            tempRecord.fileStreamIndex = tmpIndex;

            // EOF reached for run file
            if (tempRecord.no_values == -1) {
                //cout << "No more records in run file " << tempRecord.fileStreamIndex << "..." << endl;
                continue;
            } else {
                // Add replacement record to main memory
                mainMemory.push(tempRecord);
                }

            // Finish print out statement above
            cout << tempRecord.emp_record.eid << endl;
        }
        //cout << "Merger loop complete. Main memory and associated runs exhausted." << endl;

        
        runs -= loopIndex;

        if (runs > 0) {
            cout << "Runs remaining: " << runs << endl;
        }

        // Stop measure for testing
        /*
        if (iteration == 2) {
            exit(1);
        }
        */

        newRunFile.close();
        
    } // Ends when all runs for current iteration have been merged

    if (newRuns > 0) {
            newRunFile.close();
            cout << "Recursive call to merge run. New runs: " << newRuns << " Iteration: " << iteration << endl;
            Merge_Runs(newTempFiles, iteration);  // Recursively merge runs until all runs are merged
        }
    else {
        cout << "All runs merged. Exiting merge run function..." << endl;
    }
        
}

void PrintSorted() {
    // This could be integrated within Merge_Runs or called after merging
    // to finalize writing to EmpSorted.csv
}

int main() {
    fstream empin, SortOut;
    //int runs = 0;
    cout << "Begin sorting..." << endl;
    empin.open("Emp.csv", ios::in);
    cout << "File opened..." << endl;
    // SortOut.open("EmpSorted.csv", ios::out | ios::app);
    cout << "Output file opened..." << endl;

    int runs = Sort_Buffer(empin); // Create sorted runs
    Merge_Runs(tempFiles);  // Merge runs and print sorted data

    // Clean-up code to delete temporary files
}


