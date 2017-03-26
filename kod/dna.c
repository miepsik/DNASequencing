#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<string.h>

int **graph, l, n, s;
char **words;

int calculateDistance(char* a, char* b){
    for (int i = 1; i < l; i++){
       if (strncmp(a+i, b, l-i) == 0)
          return i; 
    }
    return l;
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
                graph[i][i] = 0;
        }
    }
    for (int i = 0; i < n; i++){
        printf("\n");
        for (int j = 0; j < n; j++) {
            printf("%d ", graph[i][j]);
        }
    }

}

int main() {
    readWords("test.txt");

}
