#ifndef   _COMMON_H_
#define	  _COMMON_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/stat.h>
#include <fcntl.h>


//Semaphore
#define MUTEX 	0
#define FULL 		1

#define SEM_NUM 	2

//constant define
#define SHM_SIZE 1024

//#define KEY_NUM 1000 


key_t get_key();

int get_shmid(key_t key);

int get_semid(key_t key,int sem_num);

void set_sem(int semid,int sem_name,int value);

void rm_sem(int semid,int sem_num);

void  P(int semid,int sem_name);

void  V(int semid,int sem_name);

#endif