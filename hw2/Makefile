clean: master_obj worker_obj

master_obj:
	rm -rf master
worker_obj:
	rm -rf worker

worker: worker.c
	gcc -g -o worker worker.c

master: master.c
	gcc -g -o master master.c

build: worker master

check: clean build
