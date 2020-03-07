#include "MyWebServer.h"
#include <getopt.h>
#include <string>
int main(int argc,char* argv[])
{
	//端口，连接数目，连接时长，线程池数,文件路径
	int port=4000;
	int backlog=10;
	int timeout=500;
	int threadNum=4;
	std::string filePath="../datum"; 
	
	int opt;
	const char* str="p:b:o:t:r";
	while((opt=getopt(argc,argv,str))!=-1)
	{
		switch(opt)
		{
			case 'p':
				port=atoi(optarg);
				break;
			case 'b':
				backlog=atoi(optarg);
				break;
			case 'o':
				timeout=atoi(optarg);
				break;
			case 't':
				threadNum=atoi(optarg);
				break;
			case 'r':
				filePath=optarg;
				break;
			default:
				break;
		}
	}
	

	MyWebServer server(port,backlog,timeout,threadNum,filePath);
	server.run();
	return 0;
}
