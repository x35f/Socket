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
#define MAXLENGTH 655320

#include <iostream>
#include <map>
using namespace std;

map<string, string> filetype;
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

void url_process();

int servfd = -1;
const char response[] = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Length: %d\r\n"
	"Content-Type:%s\r\n"
	"\r\n";

void sigint_handler(int signum)
{
	printf("SIGINT detected, the server is closed.\n");
	close(servfd);
	exit(0);
}

int main()
{
	servfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	signal(SIGINT, sigint_handler);
	
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8080);
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
	
	mapinit();

	while ((clnt_sock = accept(servfd, (struct sockaddr*)&clnt_addr, &clnt_addr_size)) != -1)
	{
		char _rev[1024];
		memset(_rev, 0, sizeof(_rev));
		read(clnt_sock, _rev, sizeof(_rev)-1);
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
	}

	return 0;
}

