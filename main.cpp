#include <bits/stdc++.h>
#include "record_class.h"

using namespace std;

#define BUFFER_SIZE 22
//Records buffers[BUFFER_SIZE];
vector<Records> buffers(BUFFER_SIZE);
vector<string> tempFiles;

// Utility function to load records, sort them, and write to temp files
void Sort_Buffer(fstream &empin) {
    cout << "Sort buffer entered..." << endl;
    //std::fstream empin(inputFile, std::ios::in);
    string runFileName;
    Records tempRecord;
    int counter = 0;

    cout << "Loading records into buffer..." << endl;
    // Load records into the buffer
    cout << "test print1" << endl;
    tempRecord.printRecord();
    
    // Loop gets records from Emp.csv and loads them into the page buffer: loads a single record
    // Terminates when main memory is full or no more records to load
    while (tempRecord.no_values != -1) {
    while (counter < BUFFER_SIZE && tempRecord.no_values != -1) {
        cout << "test print2" << endl;
        tempRecord = Grab_Emp_Record(empin);
        if (tempRecord.no_values == -1) {
            cout << "No more records to load..." << endl;
            break;
            }
        
        tempRecord.printRecord();
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
    runFileName = "run" + to_string(tempFiles.size()) + ".csv";
    tempFiles.push_back(runFileName);
    cout << "Writing sorted buffer to " << runFileName << "..." << endl;
    std::fstream runFile(runFileName, std::ios::out);
    it = buffers.begin();
    while (it != buffers.end() && it->no_values != -1) {
        runFile << it->emp_record.eid << "," << it->emp_record.ename << "," << it->emp_record.age << "," << it->emp_record.salary << endl;
        ++it;
    }
    runFile.close();
    counter = 0;
    }
}

// Utility function to merge runs and output to EmpSorted.csv
void Merge_Runs() {
    // Merge sorted runs using a multi-way merge algorithm

    /*
    Open streams for as many runs as possible: up to 21 in this case. There are 18 runs

    Find a way to track each runs attachment to specific position in the buffer: associate each run with a buffer position

    
    */



    // Output the final sorted list to EmpSorted.csv
}

void PrintSorted() {
    // This could be integrated within Merge_Runs or called after merging
    // to finalize writing to EmpSorted.csv
}

int main() {
    fstream empin, SortOut;
    cout << "Begin sorting..." << endl;
    empin.open("Emp.csv", ios::in);
    cout << "File opened..." << endl;
    SortOut.open("EmpSorted.csv", ios::out | ios::app);
    cout << "Output file opened..." << endl;

    Sort_Buffer(empin); // Create sorted runs
    // Merge_Runs();  // Merge runs and print sorted data

    // Clean-up code to delete temporary files
}


