object=rmfsystem.h shm_mem.h
.PHONY:all
all:data_deal  plc_simulate client_pipe comtest_pipe main
main:main.o init.o read_file.o change_profile.o mini-ntpclient.o
	gcc -o main main.o init.o change_profile.o read_file.o mini-ntpclient.o -lsqlite3 -lpthread
main.o:main.c 
	gcc -c main.c
data_deal:data_deal.o shm_mem.o read_file.o
	gcc -o data_deal data_deal.o shm_mem.o read_file.o -lsqlite3 -lpthread
data_deal.o:data_deal.c $(object) read_file.h data_deal.h
	gcc -c  data_deal.c 
shm_mem.o:shm_mem.c shm_mem.h
	gcc -c shm_mem.c
read_file.o:read_file.c read_file.h
	gcc -c read_file.c
init.o:init.c init.h
	gcc -c init.c
mini-ntpclient.o:mini-ntpclient.c mini-ntpclient.h
	gcc -c mini-ntpclient.c
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
comtest_pipe:comtest_pipe.o read_file.o shm_mem.o
	gcc -o comtest_pipe comtest_pipe.o read_file.o shm_mem.o  -lpthread -lxml2
comtest_pipe.o:comtest_pipe.c $(object)
	gcc -c comtest_pipe.c -I /usr/include/libxml2/
client_pipe:client_pipe.o shm_mem.o read_file.o base64.o change_profile.o
	gcc -o client_pipe client_pipe.o change_profile.o shm_mem.o read_file.o base64.o -lpthread -lsqlite3
client_pipe.o:client_pipe.c $(object)
	gcc -c client_pipe.c
change_profile.o:change_profile.c change_profile.h
	gcc -c change_profile.c
base64.o:base64.c base64.h
	gcc -c base64.c

.PHONY:clean
clean:
	rm data_deal.o client_pipe.o comtest_pipe.o plc_simulate.o main.o
