LW_LIB:=libLWLogc.so
$(LW_LIB):
	gcc LWLogc.c -fPIC -shared -lpthread

.PHONY:clean
clean:
	-@rm -f libLWLogc.so *.o

