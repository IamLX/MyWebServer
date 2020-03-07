#include "HttpConnect.h"
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

using namespace std;

//初始化静态变量 
//Method
const unordered_set<string> HttpConnect::Method={"GET","POST","HEAD","PUT","DELETE"};
//状态码转信息
const unordered_map<int,string> HttpConnect::CodeToMessage=
{
	{200,"OK"},
	{400,"Bad Request"},
	{403,"Forbidden"},
	{404,"Not Found"}
};
//后缀转格式
const unordered_map<string,string> HttpConnect::SuffixToType=
{
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"}
};



HttpConnect::HttpConnect(int sock_fd,int timeout_,string root_):
	working(false),
	socket_fd(sock_fd),	
	bytesHaveRead(0),
	readBuffer(new char[READ_BUFFER_SIZE]()),
	timeout(timeout_),
	timer(nullptr),
	basicPath(root_),
	state(ExpectRequestLine),
	version(Unknown)
{

}


HttpConnect::~HttpConnect()
{
	close(socket_fd);
	if(readBuffer!=nullptr)
	{
		delete[] readBuffer;
		readBuffer=nullptr;
	}
}

//读入数据,断开连接返回0，读入成功返回1，没读入字符连接未断返回-1
//由于ET一次性读完
HttpConnect::RetCode HttpConnect::ReadSock()
{
	int readSize=0;
	char* buff=readBuffer;
	while(true)
	{
		readSize=recv(socket_fd,buff,READ_BUFFER_SIZE-readSize,0);
		if(readSize==-1)
		{
			if(errno==EAGAIN||errno==EWOULDBLOCK)
			   break;
		}
		if(readSize==0)
		  return NotConnected;
		buff=buff+readSize;
		bytesHaveRead+=readSize;
	}
	return buff==readBuffer?Fail:Success;
}
//写出数据
HttpConnect::RetCode HttpConnect::WriteSock()
{
	//无数据可写
	if(writeBuffers.empty())
	{
		cout<<"[HttpConnect::WriteSock()]: no data to write"<<endl;
		return Fail;
	}
	const char* data=writeBuffers.front().data();
	size_t dataSize=writeBuffers.front().size();
	writeBuffers.pop();
	
	while(dataSize)
	{
		ssize_t nwrite=write(socket_fd,data,dataSize);
		if(nwrite==-1)
		{
			if(errno!=EAGAIN)
			{
				cout<<"[HttpConnect::WriteSock()]: write error"<<endl;
				return Fail;
			}
			break;
		}
		data+=nwrite;
		dataSize-=nwrite;
	}
	return Success;

}


//解析报文
bool HttpConnect::ProcessRequest()
{
	int index=0;
	while(state!=GotAll)
	{
		if(state==ExpectRequestLine)
		{
			int res=findCRLF(readBuffer,0,bytesHaveRead);
			if(res==bytesHaveRead)
			  return false;
			if(!ProcessRequsetLine(0,res-1))
			  return false;
			index=res+1;
			state=ExpectHeaders;
		
		}
		else if(state==ExpectHeaders)
		{
			int res=findCRLF(readBuffer,index,bytesHaveRead);
			int i;
			for(i=index;i<res-1&&readBuffer[i]!=':';i++);
			if(i==res)
			  return false;
			headers[string(readBuffer+index,readBuffer+i)]=
				string(readBuffer+i+2,readBuffer+res-1);	
			index=res+1;
			if(readBuffer[index]=='\r'&&readBuffer[index+1]=='\n')
			{
				if(index+2==bytesHaveRead)
				 state=GotAll;
				else
				  state=ExpectBody;
			}

		}
		else if(state==ExpectBody)
		{
			//to do
		}
	}
	return true;
}
//处理请求行
bool HttpConnect::ProcessRequsetLine(int start,int end)
{
	vector<string> parseVec=parse(readBuffer,start,end,' ');
	if(parseVec.size()!=3)
	  return false;
	//方法
	if(Method.find(parseVec[0])==Method.cend())
		return false;
	method=parseVec[0];
	//路径
	vector<string> parseRoute=parse(parseVec[1].data(),0,parseVec[1].size(),'?');
	if(parseRoute[0]=="/")
		filePath = basicPath + "/hello.html";
	else
		filePath = basicPath + parseRoute[0];

	//参数	  
	if(filePath.size()==2)
	  query=parseRoute[1];
	//协议版本
	if(parseVec[2]=="HTTP/1.0")
	  version=HTTP1_0;
	else if(parseVec[2]=="HTTP/1.1")
	  version=HTTP1_1;
	else
	{
		version=Unknown;
		return false;
	}
	return true;
	
}


//查找\r\n
int HttpConnect::findCRLF(const char* str,int begin,int end)
{
	int index=begin;
	while(index<end&&str[index])
	{
		if(str[index]=='\r'&&index+1<end&&str[index+1]=='\n')
		  return index+1;
		index++;
	}
	return end;
}

//分割字符串
vector<string> HttpConnect::parse(const char* str,int begin,int end,char ch)
{
	vector<string> res;
	int index=begin;
	while(index!=end)
	{
		if(str[index]==ch)
		{
			res.push_back(string(str+begin,str+index));
			begin=index=index+1;
		}
		else index++;
	}
	if(begin!=end)
		res.push_back(string(str+begin,str+end));
	return res;
}
//是否保持连接
bool HttpConnect::KeepAilive()
{
	if(headers.find("Connection")==headers.cend())
	  return false;
	return  headers.at("Connection")=="keep-alive"||(version==HTTP1_1&&headers.at("Connection")!="close");
}


//重置状态
void HttpConnect::Reset()
{
	method="";
	filePath ="";
	query="";
	memset(readBuffer,0,bytesHaveRead);
	bytesHaveRead=0;
	state=ExpectRequestLine;
	version=Unknown;
}


//添加响应报文
bool HttpConnect::AddResponse(int code,string msg)
{
	//如果当前未收到报文，直接返回
	if(bytesHaveRead<=0)
	  return false;
	//状态码200
	if(code==200)
	{
		struct stat sbuf;
		//404 NOT FOUND
		if(stat(filePath.data(),&sbuf)<0)
		{
			code=404;
			msg=CodeToMessage.at(404);
			AddClientFailResponse(code,msg);
		}
		//权限不够
		else if(!(S_ISREG(sbuf.st_mode)||!(S_IRUSR & sbuf.st_mode)))
		{
			code=403;
			msg=CodeToMessage.at(403);
			AddClientFailResponse(code,msg);
		}
		else
		{
		    if(msg.empty())
			  msg=CodeToMessage.at(200);
			AddSuccessResponse(code,msg,sbuf.st_size); 
		}
	}
	//状态码400
	else if(code==400)
	{
		if(msg.empty())
		  msg=CodeToMessage.at(400);
		AddClientFailResponse(code,msg);
	}
	else
	{
		cout<<"[HttpConnect::AddResponse()]: Unknown status code "<<code<<endl;
		return false;
	}
	//响应报文添加成功，重置状态
	return true;
}
void HttpConnect::AddClientFailResponse(int code,string msg)
{
	string output;
	
	//报文体
	string body;
	body+="<html><title>Error</title>";
	body+="<body bgcolor=\"ffffff\">";
	body+=to_string(code)+" : "+msg+"\n";
	body+="<p>"+msg+"</p>";
	body+="<hr><em>MyWebServer</em></body></html>";	
	//响应行
	output+="HTTP/1.1 "+to_string(code)+" "+msg+"\r\n";
	//报文头
	output+="Server: MyWebServer\r\n";
	output+="Content-type: text/html\r\n";
	output+="Connection: close\r\n";
	output+="Content-length "+to_string(body.size())+"\r\n\r\n";

	output+=body;
	writeBuffers.push(output);
}


void HttpConnect::AddSuccessResponse(int code,string msg,long fileSize)
{
	string output;
	output += string("HTTP/1.1 "+to_string(code)+" "+msg+"\r\n"); 
	if(KeepAilive())
	{
		output+=("Connection: Keep-Alive\r\n");
		output+=("Keep-Alive: timeout=" + std::to_string(timeout) + "\r\n");
	}
	else
	  output+="Connection: close\r\n";
	size_t index=filePath.find_last_of('.');
	if(index==string::npos)
	  output+="Content-type: text/plain\r\n";
	else
	{
		string suffix=filePath.substr(index);
		auto it=SuffixToType.find(suffix);
		if(it==SuffixToType.cend())
			output+="Content-type: text/plain\r\n";
		else
		    output+="Content-type: "+it->second+"\r\n";
	
	}
	output+="Content-length: "+to_string(fileSize)+"\r\n";
	output+="Server: MyWebServer\r\n";
	output+="\r\n";
	
	//报文体
	int srcFd=open(filePath.data(),O_RDONLY,0);
	void* mmapRet=mmap(nullptr,fileSize,PROT_READ,MAP_PRIVATE,srcFd,0);
	close(srcFd);
	if(mmapRet==(void*) -1)
	{
		munmap(mmapRet,fileSize);
		output.clear();
		code=404;
		AddClientFailResponse(code,CodeToMessage.at(400));
		return;
	}
	char* srcAddr=static_cast<char*>(mmapRet);
	output+=string(srcAddr,fileSize);
	munmap(srcAddr,fileSize);
	writeBuffers.push(output);	
}


