#include <iostream>
#include <fstream>
#include <vector>
#include <climits>
#include <iomanip>
using namespace std;

struct Process {
    int pid; // Process ID
    int bt;  // Burst Time
    int art; // Arrival Time
};

// Function declarations
void FCFS(vector<Process> &processes);
void SRTF(vector<Process> &processes);
void RR(vector<Process> &processes, int quantum);
void printResults(vector<pair<int, pair<int, int>>> &gantt, vector<int> &waitingTimes, vector<int> &turnaroundTimes, int n);
double calculateCPUUtilization(vector<pair<int, pair<int, int>>> &gantt, int totalTime);

int main() {
    vector<Process> processes;
    int quantum;

    // Reading processes from file
    ifstream inputFile("processes.txt");
    if (!inputFile) {
        cerr << "Error: Unable to open file!\n";
        return 1;
    }

    int pid, bt, art;
    inputFile >> quantum; // First line contains quantum
    while (inputFile >> pid >> art >> bt) {
        processes.push_back({pid, bt, art});
    }
    inputFile.close();

    int choice;
    do {
        cout << "\nSelect Scheduling Algorithm:\n";
        cout << "1. First Come First Serve (FCFS)\n";
        cout << "2. Shortest Remaining Time First (SRTF)\n";
        cout << "3. Round Robin (RR)\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            FCFS(processes);
            break;
        case 2:
            SRTF(processes);
            break;
        case 3:
            RR(processes, quantum);
            break;
        case 4:
            cout << "Exiting...\n";
            break;
        default:
            cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 4);

    return 0;
}

void FCFS(vector<Process> &processes) {
    int n = processes.size();
    vector<int> waitingTimes(n), turnaroundTimes(n);
    vector<pair<int, pair<int, int>>> gantt;
    int currentTime = 0;

    for (int i = 0; i < n; ++i) {
        if (currentTime < processes[i].art)
            currentTime = processes[i].art;
        gantt.push_back({processes[i].pid, {currentTime, currentTime + processes[i].bt}});
        waitingTimes[i] = currentTime - processes[i].art;
        currentTime += processes[i].bt;
        turnaroundTimes[i] = waitingTimes[i] + processes[i].bt;
    }

    printResults(gantt, waitingTimes, turnaroundTimes, n);
    double cpuUtilization = calculateCPUUtilization(gantt, currentTime);
    cout << "\nCPU Utilization: " << fixed << setprecision(2) << cpuUtilization << "%\n";
}

void SRTF(vector<Process> &processes) {
    int n = processes.size();
    vector<int> remainingTime(n), waitingTimes(n, 0), turnaroundTimes(n);
    vector<pair<int, pair<int, int>>> gantt;
    for (int i = 0; i < n; ++i) remainingTime[i] = processes[i].bt;

    int completed = 0, currentTime = 0, shortest = -1, minRemaining = INT_MAX;
    int prev = -1;
    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (processes[i].art <= currentTime && remainingTime[i] > 0 && remainingTime[i] < minRemaining) {
                minRemaining = remainingTime[i];
                shortest = i;
            }
        }

        if (shortest == -1) {
            currentTime++;
            continue;
        }

        if (prev != shortest) {
            if (prev != -1) {
                gantt.back().second.second = currentTime;
            }
            gantt.push_back({processes[shortest].pid, {currentTime, 0}});
        }

        remainingTime[shortest]--;
        currentTime++;
        prev = shortest;
        if (remainingTime[shortest] == 0) {
            completed++;
            minRemaining = INT_MAX;
            int finishTime = currentTime;
            waitingTimes[shortest] = finishTime - processes[shortest].bt - processes[shortest].art;
            turnaroundTimes[shortest] = waitingTimes[shortest] + processes[shortest].bt;
        }
    }

    if (!gantt.empty()) {
        gantt.back().second.second = currentTime;
    }

    printResults(gantt, waitingTimes, turnaroundTimes, n);
    double cpuUtilization = calculateCPUUtilization(gantt, currentTime);
    cout << "\nCPU Utilization: " << fixed << setprecision(2) << cpuUtilization << "%\n";
}

void RR(vector<Process> &processes, int quantum) {
    int n = processes.size();
    vector<int> remainingTime(n), waitingTimes(n, 0), turnaroundTimes(n);
    vector<pair<int, pair<int, int>>> gantt;
    for (int i = 0; i < n; ++i) remainingTime[i] = processes[i].bt;

    int currentTime = 0;
    bool done;
    do {
        done = true;
        for (int i = 0; i < n; ++i) {
            if (remainingTime[i] > 0) {
                done = false;
                if (remainingTime[i] > quantum) {
                    if (!gantt.empty() && gantt.back().first == processes[i].pid) {
                        gantt.back().second.second += quantum;
                    } else {
                        gantt.push_back({processes[i].pid, {currentTime, currentTime + quantum}});
                    }
                    currentTime += quantum;
                    remainingTime[i] -= quantum;
                } else {
                    if (!gantt.empty() && gantt.back().first == processes[i].pid) {
                        gantt.back().second.second = currentTime + remainingTime[i];
                    } else {
                        gantt.push_back({processes[i].pid, {currentTime, currentTime + remainingTime[i]}});
                    }
                    currentTime += remainingTime[i];
                    waitingTimes[i] = currentTime - processes[i].bt - processes[i].art;
                    remainingTime[i] = 0;
                }
            }
        }
    } while (!done);

    for (int i = 0; i < n; ++i) {
        turnaroundTimes[i] = waitingTimes[i] + processes[i].bt;
    }

    printResults(gantt, waitingTimes, turnaroundTimes, n);
    double cpuUtilization = calculateCPUUtilization(gantt, currentTime);
    cout << "\nCPU Utilization: " << fixed << setprecision(2) << cpuUtilization << "%\n";
}

void printResults(vector<pair<int, pair<int, int>>> &gantt, vector<int> &waitingTimes, vector<int> &turnaroundTimes, int n) {
    cout << "\nGantt Chart Timeline Overview:\n";
    for (size_t i = 0; i < gantt.size(); i++) {
        cout << "P" << gantt[i].first << " runs from time " << gantt[i].second.first << " to " << gantt[i].second.second << ".\n";
    }

    cout << "\nPID  Waiting Time  Turnaround Time\n";

    int totalWaiting = 0, totalTurnaround = 0;
    for (int i = 0; i < n; ++i) {
        cout << "P" << (i + 1) << "\t" << waitingTimes[i] << "\t\t" << turnaroundTimes[i] << "\n";
        totalWaiting += waitingTimes[i];
        totalTurnaround += turnaroundTimes[i];
    }

    cout << "\nAverage Waiting Time: " << fixed << setprecision(2) << (double)totalWaiting / n;
    cout << "\nAverage Turnaround Time: " << fixed << setprecision(2) << (double)totalTurnaround / n;
}

double calculateCPUUtilization(vector<pair<int, pair<int, int>>> &gantt, int totalTime) {
    int activeTime = 0;
    for (const auto &entry : gantt) {
        activeTime += entry.second.second - entry.second.first;
    }
    return (double)activeTime / totalTime * 100.0;
}
