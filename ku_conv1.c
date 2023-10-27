#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "ku_input.h"

#define LAYER_NUM (N - 3 + 1)
#define MAX_TASK LAYER_NUM*LAYER_NUM
#define MAX_PROCESS MAX_TASK

typedef struct
{
        int task_count;
        int tasks[MAX_TASK];
} Job;

Job child_task[MAX_PROCESS];


int filter[3][3] = {
    {-1, -1, -1},
    {-1, 8, -1},
    {-1, -1, -1}
};

extern int input[N][N];

int all_pipefd[MAX_PROCESS][2];
int all_pid[MAX_PROCESS];


int compute_value(int child_id, int task_id)
{
        int sum = 0;
        if (task_id <= MAX_TASK)
        {
                int tx = task_id / LAYER_NUM;
                int ty = task_id % LAYER_NUM;
                for (int x = 0; x < 3; x++)
                {
                        for (int y = 0; y < 3; y++)
                        {
                                sum += input[tx + x][ty + y] * filter[x][y];
                        }
                }
        }
        else
        {
                return -99999;
        }

        printf("child %2d: pid=%d task_id=%d layer_num=%d, comput_value=%d \n", child_id, getpid(), task_id, LAYER_NUM, sum);
        return sum;
}

int main(int argc, char *argv[])
{
        if (argc != 2)
        {
                printf("usage : %s <integet>\n", argv[0]);
                return 1;
        }

        int process_num = atoi(argv[1]);
        int tasks = LAYER_NUM * LAYER_NUM;
        if (process_num > tasks)
        {
                printf("Warning: Invalid input - process should be less than total tasks");
                process_num = tasks;
        }

        int tasks_per_process = tasks / process_num;
        int more_task_process_id = tasks % process_num;
        int task_id = 0;

        for (int i = 0; i < process_num; i++)
        {
                child_task[i].task_count = (i < more_task_process_id) ? tasks_per_process + 1 : tasks_per_process;
                for (int j = 0; j < child_task[i].task_count; j++)
                {
                        child_task[i].tasks[j] = task_id++;
                }
        }

        if (task_id != tasks)
        {
                printf("Something wrong: tasks assigned is %d, but total task is %d\n", task_id, tasks);
                exit(1);
        }

        printf("child process num : %d\n", process_num);
        printf("result size = %d x %d\n", LAYER_NUM, LAYER_NUM);
        printf("task numbers : %d\n", tasks);
        printf("child task table\n");
        for (int i = 0; i < process_num; i++)
        {
                printf("child %d :", i);
                for (int j = 0; j < child_task[i].task_count; j++)
                {
                        printf("%2d ", child_task[i].tasks[j]);
                }
                printf("\n");
        }

        printf("\n");

        // ready for fork
        pid_t all_pid[process_num];
        int child_status;

        for (int p = 0; p < process_num; p++)
        {
                int pipefd[2];
                int pid;

                if (pipe(pipefd) == -1)
                {
                        perror("fd pipe");
                        exit(EXIT_FAILURE);
                }

                if ((pid = fork()) == 0)
                {
                        printf("child %2d: pid=%d task_count=%d\n", p, getpid(), child_task[p].task_count);
                        for (int t = 0; t < child_task[p].task_count; t++)
                        {
                                int result = compute_value(p, child_task[p].tasks[t]);
                                write(pipefd[1], &result, sizeof(result));
                        }
                        close(pipefd[0]);
                        close(pipefd[1]);
                        exit(0);
                }
                else
                {
                        all_pid[p] = pid;
                        all_pipefd[p][0] = pipefd[0];
                        all_pipefd[p][1] = pipefd[1];
                }
        }

        printf("**** child process folk done! ****\n");

        // read results from pipes
        int task_index = 0;
        int resultMatrix[tasks];

        for (int p = 0; p < process_num; p++)
        {
                int data;
                ssize_t byteRead;
                for (int t = 0; t < child_task[p].task_count; t++)
                {
                        byteRead = read(all_pipefd[p][0], &data, 4);
                        if (byteRead < 0)
                        {
                                perror("pipere read error");
                                return 1;
                        }
                        printf("parent read data[%2d]= %d\n", task_index, data);
                        resultMatrix[task_index++] = data;
                }

                close(all_pipefd[p][1]);
                close(all_pipefd[p][0]);
        }

        for (int i = 0; i < LAYER_NUM; i++)
        {
                for (int j = 0; j < LAYER_NUM; j++)
                {
                        printf("%3d ", resultMatrix[(LAYER_NUM * i) + j]);
                }
                printf("\n");
        }

        for (int j = 0; j < process_num; j++)
        {
                wait(NULL);
        }

        return 0;
}
