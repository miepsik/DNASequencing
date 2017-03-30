#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<string.h>
#include<unistd.h>

#define K 3
#define Z 2

int **graph, l, n, s;
char **words;

int calculateDistance(char* a, char* b){
    for (int i = 1; i < l; i++){
       if (strncmp(a+i, b, l-i) == 0)
          return i; 
    }
    return l;
}

int calculateCostParallel(int *path, int length) {
    int result = 0;
#pragma omp parallel for reduction(+: result)
    for (int i = 0; i < length; i++) {
        result += graph[path[i]][path[i+1]];
    }
    return result;
}

int calculateCost(int *path, int length) {
    int result = 0;
    for (int i = 0; i < length; ) 
        result += graph[path[i]][path[i++]];
    return result;
}

void secondBeam(int **path, int *length) {
    int visited[K][n], cost[K], **best;
#pragma omp parallel for
    for (int i = 0; i < K; i++)
        cost[i] = calculateCost(path[i], length[i]);
    best = (int**) malloc((K+1) * sizeof(int*));
    for (int i = 0; i <= K; i++)
        best[i] = (int*) malloc(3 * sizeof(int));
#pragma omp parallel for
    for (int i = 0; i < K; i++)
        for (int j = 0; j < n; j++)
            visited[i][j] = 0;
#pragma omp parallel for
    for (int i = 0; i < K; i++)
        for (int j = 0; j < length[i]; j++)
            visited[i][path[i][j]] = 1;
}

void sortBest(int ** best){
    for (int k = K; k > 0; k--) {
        if (best[k][2] < best[k-1][2]) {
            int *tmp = best[k-1];
            best[k-1] = best[k];
            best[k] = tmp;
        } else {
            break;
        }
    }
}

void wiazkowy(){
    int visited[K][n], **path, *length, cost[K] = {l}, **best;
    length = (int*) malloc(K * sizeof(int));
    path = (int**) malloc(K * sizeof(int*));
    best = (int**) malloc((K+1) * sizeof(int*));
    for (int i = 0; i < K; i++){
        path[i] = (int*) malloc((s+2) * sizeof(int));
        best[i] = (int*) malloc(3 * sizeof(int));
    }
    best[K] = (int*) malloc(3 * sizeof(int));
    for (int i = 0; i < K; i++){
        length[i] = 1;
        cost[i] = l;
    }
#pragma omp parallel for
    for (int i = 0; i < K; i++)
        for (int j = 0; j < n; j++)
            visited[i][j] = 0;
#pragma omp parallel for
    for (int i = 0; i < K; i++){
        path[i][0] = rand()%n;
    }
    while (cost[0] < s) {
#pragma omp parallel for
        for (int i = 0; i < K; i++) {
            best[i][2] = 999999999;
            best[i][1] = -1;
            best[i][0] = -1;
        }
        for (int i = 0; i < K; i++) {
            if (cost[i] < s){
                for (int j = 0; j < n; j++) {
                    best[K][0] = i;
                    best[K][2] = cost[i] + graph[path[i][length[i]-1]][j];
                    if (visited[i][j])
                        best[K][2] += Z;
                    best[K][1] = j;
                    sortBest(best);

                    best[K][0] = i;
                    best[K][2] = cost[i] + graph[j][path[i][0]];
                    if (visited[i][j])
                        best[K][2] += Z;
                    best[K][1] = -j;
                    sortBest(best);
                }
            }
        }
        int newLength[K], newPath[K][s];
#pragma omp parallel for
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < n; j++)
                visited[i][j] = 0;
        }
        for (int i = 0; i < K; i++) {
            printf("%d: ", i);
            if (best[i][0] != -1) {
                newLength[i] = length[best[i][0]]+1;
                cost[i] = best[i][2];
                if (best[i][1] > 0) {
                    for (int j = 0; j < length[i]; j++) {
                        newPath[i][j] = path[best[i][0]][j];
                        printf("%d ", newPath[i][j]);
                        visited[i][newPath[i][j]] = 1;
                    }
                    newPath[i][newLength[i]-1] = best[i][1];
                    printf("%d\n", best[i][1]);
                } else {
                    newPath[i][0] = -best[i][1];
                    visited[i][newPath[i][0]] = 1;
                    printf("%d ", newPath[i][0]);
                    for (int j = 0; j < length[i]; j++) {
                        newPath[i][j+1] = path[best[i][0]][j];
                        visited[i][newPath[i][j+1]] = 1;
                        printf("%d ", newPath[i][j+1]);
                    }
                    printf("\n");
                }
            }
        }
#pragma omp parallel
        for (int i = 0; i < K; i++){
            length[i] = newLength[i];
            for (int j = 0; j < length[i]; j++)
                path[i][j] = newPath[i][j];
        }
    }
}

void readWords(char* file){
    FILE *fp;
    fp = fopen(file, "r");
    fscanf(fp, "%d %d %d\n", &n, &l, &s);
    printf("%d\n", n);
    graph = (int**) malloc(n * sizeof(int*));
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        graph[i] = (int*) malloc(n * sizeof(int));
    }
    words = (char**) malloc(n*sizeof(char*));
#pragma omp parallel for
    for (int i = 0; i < n; i++)
        words[i] = (char*) malloc((l+1) * sizeof(char));
    for (int i = 0; i < n; i++)
        fscanf(fp, "%s\n", words[i]);
#pragma omp parallel for
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (i != j)
                graph[i][j] = calculateDistance(words[i], words[j]);
            else 
                graph[i][i] = 99999999;
        }
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++) {
            printf("%d ", graph[i][j]);
        }
        printf("\n");
    }
    //close(&fp);
}

int main() {
    readWords("test.txt");
    wiazkowy();
    for (int i = 0; i < n; i++){
        free(graph[i]);
        free(words[i]);
    }
    free(graph);
    free(words);
}
