#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "string.h"


/*union senum 
{
	int val; //Value for SETVAL
	struct semid_ds *buf; // buffer for IPC_STAT, IPC_SET
	unsigned short int *array; // array fro GETALL, SETALL
};*/ 

struct account
{
	 
	char name1[20], name2[20]; 
	int active; 
	long acc_no; 
	char password[20]; 
	int usertype;
	double balance;  
	char action[10]; 
	int result;  
}acc1, acc2;

int main(int argc, char const *argv[])
{
/*	union senum arg; 
	key_t key = ftok(".", 67);  
	int semid = semget(key, 1, IPC_CREAT|0644); 
	//printf("%d\n", semid);
	arg.val = 1; // 2 for counting semaphore
	semctl(semid, 0, SETVAL, arg);*/ 
	//perror("");

	acc1.acc_no = 11111;  
	printf("Enter admin password : \n"); 
	scanf(" %[^\n]",acc1.password); 
;
	acc1.usertype=1; 
	int fd = open("account", O_CREAT|O_TRUNC|O_RDWR, 0644) ;
	write(fd, &acc1, sizeof(acc1)); 


	close(fd);
	
	return 0;
}
