main:main.o protocol.o crc32.o smrt_rcv.o smrt_tx.o smrt_link.o smrt_sensor.o smrt_process.o bt_link.o bt_rcv.o bt_tx.o
	gcc -o $@ $^ -lpthread -lbluetooth
main.o:main.c protocol.h smrt_rcv.h smrt_tx.h smrt_link.h smrt_sensor.h
	gcc -c $^
protocol.o:protocol.c protocol.h crc32.h
	gcc -c $^
crc32.o:crc32.c crc32.h
	gcc -c $^
smrt_rcv.o:smrt_rcv.c protocol.h
	gcc -c $^
smrt_tx.o:smrt_tx.c protocol.h
	gcc -c $^
smrt_link.o:smrt_link.c smrt_link.h protocol.h crc32.h
	gcc -c $^
smrt_sensor.o:smrt_sensor.c smrt_sensor.h protocol.h crc32.h
	gcc -c $^
smrt_process.o:smrt_process.c protocol.h
	gcc -c $^
bt_link.o:bt_link.c protocol.h bt_link.h crc32.h
	gcc -c $^
bt_rcv.o:bt_rcv.c protocol.h crc32.h
	gcc -c $^
bt_tx.o:bt_tx.c protocol.h
	gcc -c $^

.PHONY:clean
clean:
	rm *.o main