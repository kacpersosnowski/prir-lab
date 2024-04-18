#include "err.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define BUFFOR_SIZE 80

int *range_addr;
double *result_addr;
pid_t cpid;
double *vector;

double sum(double *vector, int n) {
  int i;
  double sum = 0.0f;
  for (i = 0; i < n; i++) {
    sum += vector[i];
  }
  return sum;
}

void on_usr1(int signal) {
  if (cpid == 0) {
    result_addr[0] += vector[0];
  }

  for (int i = range_addr[cpid] + 1; i < range_addr[cpid + 1] + 1; i++) {
    result_addr[cpid] += vector[i];
  }
}

void detach_and_remove(int shmid, void *shmaddr) {
  if (shmdt(shmaddr) < 0)
    fprintf(stderr, "shmdt");
  if (shmctl(shmid, IPC_RMID, NULL) < 0)
    fprintf(stderr, "shmctl");
}

int main(int argc, char **argv) {
  int range_shmid;
  int result_shmid;
  int vector_shmid;

  pid_t pid;
  pid_t ppid = getpid();

  int children[] = {1, 2, 4, 8, 16};

  sigset_t mask;

  struct sigaction usr1;
  sigemptyset(&mask);
  usr1.sa_handler = (&on_usr1);
  usr1.sa_mask = mask;
  usr1.sa_flags = SA_SIGINFO;
  sigaction(SIGUSR1, &usr1, NULL);
  sigemptyset(&mask);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  for (int i = 0; i < (sizeof(children) / sizeof(children[0])); i++) {
    pid_t *children_ids = malloc(sizeof(pid_t) * children[i]);

    FILE *f = fopen("vector.dat", "r");
    char buffor[BUFFOR_SIZE + 1];
    int n;
    int k;

    range_shmid =
        shmget(IPC_PRIVATE, sizeof(int) * children[i] + 1, IPC_CREAT | 0666);
    range_addr = (int *)shmat(range_shmid, NULL, 0);

    result_shmid =
        shmget(IPC_PRIVATE, sizeof(double) * children[i], IPC_CREAT | 0666);
    result_addr = (double *)shmat(result_shmid, NULL, 0);

    vector_shmid =
        shmget(IPC_PRIVATE, sizeof(double) * BUFFOR_SIZE, IPC_CREAT | 0666);
    vector = (double *)shmat(vector_shmid, NULL, 0);

    for (int j = 0; j < children[i]; j++) {
      switch (pid = fork()) {
      case -1:
        fprintf(stderr, "Blad w fork\n");
        return EXIT_FAILURE;
      case 0:
        cpid = j;
        pause();
        exit(0);
      default:
        children_ids[j] = pid;
      }
    }

    sleep(1);

    if (getpid() == ppid) {
      fgets(buffor, BUFFOR_SIZE, f);
      n = atoi(buffor);
      for (k = 0; k < n; k++) {
        fgets(buffor, BUFFOR_SIZE, f);
        vector[k] = atof(buffor);
      }
      fclose(f);

      clock_t start, end;
      double cpu_time_used;

      start = clock();

      range_addr[0] = 0;
      range_addr[children[i]] = n - 1;

      for (int j = 1; j < children[i]; j++) {
        int divisor = (n / children[i]) + 1;
        range_addr[j] = j * divisor;
      }

      for (int j = 0; j < children[i]; j++) {
        result_addr[j] = 0;
      }

      for (int j = 0; j < children[i]; j++) {
        kill(children_ids[j], SIGUSR1);
      }
      int status;

      for (int j = 0; j < children[i]; j++) {
        wait(NULL);
      }

      double sum = 0;
      for (int k = 0; k < 100000; k++) {
        for (int j = 0; j < children[i]; j++) {
          sum += result_addr[j];
        }
      }
      end = clock();
      cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
      printf("Czas potrzebny na policzenie wektora przez %d proceów to: %f "
             "sekund\n\n",
             children[i], cpu_time_used);
      detach_and_remove(range_shmid, range_addr);
      detach_and_remove(result_shmid, result_addr);
      detach_and_remove(vector_shmid, vector);

      printf("Suma to: %f\n\n", sum);
    }
  }

  return 0;
}