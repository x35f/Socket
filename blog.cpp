#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include<pthread.h>
#include<assert.h>
using namespace std;
#define MAXLENGTH 655320
#define buf_length 655320
int servfd = -1;
int thread_connecting=0;
map<string, string> filetype;
const char response[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Length: %d\r\n"
	"Content-Type:%s\r\n"
	"\r\n";


void mapinit();
void sigint_handler(int signum);
void *mt_thread(void * sock_fd);

struct clnt{
	int fd;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
};



int main()
{
	servfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	signal(SIGINT, sigint_handler);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8080);
	int sock_close=1;
	if(setsockopt(servfd,SOL_SOCKET,SO_REUSEADDR,&sock_close,sizeof(sock_close))==-1)
	{
		printf("socket port occupied\n");
		exit(1);
	}
	int ret = bind(servfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == -1)
	{
		printf("bind failed. Please try it later.\n");
		return 0;
	}
	listen(servfd, 20);

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int clnt_sock = -1;
	int clnt_sock_length=-1;
	mapinit();
		while(1)
		{
			printf("server available for new connection\n");
			pthread_t thread_fd;
			clnt_sock=accept(servfd, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
			assert(clnt_sock!=-1);
			printf("new connection request confirmed, establishing....\n");
			clnt new_clnt;
			new_clnt.fd=clnt_sock;
			new_clnt.clnt_addr=clnt_addr;
			new_clnt.clnt_addr_size=clnt_addr_size;
			assert(pthread_create(&thread_fd,NULL,mt_thread,(void *)(&new_clnt))==0);

		}

	return 0;
}

void *mt_thread(void* new_clnt_fd)
{
	thread_connecting++;
	clnt new_clnt=*((clnt *)new_clnt_fd);
	int clnt_sock=new_clnt.fd;
	struct sockaddr_in clnt_addr=new_clnt.clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	char _rev[buf_length];
	memset(_rev,0,sizeof(_rev)-1);
	read(clnt_sock,_rev,buf_length);
	printf("data recieved from client %d:\n %s \n ",clnt_sock,_rev);
		char *_header;
		_header = strtok(_rev, "\r\n");
		char *_url;
		_url = strtok(_header, " ");
		_url = strtok(NULL, " ");
		_url = (char *)_url + 1;
		int fd = open(_url, O_RDONLY);// Open source file
		char _body[MAXLENGTH], buf[MAXLENGTH];
		memset(_body, 0, sizeof(_body));
		int body_size;
		string _type;
		_type = "text/html";
		if (fd == -1)
		{
			strcpy(_body, "<html> 404 </html>\0");
			body_size = strlen(_body);
		}
		else
		{
			body_size = read(fd, _body, MAXLENGTH);
			string ftype, str_url;
			str_url = _url;
			string::size_type query_find;
			query_find = str_url.find('?');
			if (query_find == str_url.npos)
			{
				ftype = str_url.substr(str_url.find_last_of('.') + 1);
				map<string, string>::iterator it;
				it = filetype.find(ftype);
				if (it == filetype.end())
				{
					printf("An Error happened!\n");
					strcpy(_body, "<html> 404 </html>\0");
					body_size = strlen(_body);
				}
				else
				{
					_type = filetype[ftype];
				}
			}
			else
			{
				strcpy(_body, "<html> 404 </html>\0");
				body_size = strlen(_body);
			}
		}
		sprintf(buf, response, body_size, _type.c_str());
		int header_size = strlen(buf);
		memcpy(buf + header_size, _body, body_size);
		write(clnt_sock, buf, header_size + body_size);
			close(clnt_sock);
	pthread_exit(NULL);
	printf("thread_ended\n");
}

void mapinit()
{
	filetype.insert(pair<string, string>("html", "text/html"));
	filetype.insert(pair<string, string>("ico", "image/ico"));
	filetype.insert(pair<string, string>("css", "text/css"));
	filetype.insert(pair<string, string>("js", "application/js"));
	filetype.insert(pair<string, string>("json", "application/json"));
	filetype.insert(pair<string, string>("jpg", "image/jpg"));
	filetype.insert(pair<string, string>("jpeg", "image/jpeg"));
	filetype.insert(pair<string, string>("png", "image/png"));
	filetype.insert(pair<string, string>("gif", "image/gif"));
	filetype.insert(pair<string, string>("pdf", "application/pdf"));
	return;
}
void sigint_handler(int signum)
{
	printf("SIGINT detected, the server is closed.\n");
	close(servfd);
	exit(0);
}
