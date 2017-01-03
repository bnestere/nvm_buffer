RM     := /bin/rm -rf

buffer: 
	gcc -g -pthread mmp_init.c mmp_thread.c mmp_user.c vecs.c -o xvecs_buffer

base:
	gcc -g base_vecs.c -o xvecs_base

clean: 
	$(RM) xvecs_buffer xvecs_base

