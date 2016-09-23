#cc = arm-hisiv100nptl-linux-gcc
cc = gcc

main:main.o protocol.o crc32.o smrt_rcv.o smrt_tx.o smrt_link.o smrt_sensor.o smrt_process.o bt_inquiry.o bt_rcv.o bt_tx.o misc.o
	$(cc) -o $@ $^ -lpthread -lbluetooth
main.o:main.c protocol.h smrt_rcv.h smrt_tx.h smrt_link.h smrt_sensor.h
	$(cc) -c $^
protocol.o:protocol.c protocol.h crc32.h misc.h
	$(cc) -c $^
crc32.o:crc32.c crc32.h
	$(cc) -c $^
smrt_rcv.o:smrt_rcv.c protocol.h
	$(cc) -c $^
smrt_tx.o:smrt_tx.c protocol.h
	$(cc) -c $^
smrt_link.o:smrt_link.c smrt_link.h protocol.h crc32.h
	$(cc) -c $^
smrt_sensor.o:smrt_sensor.c smrt_sensor.h protocol.h crc32.h
	$(cc) -c $^
smrt_process.o:smrt_process.c protocol.h
	$(cc) -c $^
bt_inquiry.o:bt_inquiry.c protocol.h bt_inquiry.h crc32.h
	$(cc) -c $^
bt_rcv.o:bt_rcv.c protocol.h crc32.h
	$(cc) -c $^
bt_tx.o:bt_tx.c protocol.h misc.h
	$(cc) -c $^
misc.o:misc.c
	$(cc) -c $^

.PHONY:clean
clean:
	rm *.o main