#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int *thread_range;

double **C;
double **A;
double **B;
int ma, mb, na, nb;
double frobenius;

void mnoz(double **A, int a, int b, double **B, int c, int d, double **C) {
  int i, j, k;
  double s;
  for (i = 0; i < a; i++) {
    for (j = 0; j < d; j++) {
      s = 0;
      for (k = 0; k < b; k++) {
        s += A[i][k] * B[k][j];
      }
      C[i][j] = s;
    }
  }
}

void *thread_multiply(void *vargp) {
  int thread_id = (int)vargp;
  printf("Mój numer wątku to %d\n", thread_id);
  printf("Moje indeksy macierzy to %d i %d\n\n", thread_range[thread_id],
         thread_range[thread_id + 1]);
  double s = 0.0;
  int m, n;
  if (thread_id == 0) {
    for (int j = 0; j < na; j++) {
      s += A[0][j] * B[j][0];
    }
    C[0][0] = s;
    frobenius += s * s;
  }
  for (int i = thread_range[thread_id] + 1; i < thread_range[thread_id + 1] + 1;
       i++) {
    s = 0.0;
    int m = i / nb;
    int n = i % nb;
    for (int j = 0; j < na; j++) {
      s += A[m][j] * B[j][n];
    }
    C[m][n] = s;
    frobenius += s * s;
  }
}

void print_matrix(double **A, int m, int n) {
  int i, j;
  printf("[");
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      printf("%f ", A[i][j]);
    }
    printf("\n");
  }
  printf("]\n");
}
int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Zła liczba argumentów wejściowych\n");
    return -1;
  }
  
  char *file_one = argv[1];
  char *file_two = argv[2];
  int number_of_threads = atoi(argv[3]);

  FILE *fpa;
  FILE *fpb;
  int i, j;
  double x;
  pthread_t *tid = malloc(sizeof(pthread_t) * number_of_threads);

  frobenius = 0;
  fpa = fopen(file_one, "r");
  fpb = fopen(file_two, "r");
  if (fpa == NULL || fpb == NULL) {
    perror("błąd otwarcia pliku");
    exit(-10);
  }

  fscanf(fpa, "%d", &ma);
  fscanf(fpa, "%d", &na);

  fscanf(fpb, "%d", &mb);
  fscanf(fpb, "%d", &nb);

  printf("pierwsza macierz ma wymiar %d x %d, a druga %d x %d\n", ma, na, mb,
         nb);

  if (na != mb) {
    printf("Złe wymiary macierzy!\n");
    return EXIT_FAILURE;
  }

  A = malloc(ma * sizeof(double));
  for (i = 0; i < ma; i++) {
    A[i] = malloc(na * sizeof(double));
  }

  B = malloc(mb * sizeof(double));
  for (i = 0; i < mb; i++) {
    B[i] = malloc(nb * sizeof(double));
  }

  int c_matrix_size = ma * nb;
  if (c_matrix_size < number_of_threads) {
    number_of_threads = c_matrix_size;
  }

  thread_range = malloc(sizeof(int) * number_of_threads + 1);
  thread_range[0] = 0;
  thread_range[number_of_threads] = c_matrix_size - 1;
  for (i = 1; i < number_of_threads; i++) {
    int divisor = c_matrix_size / number_of_threads;
    thread_range[i] = i * divisor;
  }

  C = malloc(ma * sizeof(double));
  for (i = 0; i < ma; i++) {
    C[i] = malloc(nb * sizeof(double));
  }

  printf("Rozmiar C: %dx%d\n", ma, nb);
  for (i = 0; i < ma; i++) {
    for (j = 0; j < na; j++) {
      fscanf(fpa, "%lf", &x);
      A[i][j] = x;
    }
  }

  printf("A:\n");
  print_matrix(A, ma, mb);

  for (i = 0; i < mb; i++) {
    for (j = 0; j < nb; j++) {
      fscanf(fpb, "%lf", &x);
      B[i][j] = x;
    }
  }

  printf("B:\n");
  print_matrix(B, mb, nb);

  // mnoz(A, ma, na, B, mb, nb, C);
  for (i = 0; i < number_of_threads; i++) {
    pthread_create(&tid[i], NULL, thread_multiply, (void *)i);
  }
  for (i = 0; i < number_of_threads; i++) {
    pthread_join(tid[i], NULL);
  }

  printf("C:\n");
  print_matrix(C, ma, nb);

  printf("Frobenius: %f\n", sqrt(frobenius));

  pthread_exit(NULL);
  for (i = 0; i < na; i++) {
    free(A[i]);
  }
  free(A);

  for (i = 0; i < nb; i++) {
    free(B[i]);
  }
  free(B);

  for (i = 0; i < nb; i++) {
    free(C[i]);
  }
  free(C);

  fclose(fpa);
  fclose(fpb);

  return 0;
}