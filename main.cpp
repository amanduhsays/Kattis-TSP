#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace std;

int N;
float loc[1000][2];
int dist[1000][1000];

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
vector<int> twoOpt(vector<int> tour, chrono::time_point<chrono::high_resolution_clock> &deadline) {
    bool hasImprovement = true;
    int bestDistance = computeTourLength(tour);
    while (hasImprovement && chrono::high_resolution_clock::now() < deadline) {
        hasImprovement = false;
        for (int i = 0; i < N; i++) {
            for (int k = i + 1; k < N; k++) {
                
                int iB = i - 1;
                int kA = k + 1;
                
                if (i == 0) {
                    iB = N-1;
                }
                
                if (k == N - 1) {
                    kA = 0;
                }
                
                int removedEdges = bestDistance - dist[tour[i]][tour[iB]] - dist[tour[k]][tour[kA]];
                int newDistance = removedEdges + dist[tour[iB]][tour[k]] + dist[tour[i]][tour[kA]];
                
                if (newDistance < bestDistance) {
                    tour = twoOptSwap(tour, i, k);
                    bestDistance = newDistance;
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
    
    computeDistMatrix();
    
    vector<int> tour;
    tour = greedyTour();
    
    //auto start = now();
    
    auto twoOptLimit = now() + chrono::milliseconds(50);
    tour = twoOpt(tour, twoOptLimit);
    
    auto threeOptLimit = now() + chrono::milliseconds(50);
    tour = threeOpt(tour, threeOptLimit);
    
    //chrono::milliseconds totalTime = chrono::duration_cast<chrono::milliseconds>(now() - start);
    //cout << "Total time = " << totalTime.count() << "\n";
    //printf("Tour length = %d\n", computeTourLength(tour));
    
    // Print Answer
    for (int i = 0; i < N ; i++) {
        printf("%d\n", tour[i]);
    }
    
    return 0;
}
