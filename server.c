#include <sys/types.h>
#include "sys/stat.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

typedef struct account 
{
	char name1[20], name2[20];
	int active;  
	long acc_no; 
	char password[20]; 
	int usertype;
	double balance;
	char action[10]; 
	int result;    
}account;

typedef struct transaction 
{	 
	double amount; 
	double balance;
	char action[10]; 
	    
}trans;

int getslfromaccno(long accno)
{
	int fd = open("account", O_RDONLY); 
	account acc; int count=1; 
	int size = read(fd, &acc, sizeof(acc)); 
	while(size!=0)
	{
		size = read(fd, &acc, sizeof(acc)); 
		if(acc.acc_no == accno && acc.active==1)
			return count; 
		count++; 
	}
	return -1; 
}

int getfreespace(long accno)
{
	int fd = open("account", O_RDONLY); 
	account acc; int count=1; 
	int size = read(fd, &acc, sizeof(acc)); 
	while(size!=0)
	{
		size = read(fd, &acc, sizeof(acc)); 
		if(acc.active==0)
			return count; 
		count++; 
	}
	return count--; 
}

int main(int argc, char const *argv[])
{
	struct sockaddr_in server, client; 
	int sd, sz, nsd; 
	sd = socket(AF_UNIX, SOCK_STREAM, 0); 

	server.sin_family = AF_UNIX; 
	server.sin_addr.s_addr=INADDR_ANY; 
	server.sin_port=htons(7046); // <------------------------------------------- change port  

	bind(sd, (void *)(&server), sizeof(server)); 

	listen(sd, 5); 
	int size = sizeof(client); 
	
	while(1)
	{ 
		nsd = accept(sd, (void *)(&client), &size); 	
		if(!fork())
		{
			close(sd); 
			account acccli, accser;
			read(nsd, &acccli, sizeof(acccli));
			printf("Connection Initialized....\n"); 
			printf("Message from Client : %s\n", acccli.action); 
			

			if(strcmp(acccli.action,"LOGIN")==0){/////////////////////////// LOGIN 		

				int fd = open("account", O_RDONLY);
				read(fd, &accser, sizeof(accser)); 
				if(acccli.usertype==1){ 
					if(strcmp(acccli.password,accser.password)==0) 
					{	acccli.result = 1; printf("Admin Login Success\n"); } 
					else 
					{	acccli.result = 0; printf("Admin Login Failure\n"); } 
				}
				else 
				{
					int slno = getslfromaccno(acccli.acc_no);
					if(slno != -1) 
					{lseek(fd, slno*sizeof(account), SEEK_SET); 
					read(fd, &accser, sizeof(accser)); 
					if(acccli.acc_no==accser.acc_no && strcmp(acccli.password,accser.password)==0 && accser.usertype==acccli.usertype)
						{ acccli.result = 1; printf("User login success \n"); } 
					else 
						{ acccli.result = 0; printf("User login failure \n"); }
					} 

					else acccli.result = -1;  
				}
				close(fd); 
				write(nsd, &acccli, sizeof(acccli));
			
			}
			if(strcmp(acccli.action,"CREATE")==0){ //////////////////////////CREATE 

				int slno = getslfromaccno(acccli.acc_no); 
				if(slno == -1) {
				int fd = open("account", O_WRONLY);
				int sl = getfreespace(acccli.acc_no); 
				lseek(fd, sl*sizeof(account), SEEK_SET); 
				acccli.active = 1; 
				write(fd, &acccli, sizeof(acccli)); 
				printf("New Account Created\n");
				char buf1[20] = "passbook"; 
				char buf2[10]; 
				sprintf(buf2, "%ld" ,acccli.acc_no); 
				strcat(buf1,buf2); 
				open(buf1, O_CREAT|O_TRUNC|O_RDWR, 0644); 
				close(fd); 	
				acccli.result = 1;} 
				else acccli.result = 0;  
				write(nsd, &acccli, sizeof(acccli));

			} 
			if(strcmp(acccli.action,"MODIFY")==0){/////////////////////////////MODIFY

				int fd = open("account", O_WRONLY);
				if(acccli.acc_no != 11111) { 				
				int slno = getslfromaccno(acccli.acc_no); 
				if(slno!=-1) { 
				acccli.active = 1; 
				lseek(fd, slno*sizeof(account), SEEK_SET ); 
				write(fd, &acccli, sizeof(acccli));
				printf("Account modified\n");
				acccli.result = 1; } 
				else acccli.result = 0;  
				write(nsd, &acccli, sizeof(acccli)); }

				else {	
						write(fd, &acccli, sizeof(acccli)); 
						printf("Admin password modified\n");
						acccli.result = 1;
						write(nsd, &acccli, sizeof(acccli));

					}

			} 
			if(strcmp(acccli.action,"SEARCH")==0){ ///////////////////////////////SEARCH 

				printf("Searching for %ld \n", acccli.acc_no); 
				int fd = open("account", O_RDONLY);
				int slno = getslfromaccno(acccli.acc_no);
				if(slno != -1)
				{lseek(fd, slno*sizeof(account), SEEK_SET ); 
				read(fd, &accser, sizeof(accser));
				accser.result = 1; 
				write(nsd, &accser, sizeof(accser));
				}
				else{ acccli.result = 0; write(nsd, &acccli, sizeof(acccli)); }
				printf("%d\n", slno); 

			}

			if(strcmp(acccli.action,"DELETE")==0) { ///////////////////////////////DELETE

				printf("Searching for %ld \n", acccli.acc_no); 
				int fd = open("account", O_WRONLY);
				int slno = getslfromaccno(acccli.acc_no);
				if(slno != -1)
				{lseek(fd, slno*sizeof(account), SEEK_SET ); 
				accser.active = 0; 
				write(fd, &accser, sizeof(accser)); 
				acccli.result = 1; 
				}				
				else acccli.result = 0;
				write(nsd, &acccli, sizeof(acccli)); 
				printf("%d\n", slno);

			}

			if(strcmp(acccli.action,"DEPOSIT")==0){/////////////////////////////////DEPOSIT

				trans t;
				char buf1[20] = "passbook"; 
				char buf2[10]; 
				sprintf(buf2, "%ld" ,acccli.acc_no);
				strcat(buf1,buf2); 
				int fd = open("account", O_RDWR);
				int fd2 = open(buf1, O_WRONLY|O_APPEND); 
				int slno = getslfromaccno(acccli.acc_no); 
				if(slno != -1)
				{
					lseek(fd, slno*sizeof(account), SEEK_SET); 
					read(fd, &accser, sizeof(accser));
					lseek(fd, slno*sizeof(account), SEEK_SET);
					t.amount = acccli.balance;  
					accser.balance = accser.balance + acccli.balance; 
					t.balance = accser.balance; 
					write(fd, &accser, sizeof(accser)); 
					acccli.result = 1; 
					acccli.balance = accser.balance;
					strcpy(t.action, "DEPOSIT");
					write(fd2, &t, sizeof(t)); 

				}
				else acccli.result = 0; 
				write(nsd, &acccli, sizeof(acccli)); 
			}

			if(strcmp(acccli.action,"WITHDRAW")==0){////////////////////////////////WITHDRAW

				trans t;
				char buf1[20] = "passbook"; 
				char buf2[10]; 
				sprintf(buf2, "%ld" ,acccli.acc_no); 
				strcat(buf1,buf2); 
				int fd = open("account", O_RDWR);
				int fd2 = open(buf1, O_WRONLY|O_APPEND); 
				int slno = getslfromaccno(acccli.acc_no); 
				if(slno != -1)
				{
					lseek(fd, slno*sizeof(account), SEEK_SET); 
					read(fd, &accser, sizeof(accser));
					 
					if(acccli.balance <= accser.balance)
					{	
						strcpy(t.action,"WITHDRAW"); 
						t.amount = acccli.balance; 
						accser.balance = accser.balance - acccli.balance;
						t.balance = accser.balance;  
						lseek(fd, slno*sizeof(account), SEEK_SET);
						write(fd, &accser, sizeof(accser)); 
						acccli.result = 1; 
						acccli.balance = accser.balance;
						write(fd2, &t, sizeof(t));  
					}
					else acccli.result = 0;  		

				}
				else acccli.result = 0; 
				write(nsd, &acccli, sizeof(acccli)); 
			}

			if(strcmp(acccli.action,"BALANCE")==0){/////////////////////////////////BALANCE

				int fd = open("account", O_RDONLY);
				int slno = getslfromaccno(acccli.acc_no); 
				if(slno != -1)
				{
					lseek(fd, slno*sizeof(account), SEEK_SET); 
					read(fd, &accser, sizeof(accser));				
					acccli.result = 1; 
					acccli.balance = accser.balance;		

				}
				else acccli.result = 0; 
				write(nsd, &acccli, sizeof(acccli)); 
			}

			if(strcmp(acccli.action,"DETAILS")==0){/////////////////////////////////DETAILS
				trans t; 
				write(nsd, &acccli, sizeof(acccli)); 
				char buf1[20] = "passbook"; 
				char buf2[10]; 
				sprintf(buf2, "%ld" ,acccli.acc_no); 
				strcat(buf1,buf2); 
				int fd = open(buf1, O_RDONLY);
				

				int size = read(fd, &t, sizeof(trans));
				printf("%s\n", t.action); 
				while(size!=0)
				{
					write(nsd, &t, sizeof(trans)); 
 					
					size = read(fd, &t, sizeof(t));
					printf("%s\n", t.action);  
				}
				 

			}

			if(strcmp(acccli.action,"LOCK")==0){//////////////////////////////////////LOCK

				int fd = open("account", O_RDWR);
				int slno = getslfromaccno(acccli.acc_no); 
				lseek(fd, slno*sizeof(account), SEEK_SET); 
				read(fd, &accser, sizeof(accser));
				lseek(fd, slno*sizeof(account), SEEK_SET);

				if(strcmp(accser.action,"LOCK")==0) acccli.result = 0; 
				else {strcpy(accser.action, "LOCK"); write(fd,&accser,sizeof(accser)); acccli.result = 1; }
				write(nsd, &acccli, sizeof(acccli)); 
			}

			if(strcmp(acccli.action,"UNLOCK")==0){////////////////////////////////////UNLOCK

				int fd = open("account", O_RDWR);
				int slno = getslfromaccno(acccli.acc_no); 
				lseek(fd, slno*sizeof(account), SEEK_SET); 
				read(fd, &accser, sizeof(accser)); 
				lseek(fd, slno*sizeof(account), SEEK_SET);
				strcpy(accser.action, "UNLOCK"); 
				write(fd,&accser,sizeof(accser));
				write(nsd, &acccli, sizeof(acccli)); 
			} 
			
			exit(0); 
			
		}
		else
		{
			close(nsd); 
		}
	} 

	return 0;
}