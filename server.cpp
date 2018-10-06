#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUF_LEN 1024

static pthread_t thr;
static bool thr_exit = true;
static int thr_id;
static void *treturn;
static struct sockaddr_in server_addr, client_addr;
static int server_fd, client_fd, n, n2;
static char recv_data[BUF_LEN];

void *thread_recv(void *arg);

void thread_start()
{
		thr_exit = false;
		thr_id = pthread_create(&thr, NULL, thread_recv, NULL);
}

void thread_stop()
{
		thr_exit = true;
		thr_id = pthread_join(thr,&treturn);
}

void *thread_recv(void *arg)
{
		while(!thr_exit)
		{
				if((n = recv(client_fd, recv_data, sizeof(recv_data),0)) == -1)
				{
						printf("나: %s(%d) disconnect. \n", inet_ntoa(client_addr.sin_addr),
										ntohs(client_addr.sin_port));
						thread_stop();
						close(client_fd);
				}else if(n > 0)
				{
						recv_data[n] = '\0';
						printf("\n 나)  %s\n"
									, recv_data);

				}
		}
		pthread_exit((void*)0);

}

int main(int argc, char *argv[])
{
		char chat_data[BUF_LEN];
		char temp[20];
		int len;

		if(argc != 2)
		{
				printf("Usage ./filename [PORT] \n");
				exit(0);
		}
		if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
				printf("Server : can not Open Socket.\n");
				exit(0);
		}

		memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Server : can not bind local address.\n");
        exit(0);
    }

    if(listen(server_fd, 5) < 0)
    {
        printf("Server: can not listen connect. \n");
        exit(0);
    }

    memset(recv_data, 0x00, sizeof(recv_data));
    len = sizeof(client_addr);
    printf("====[PORT] : %d ==== \n", atoi(argv[1]));
    printf("Server : waiting connection request.\n");

    while(1)
    {
		// waiting...
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);

        if(client_fd < 0)
        {
            printf("Server : accept failed. \n");
            exit(0);
        }

        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
        printf("Server : %s client connect. \n", temp);

		printf("\n %s(%d) 님이 들어오셨습니다. 나가려면 q를 누르세요.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		while(1)
		{
				thread_start();
				fgets(chat_data, sizeof(chat_data), stdin);
				if((n2 = send(client_fd, chat_data, sizeof(chat_data),0)) == -1)
				{
					break;
				}
		}
		thread_stop();
        close(client_fd);
        printf("Server : %s client closed. \n", temp);

    }
    close(server_fd);
	return 0;

}
