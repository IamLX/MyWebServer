#ifndef _HTTPCONNECT_H_
#define _HTTPCONNECT_H_

#include <sys/socket.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <string>
#include "Timer.h"


const int READ_BUFFER_SIZE=2048;

class HttpConnect
{  	
public:
	//Http处理状态
	enum HttpProcessState
	{
		ExpectRequestLine,  //请求行
		ExpectHeaders,	    //请求头
		ExpectBody,			//请求体
		GotAll				//处理完毕
	};
	//Http版本
	enum Version
	{
		Unknown,HTTP1_0,HTTP1_1
	};
	//读写函数返回码
	enum RetCode
	{
		Success,Fail,NotConnected
	};

public:
	HttpConnect(int sock_fd,int timeoiut_=500,std::string root_="");
	//读入数据
	RetCode ReadSock();
	//处理数据
	bool ProcessRequest();
	//写出
	RetCode WriteSock();
	//获取fd
	int GetFd() const {return socket_fd;}
	//添加响应报文
	bool AddResponse(int code,std::string msg="");
	//查看有多少个报文没发
	size_t NumToSend(){return writeBuffers.size();}
	//查看是否是Keep-Alive
	bool KeepAilive();
	//work的set/get
	bool IsWorking(){return working;}
	void SetWorking(bool on){working=on;}
	//计时器set/get
	void SetTimer(std::shared_ptr<Timer> t){timer=t;}
	std::shared_ptr<Timer> GetTimer()const {return timer;}
	//重置状态
	void Reset();
	~HttpConnect();
private:
	//查找\r\n
	int findCRLF(const char* str,int begin,int end);
	//分割字符串
	std::vector<std::string> parse(const char*str,int begin,int end,char ch);
	//处理请求行,ProcessRequset调用
	bool ProcessRequsetLine(int start,int end);
	//添加成功报文 code:200-299
	void AddSuccessResponse(int code,std::string msg,long fileSize);
	//添加失败报文 code:400-499
	void AddClientFailResponse(int code,std::string msg);
private:
	const static std::unordered_set<std::string> Method;				//方法集合
	const static std::unordered_map<int,std::string> CodeToMessage;		//状态码转字符串
	const static std::unordered_map<std::string,std::string> SuffixToType; //扩展名转类型
	
	bool working;			//是否处于工作状态
	int socket_fd;			
	int bytesHaveRead;		//读入缓存区大小
	char* readBuffer;		//读入缓存区
	int timeout;
	std::shared_ptr<Timer> timer;

	std::string basicPath;	//资源路径
	std::string filePath;	//文件路径
	std::string query;		//参数
	std::string method;	

	std::unordered_map<std::string,std::string> headers;
	std::queue<std::string> writeBuffers;
	HttpProcessState state;
	Version version;


	
};

#endif
