cc = g++
std = c++11
prom = Server
deps = MyWebServer.h HttpConnect.h Epoll.h ThreadPool.h Timer.h TimerManager.h
obj = main.o MyWebServer.o HttpConnect.o Epoll.o ThreadPool.o Timer.o TimerManager.o

$(prom): $(obj)
	$(cc) -o $(prom) $(obj) -lpthread

%.o: %.cpp $(deps)
	$(cc) -g -Wall -c $< -o $@ -std=$(std) -lpthread

clean:
	rm -f $(obj) $(prom)
