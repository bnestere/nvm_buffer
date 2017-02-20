RM     := /bin/rm -rf

buffer: 
	gcc -g -pthread mmp_init.c mmp_thread.c mmp_user.c vecs.c -o xvecs_buffer

base:
	gcc -g base_vecs.c -o xvecs_base

thread_buffer:
	gcc -O0 -g thread_buffer.c mmp_user.c mmp_init.c mmp_thread.c -o xthread_buff -lpthread

thread_base:
	gcc -O0 -g thread_base.c -o xthread_base -lpthread

clean: 
	$(RM) xvecs_buffer xvecs_base

