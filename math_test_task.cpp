#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

string getFilename();
vector<vector<int>> readData(string filename);
void countWeights(const vector<vector<int>> &vectors, vector<int> &weights, int start, int finish, mutex &m);
vector<int> sumVectors(vector<int> v1, vector<int> v2);
void writeResults(vector<int> weights);

int main() {
    string filename = getFilename();

    vector<vector<int>> vectors = readData(filename);
    vector<int> weights(vectors.size() + 1, 0);
    int numOfLinearCombinations = int(pow(2, vectors.size()));

    mutex m;

    cout << "Enter the number of threads: ";
    int numOfThreads;
    cin >> numOfThreads;
    if (numOfThreads <= 0) {
        numOfThreads = 1;
    }
    if (numOfThreads > numOfLinearCombinations) {
        numOfThreads = numOfLinearCombinations;
    }

    vector<thread> threads;

    for (int i = 0; i < numOfThreads; ++i) {
        thread t(countWeights, ref(vectors), ref(weights), i * numOfLinearCombinations / numOfThreads, 
                 (i + 1) * numOfLinearCombinations / numOfThreads, ref(m));
        threads.push_back(move(t));
    }

    for (int i = 0; i < numOfThreads; ++i) {
        (threads[i]).join();
    }

    writeResults(weights);

    return 0;
}

string getFilename() {
    cout << "Enter the name of the input file: ";
    string filename;
    cin >> filename;
    return filename;
}
vector<vector<int>> readData(string filename) {
    ifstream in;
    in.open(filename);

    vector<vector<int>> vectors;
    
    if (in.is_open()) {
        string line;

        while (getline(in, line)) {
            vector<int> newVector;
            
            for (int i = 0; i < line.size(); ++i) {
                newVector.push_back(line[i] - '0');
            }

            vectors.push_back(newVector);
        }
    }

    in.close();

    return vectors;
}

void countWeights(const vector<vector<int>> &vectors, vector<int> &weights, int start, int finish, mutex &m) {
    int k = vectors.size();
    int n = vectors[0].size();

    for (int j = start; j < finish; ++j) {
        vector<int> linearCombination(n, 0);

        for (int i = 0; i < k; ++i) {
            if ((j >> i) & 1 == 1) {
                linearCombination = sumVectors(linearCombination, vectors[i]);
            }
        }

        int weight = 0;
        for (int i = 0; i < n; ++i) {
            weight += linearCombination[i];
        }

        m.lock();
        ++(weights[weight]);
        m.unlock();
    }
}

vector<int> sumVectors(vector<int> v1, vector<int> v2) {
    int n = v1.size();
    vector<int> result(n);

    for (int i = 0; i < n; ++i) {
        result[i] = (v1[i] + v2[i]) % 2;
    }

    return result;
}

void writeResults(vector<int> weights) {
    ofstream out;
    out.open("out.txt");

    int n = weights.size();
    for (int i = 0; i < n; ++i) {
        out << i << "\t" << weights[i] << endl;
    }

    out.close();
}