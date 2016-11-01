#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int N;
float loc[1000][2];
int dist[1000][1000];

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

int computeTourLength(vector<int> tour) {
    int totalDistance = 0;
    for (int i = 0; i < N - 1; i++) {
        totalDistance += dist[tour[i]][tour[i+1]];
    }
    totalDistance += dist[tour[N-1]][tour[0]];
    
    return totalDistance;
}

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
    newTour.insert(newTour.end(), tour.begin(), tour.begin() + i);
    newTour.insert(newTour.end(), tour.rbegin() + (N - 1 - k), tour.rbegin() + (N - i));
    newTour.insert(newTour.end(), tour.begin() + (k + 1), tour.end());
    return newTour;
}

vector<int> twoOpt(vector<int> tour) {
    bool hasImprovement = true;
    while (hasImprovement) {
        hasImprovement = false;
        int bestDistance = computeTourLength(tour);
        for (int i = 0; i < N; i++) {
            for (int k = i + 1; k < N; k++) {
                vector<int> newTour = twoOptSwap(tour, i, k);
                int newDistance = computeTourLength(newTour);
                if (newDistance < bestDistance) {
                    tour = newTour;
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
    tour = twoOpt(tour);
    
    printf("Tour length = %d\n", computeTourLength(tour));
    
    // Print Answer
    for (int i = 0; i < N ; i++) {
        printf("%d\n", tour[i]);
    }
    
    return 0;
}
