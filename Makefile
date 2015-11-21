object=rmfsystem.h shm_mem.h
.PHONY:all
all:data_deal client comtest plc_simulate client_pipe comtest_pipe main
main:main.o
	gcc -o main main.o -lsqlite3 -lpthread
main.o:main.c NTP.h
	gcc -c main.c
data_deal:data_deal.o
	gcc -o data_deal data_deal.o -lsqlite3 -lpthread
data_deal.o:data_deal.c $(object)
	gcc -c  data_deal.c 
client:client.o
	gcc -o client client.o -lpthread -lsqlite3
client.o:client.c $(object)
	gcc -c  client.c 
comtest:comtest.o
	gcc -o comtest comtest.o -lpthread -lxml2
comtest.o:comtest.c $(object)
	gcc -c  comtest.c -I /usr/include/libxml2/ 
plc_simulate:plc_simulate.o
	gcc -o plc_simulate plc_simulate.o -lpthread -lm
plc_simulate.o:plc_simulate.c plc_simulate.h
	gcc -c plc_simulate.c
comtest_pipe:comtest_pipe.o
	gcc -o comtest_pipe comtest_pipe.o -lpthread -lxml2
comtest_pipe.o:comtest_pipe.c $(object)
	gcc -c comtest_pipe.c -I /usr/include/libxml2/
client_pipe:client_pipe.o
	gcc -o client_pipe client_pipe.o -lpthread -lsqlite3
client_pipe.o:client_pipe.c $(object)
	gcc -c client_pipe.c

.PHONY:clean
clean:
	rm data_deal.o client_pipe.o comtest_pipe.o plc_simulate.o main.o
