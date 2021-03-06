#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<string.h>
#include<unistd.h>
#include<time.h>

#define K 4
#define Z 5
#define ITER 100
#define FILES 52

int **graph, l, n, s;
char **words;

int calculateDistance(char *a, char *b){
    for (int i = 1; i < l; i++){
       if (strncmp(a+i, b, l-i) == 0)
          return i;
    }
    return l;
}

int calculateCostParallel(int *path, int length) {
    int result = 0;
#pragma omp parallel for reduction(+: result)
    for (int i = 0; i < length-1; i++) {
        result += graph[path[i]][path[i+1]];
    }
    return result;
}

int calculateCost(int *path, int length) {
    //printf("%d\n", length);
    int result = 0;
    for (int i = 0; i < length-1; i++)
        result += graph[path[i]][path[i+1]];
    return result;
}

/*void secondBeam(int **path, int *length) {
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
} */

void fullSearch(int **path, int *length){
#pragma omp parallel for
    for (int i = 0; i < 1; i++) {
        int z = s - l - calculateCost(path[i], length[i]);
        if (z < 1)
            continue;
        int combinations[z];
        for (int i = 0; i < z; i++)
            combinations[i] = 0;
        for (int i = 0; i <= z; i++) {
            while (combinations[0] < n) {
                
                combinations[z-1]++;
                int j = z-1;
                while (combinations[j] == n) {
                    combinations[j] = 0;
                    combinations[--j]++;
                }
            }
        }
    }
}

void sortBest(int **best){
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

void wiazkowy(int **path, int *length, int *cost){
    int visited[K][n], **best;
    best = (int**) malloc((K+1) * sizeof(int*));

    for (int i = 0; i < K; i++){
        best[i] = (int*) malloc(3 * sizeof(int));
    }

    best[K] = (int*) malloc(3 * sizeof(int));

#pragma omp parallel for
    for (int i = 0; i < K; i++)
        for (int j = 0; j < n; j++)
            visited[i][j] = 0;
#pragma omp parallel for
    for (int i = 0; i < K; i++)
        for (int j = 0; j < length[i]; j++)
            visited[i][path[i][j]] = 1;
    while (calculateCostParallel(path[0], length[0]) < s) {
#pragma omp parallel for
        for (int i = 0; i < K; i++) {
            best[i][2] = 999999999;
            best[i][1] = -1;
            best[i][0] = -1;
        }
        for (int i = 0; i < K; i++) {
            //printf("%d %d\n", length[i], cost[i]);
            if (calculateCostParallel(path[i], length[i]) < s){
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
            //printf("%d: ", i);
            if (best[i][0] != -1) {
                newLength[i] = length[best[i][0]]+1;
                cost[i] = best[i][2];
                if (best[i][1] > 0) {
                    for (int j = 0; j < length[i]; j++) {
                        newPath[i][j] = path[best[i][0]][j];
                        //printf("%d ", newPath[i][j]);
                        visited[i][newPath[i][j]] = 1;
                    }
                    newPath[i][newLength[i]-1] = best[i][1];
                    //printf("%d\n", best[i][1]);
                } else {
                    newPath[i][0] = -best[i][1];
                    visited[i][newPath[i][0]] = 1;
                    //printf("%d ", newPath[i][0]);
                    for (int j = 0; j < length[i]; j++) {
                        newPath[i][j+1] = path[best[i][0]][j];
                        visited[i][newPath[i][j+1]] = 1;
                       // printf("%d ", newPath[i][j+1]);
                    }
                    //printf("\n");
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

int repairPath(int *path, int length) {
    if (calculateCostParallel(path, length) + l <= s)
        return 0;
    int last = calculateCost(path, length-1) + l;
    int first = calculateCost(path+1, length-1) + l;
    if (last <= s) {
        if (first > s) {
            return -1;
        } else {
            if (last >= first) {
                return -1;
            } else {
                for (int i = 1; i < length; i++)
                    path[i-1] = path[i];
                return 1;
            }
        }
    } else {
        if (first <= s) {
            for (int i = 1; i < length; i++)
                path[i-1] = path[i];
            return 1;
        } else {
            return -1;
        }
    }

}

void readWords(char* file){
    FILE *fp;
    fp = fopen(file, "r");
    fscanf(fp, "%d %d %d\n", &s, &l, &n);
    printf("%d;%d;%d;", n, l, s);
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
            //printf("%d ", graph[i][j]);
        }
        //printf("\n");
    }
    //close(&fp);
}

void hillClimber(int *seq, int length, int ocena){
    int o,I,J,omin=ocena;
    I=J=-1;
    for(int k=0 ; k<ITER ; k++){
        for(int i=0 ; i<length ; i++){
            for(int j=0 ; j<length ; j++){
                if(i!=j){
                    o=ocena;
                    if(i+1==j){
                    //i-1   i   j   j+1
                        o+=graph[seq[j]][seq[i]] - graph[seq[i]][seq[j]];
                        if(i>0) o+=graph[seq[i-1]][seq[j]] - graph[seq[i-1]][seq[i]];
                        if(j<length-1) o+=graph[seq[i]][seq[j+1]] - graph[seq[j]][seq[j+1]];
                    }
                    else if(i-1==j){
                    //j-1   j   i   i+1
                        o+=graph[seq[i]][seq[j]] - graph[seq[j]][seq[i]];
                        if(j>0) o+=graph[seq[j-1]][seq[i]] - graph[seq[j-1]][seq[j]];
                        if(i<length-1) o+=graph[seq[j]][seq[i+1]] - graph[seq[i]][seq[i+1]];
                    }
                    else{
                        if(i>0) o+=graph[seq[i-1]][seq[j]] - graph[seq[i-1]][seq[i]];
                        if(j>0) o+=graph[seq[j-1]][seq[i]] - graph[seq[j-1]][seq[j]];
                        if(i<length-1) o+=graph[seq[j]][seq[i+1]] - graph[seq[i]][seq[i+1]];
                        if(j<length-1) o+=graph[seq[i]][seq[j+1]] - graph[seq[j]][seq[j+1]];
                    }
                    if(o<omin){
                        I=i;
                        J=j;
                        omin=o;
                    }
                }
            }
        }

        if(I!=-1 && J!=-1){
            int tmp = seq[I];
            seq[I] = seq[J];
            seq[J] = tmp;
            ocena = omin;
            I=-1;
            J=-1;
        }
        else{
            break;
        }
    }
}

void multiPathHillClimber(int **path, int *length){
    int cost[K];

#pragma omp parallel for
    for(int i=0 ; i<K ; i++){
        cost[i] = calculateCost(path[i], length[i]);
        hillClimber(path[i], length[i], cost[i]);
    }
}

void mark(int **path, int *length) {
    int max = 0, total, x;
    for (int j = 0; j < K; j++) {
        int nodes[n];
        for (int i = 0; i < n; i++)
            nodes[i] = 0;
        for (int i = 0; i < length[j]; i++)
            nodes[path[j][i]] = 1;
        int sum = 0;
        for (int i = 0; i < n; i++)
            sum += nodes[i];
        if (sum > max){
            max = sum;
            total = length[j];
            x = calculateCost(path[j], length[j]) + l;
        }
    }
    printf("%d;%d;%d;%d;%d\n", total, max, x, (100*max)/n, s-l+1-total);
}

int main() {
    printf("lp;ilość nukleotydów;l;długość;time[ms];użyte oligonukleotydy (z powtórzeniami);użyte oligonukleotydy;długość wynikowa;procent użytych;skoki\n");   
    for (int f = 0; f <= FILES; f++) {
        printf("%d;",f);
        clock_t start, end;
        char file[40];
        sprintf(file, "../dane/datasets/plik%d.txt", f);
        readWords(file);
        int **path, *length, *cost;
        path = (int**) malloc(K * sizeof(int*));
        length = (int*) malloc(K * sizeof(int));
        cost = (int*) malloc(K * sizeof(int));
        for (int i = 0; i < K; i++) {
            path[i] = (int*) malloc((2*s) * sizeof(int));
            path[i][0] = rand()%n;
            length[i] = 1;
            cost[i] = l;
        }
        start = clock();
        wiazkowy(path, length, cost);
#pragma omp parallel for
        for (int i = 0; i < K; i++){
            while (repairPath(path[i], length[i]) != 0){
                length[i]--;
            }
        }
        multiPathHillClimber(path, length);
#pragma omp parallel for
        for (int i = 0; i < K; i++){
            cost[i] = calculateCost(path[i], length[i]);
        }
        wiazkowy(path, length, cost);
#pragma omp parallel for
        for (int i = 0; i < K; i++){
            while (repairPath(path[i], length[i]) != 0) {
                length[i]--;
            }
        }
        end = clock();
        printf("%ld;", (end - start)/1000);
        mark(path, length);

#pragma omp parallel for
        for (int i = 0; i < n; i++){
            free(graph[i]);
            free(words[i]);
        }
#pragma omp parallel for
        for (int i = 0; i < K; i++) {
            free(path[i]);
        }
        free(path);
        free(length);
        free(cost);
    }
        free(graph);
        free(words);
}
