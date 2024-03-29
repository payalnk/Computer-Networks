//Client side
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include <string.h>

int main(int argc, char* argv[])
{
/*Variables*/
	int sock,cnt=0;
	char buffer[1024],filedata[1024],filename[1024];
	struct hostent *host;
	struct sockaddr_in server;
	FILE *fp;

	host = gethostbyname(argv[1]);
/*Socket*/
	/***********Define Socket Here**********/
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == -1)
	{
		perror("Socket Failed");
		exit(1);
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(5000);
	memcpy(&server.sin_addr,host->h_addr,host->h_length);
	
/*Connect*/
	
	if(connect(sock,(struct sockaddr *)&server,sizeof(server))<0)
	{
			perror("Connect Failed");
			exit(1);
	}
	
	printf("Enter Filename:");
	scanf("%s",filename);
	
	//Send the filename of the file to receive
	if(send(sock,filename,sizeof(filename),0)<0)
	{
			perror("Send Failed");
			exit(1);
	}
	//Receive the file status. True if file exists else False
	if(recv(sock,buffer,sizeof(buffer),0)<0)
	{
			perror("Receive Failed");
			exit(1);
	}
	printf("\nFile status Received: %s\n",buffer);
	
	//Checking if received value is True i.e. File exists
	if (strcmp(buffer,"True") == 0)
	{
			printf("File Exists.\n");
			strcat(filename,"1");
			
			//Creating the file
			fp=fopen(filename, "wb");
			
			//Initialize filedata with some random value
			strcpy(filedata,"random");
			
			//Receiving file data in packets till end of file
			while(strcmp(filedata,"end") != 0)
			{
				//Receiving File Data
				if(recv(sock,filedata,sizeof(filedata),0)<0)
				{
						perror("Receive Failed");
						exit(1);
				}
				//Writing received data into a file
				/***********Define fwrite to write to file Here**********/
				fwrite(filedata,sizeof(filedata),1,fp);
				printf("Received:%d\n",cnt);
				cnt++;
			};
			fclose(fp);	 
    }  
	else
      printf("File Doesn't Exist.\n");
	return 0;
}


//Server side

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <string.h>

//To check if the file exist
int exists(const char *fname)
{
    FILE *file;
    if(file = fopen(fname, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
	/*Variables*/
	int sock,csock;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int sin_size,rval;
	
	char filename[1024],filedata[1024],buffer[1024];
	FILE *fp;
	
	/*Socket*/
	if(((sock = socket(AF_INET,SOCK_STREAM,0))<0))
	{
		perror("Failed to Create Socket");
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(5000);
	
	/*bind*/
	if(bind(sock, (struct sockaddr *)&server, sizeof(server)))
	{
		perror("Bind Failed");
		exit(1);
	}
	
	/*listen*/	
	if(listen(sock,5) == -1)
	{
			perror("Listen failed");
			exit(1);
	}
	
	/*Accept*/
	while(1)
	{
		sin_size = sizeof(client);
		csock = accept(sock,(struct sockaddr *)&client, &sin_size); 
		if(csock == -1)
		{
				perror("Accept Failed");
		}
		printf("Connetion Received from: %s:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		
		memset(filename,0, sizeof(filename));
		memset(filedata,0, sizeof(filedata));
		memset(buffer,0, sizeof(buffer));
		
		/*Receive the file name from the client*/
		if((recv(csock,filename,sizeof(filename),0))<0)
		{
				perror("Reveive Failed");
				exit(1);
		}
		printf("Filename Check:%s\n",filename);
		
		/*Call exists function to check if file exists*/
		if(exists(filename))
		{
			//Sending true for file exists as an acknowledgement
			strcpy(buffer,"True");
			if(send(csock,buffer,sizeof(buffer),0)<0)
			{
				
				perror("Send Failed");
				exit(1);
			}
			
			//Opening the file in read mode
			fp=fopen(filename, "rb");
			usleep(100000);
			/*Reading the file in chunks*/
			while (fread(filedata,sizeof(filedata),1,fp) == 1) 
			{
				/*Sending the read file data to the client*/
				if(send(csock,filedata,sizeof(filedata),0)<0)
				{
					
					perror("Send Failed");
					exit(1);
				}
			}
			usleep(100000);
			if (feof(fp))
			{
				/*Sending the last chunk of file*/
			    if(send(csock,filedata,sizeof(filedata),0)<0)
				{
					
					perror("Send Failed");
					exit(1);
					usleep(100000);
				}
				/*Sending end as data after the file ends*/
				strcpy(filedata,"end");
				if(send(csock,filedata,sizeof(filedata),0)<0)
				{
					
					perror("Send Failed");
					exit(1);
					usleep(100000);
				}
			    printf("File written successfully\n");
			}
			else
			{
				printf("File not read/written successfully\n");
			}

			fclose(fp);
			
		}
		else
		{
			/*Sending False if file doesn't exist*/
			strcpy(buffer,"False");
			if(send(csock,buffer,sizeof(buffer),0)<0)
			{
				perror("Send Failed");
				exit(1);
			}
		}
		
/*Close*/
		close(csock);
	};
return 0;
}
