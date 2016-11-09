#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <random>

using namespace std;

int N;                      // Number of Cities
float loc[1000][2];         // Store input
int dist[1000][1000];       // Store all pair distances

// Tabu Search Values
int aspValue;               //  Unused for now
int tabuList[1000][1000];   // 'Universal' Tabu List for all methods

// Random number generator.
random_device rd;
default_random_engine rng(rd());

// Return the current time.
static inline chrono::time_point<chrono::high_resolution_clock> now() {
    return chrono::high_resolution_clock::now();
}

void printTour(vector<int> tour) {
    for (int i = 0; i < tour.size(); i++) {
        printf("%d ", tour[i]);
    }
    printf("\n");
}

void printDistMatrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", dist[i][j]);
        }
        printf("\n");
    }
}

int compute2PtDist(float x1, float y1, float x2, float y2) {
    return round (sqrt ( pow(x1-x2, 2) + pow(y1-y2, 2) ));
}

void computeDistMatrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            dist[i][j] = dist[j][i] = compute2PtDist(loc[i][0], loc[i][1], loc[j][0], loc[j][1]);
        }
    }
}

void decrementTabu() {
    int k;
    if (N % 2 == 0) {
        k = N / 2;
    } else {
        k = N / 2 + 1;
    }
    
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            if (tabuList[i][j] > 0) {
                tabuList[i][j] = tabuList[j][i] = tabuList[i][j] - 1;
            }
        }
    }
}

/*
 Calculates tour from tour[0] to tour[N-1] and back to tour[0]
 */
int computeTourLength(vector<int> tour) {
    int totalDistance = 0;
    for (int i = 0; i < N - 1; i++) {
        totalDistance += dist[tour[i]][tour[i+1]];
    }
    totalDistance += dist[tour[N-1]][tour[0]];
    
    return totalDistance;
}

/*
 Generate random tour for random re-tours
 */
vector<int> randomTour() {
    vector<int> tour;
    for (int i = 0; i < N; i++) {
        tour.push_back(i);
    }
    random_shuffle (tour.begin(), tour.end());
    
    return tour;
}

/*
 Randomly swaps two city positions in a tour
 */
vector<int> randomTourSwap(vector<int> tour) {
    int first = rand() % N;
    int second = rand() % N;
    
    if (second == first) {
        if (first < N - 1) {
            second = first + 1;
        } else if (first > 0){
            second = first - 1;
        }
    }
    
    int temp = tour[first];
    tour[first] = tour[second];
    tour[second] = temp;
    
    return tour;
}

/*
 Algorithm taken from Kattis TSP brief
 */
vector<int> greedyTour() {
    int best;
    int curr;
    vector<int> tour(N);
    bool used[N];
    for (int i = 0; i < N; i++) {
        used[i] = false;
    }
    
    tour[0] = 0;
    used[0] = true;
    curr = 0;
    for (int i = 1; i < N; i++) {
        best = -1;
        for (int j = 0; j < N; j++) {
            if ((curr) != j && !used[j] && (best == -1 || (dist[curr][j] < dist[curr][best]) ) ) {
                best = j;
            }
        }
        tour[i] = best;
        used[best] = true;
        curr = best;
    }
    
    return tour;
}

/*
 For perturbation of tour
 Minimum number of cities for 4-opt = 8
 */
vector<int> fourOpt(vector<int> tour) {
    vector<int> newTour;
    
    uniform_int_distribution<size_t> randomOffset(1, N / 4);
    size_t A = randomOffset(rng);
    size_t B = A + randomOffset(rng);
    size_t C = B + randomOffset(rng);
    
    copy(tour.begin(), tour.begin() + A, back_inserter(newTour));
    copy(tour.begin() + C, tour.end(), back_inserter(newTour));
    copy(tour.begin() + B, tour.begin() + C, back_inserter(newTour));
    copy(tour.begin() + A, tour.begin() + B, back_inserter(newTour));
    
    return newTour;
}

vector<int> twoOptSwap(vector<int> tour, int i, int k) {
    vector<int> newTour;
    int numReverse = k - i + 1;
    
    // Essentially the same, just choosing to reverse a smaller set of tours
    if (numReverse < int(N/2)) {
        newTour.insert(newTour.end(), tour.begin(), tour.begin() + i);
        newTour.insert(newTour.end(), tour.rbegin() + (N - 1 - k), tour.rbegin() + (N - i));
        newTour.insert(newTour.end(), tour.begin() + (k + 1), tour.end());
    }
    
    else {
        newTour.insert(newTour.end(), tour.rbegin(), tour.rbegin() + (N - 1 - k));
        newTour.insert(newTour.end(), tour.begin() + i, tour.begin() + (k + 1));
        newTour.insert(newTour.end(), tour.rbegin() + (N - i), tour.rend());
    }
    
    return newTour;
}

/*
 Simply reverse the tour in-between i and k (i & k inclusive)
 */
vector<int> twoOpt(vector<int> tour, int aspRatio, int tabuTenure, chrono::time_point<chrono::high_resolution_clock> &deadline) {
    memset(tabuList, 0, sizeof tabuList);
    
    int originalTourDist, localOptDist, bestDistance;
    vector<int> originalTour = tour;
    vector<int> localOpt = tour;
    
    originalTourDist = localOptDist = computeTourLength(tour);
    
    while (chrono::high_resolution_clock::now() < deadline) {
        tour = originalTour;
        bestDistance = originalTourDist;
        
        bool isLocalOpt = false;
        
        while (!isLocalOpt && chrono::high_resolution_clock::now() < deadline) {
            isLocalOpt = true;
            
            for (int i = 0; i < N; i++) {
                for (int k = i + 1; k < N - 1; k++) {
                    int iB = (i == 0)? N - 1 : i - 1;
                    int kA = (k == N - 1)? 0 : k + 1;
                    
                    int removedEdges = bestDistance - dist[tour[i]][tour[iB]] - dist[tour[k]][tour[kA]];
                    int newDistance = removedEdges + dist[tour[iB]][tour[k]] + dist[tour[i]][tour[kA]];
                    
                    if ((newDistance < bestDistance && tabuList[i][k] == 0) || (newDistance * aspRatio < bestDistance)) {
                        tour = twoOptSwap(tour, i, k);
                        bestDistance = newDistance;
                        isLocalOpt = false;
                        
                        tabuList[i][k] = tabuTenure;
                    }
                }
            }
            decrementTabu();
        }
        
        if (bestDistance < localOptDist) {
            localOpt = tour;
            localOptDist = bestDistance;
        }
    }
    
    tour = localOpt;
    
    return tour;
}

/*
 Given 2 sets of sub-tour costs a~b~c and d~e [c & d not neccessarily linked]
 Check if a new tour cost of a~c + d~b~e is better than the previous tour cost
 a.k.a Just placing b in-between d & e is to see if it's better than placing b in-between a & c
 */
vector<int> twoHalfOpt(vector<int> tour, chrono::time_point<chrono::high_resolution_clock> &deadline) {
    bool hasImprovement = true;
    while (hasImprovement && chrono::high_resolution_clock::now() < deadline) {
        hasImprovement = false;
        for (int  i = 0; i < N - 2; i++) {
            for (int j = i + 3; j < N - 1; j++) {
                
                int a = tour[i];
                int b = tour[i+1];
                int c = tour[i+2];
                int d = tour[j];
                int e = tour[j+1];
                
                int prevSubDistance = dist[a][b] + dist[b][c] + dist[d][e];
                int newSubDistance = dist[a][c] + dist[d][b] + dist[b][e];
                
                vector<int> newTour;
                if (newSubDistance < prevSubDistance) {
                    newTour.insert(newTour.end(), tour.begin(), tour.begin() + (i+1));
                    newTour.insert(newTour.end(), tour.begin() + (i+2), tour.begin() + (j+1));
                    newTour.insert(newTour.end(), tour[i+1]);
                    newTour.insert(newTour.end(), tour.begin() + (j+1), tour.end());
                    
                    tour = newTour;
                    hasImprovement = true;
                }
            }
        }
    }
    
    return tour;
}

/*
 2 possibilities for 3 OPT
 Given 0 1 2 3 4 5 6 7 8 with i = 3 and j = 6:
 - 1st permutation = 0 1 2 6 7 8 3 4 5
 - 2nd permutation = 0 1 2 5 4 3 8 7 6
 */
vector<int> threeOpt(vector<int> tour, chrono::time_point<chrono::high_resolution_clock> &deadline) {
    bool hasImprovement = true;
    int bestDistance = computeTourLength(tour);
    while (hasImprovement && chrono::high_resolution_clock::now() < deadline) {
        hasImprovement = false;
        for (int i = 1; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                
                int removedEdges = bestDistance - dist[tour[i]][tour[i-1]] - dist[tour[j]][tour[j-1]] - dist[tour[N-1]][tour[0]];
                int tour1Dist = removedEdges + dist[tour[i-1]][tour[j]] + dist[tour[N-1]][tour[i]] + dist[tour[j-1]][tour[0]];
                int tour2Dist = removedEdges + dist[tour[i-1]][tour[j-1]] + dist[tour[N-1]][tour[i]] + dist[tour[j]][tour[0]];
                
                if (tour1Dist < tour2Dist && tour1Dist < bestDistance) {
                    vector<int> newTour1;
                    newTour1.insert(newTour1.end(), tour.begin(), tour.begin() + i);
                    newTour1.insert(newTour1.end(), tour.begin() + j, tour.end());
                    newTour1.insert(newTour1.end(), tour.begin() + i, tour.begin() + j);
                    
                    bestDistance = tour1Dist;
                    tour = newTour1;
                    hasImprovement = true;
                }
                
                else if (tour2Dist < tour1Dist && tour2Dist < bestDistance) {
                    vector<int> newTour2;
                    newTour2.insert(newTour2.end(), tour.begin(), tour.begin() + i);
                    newTour2.insert(newTour2.end(), tour.rbegin() + (N - j), tour.rbegin() + (N - i));
                    newTour2.insert(newTour2.end(), tour.rbegin(), tour.rbegin() + (N - j));
                    
                    bestDistance = tour2Dist;
                    tour = newTour2;
                    hasImprovement = true;
                }
            }
        }
    }
    
    return tour;
}

int main () {
    scanf("%d", &N);
    
    // Get input
    for (int i = 0; i < N; i++) {
        float x, y;
        scanf("%f %f", &x, &y);
        loc[i][0] = x;
        loc[i][1] = y;
    }
    
    auto start = now();
    
    computeDistMatrix();
    vector<int> tour = greedyTour();
    vector<int> shortestTour = tour;
    int shortestDistance = computeTourLength(tour);
    
    chrono::milliseconds currTime = chrono::duration_cast<chrono::milliseconds>(now() - start);
    while (currTime.count() <= 1925) {
        
        if (N < 8) {
            tour = randomTourSwap(tour);
        } else {
            tour = fourOpt(tour);
        }
        
        // TWO OPT
        auto twoOptLimit = now() + chrono::milliseconds(50);
        int twoOptTabuTenure = 20;
        int twoOptAspRatio = 15;
        tour = twoOpt(tour, twoOptAspRatio, twoOptTabuTenure, twoOptLimit);
        
        // TWO HALF OPT
        auto twoHalfOptLimit = now() + chrono::milliseconds(20);
        tour = twoHalfOpt(tour, twoHalfOptLimit);
        
        // THREE OPT
        auto threeOptLimit = now() + chrono::milliseconds(50);
        tour = threeOpt(tour, threeOptLimit);
        
        currTime = chrono::duration_cast<chrono::milliseconds>(now() - start);
        
        int currLength = computeTourLength(tour);
        if (currLength < shortestDistance) {
            shortestTour = tour;
            shortestDistance = currLength;
        }
    }
    
    //chrono::milliseconds totalTime = chrono::duration_cast<chrono::milliseconds>(now() - start);
    //cout << "Total time = " << totalTime.count() << "\n";
    //printf("Tour length = %d\n", computeTourLength(shortestTour));
    
    // Print Answer
    for (int i = 0; i < N ; i++) {
        printf("%d\n", shortestTour[i]);
    }
    
    return 0;
}
