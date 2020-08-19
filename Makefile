###################################################################
# Function:Compile dynamic library.     	 		  #
# Version :1.0.0                          			  #
# Date    :2020-08-12                         			  #
# Author  :lixiaogang5                        			  #
# Company :HIKVISION                          			  #
# Contact :lxiaogang5@gmail.com                       		  #
###################################################################


LW_LIB:=libLWlogc.so
LIB_PATH:=lib
$(LW_LIB):
	gcc LWlogc.c -fPIC -shared -lpthread -o $@
	cp $@ $(LIB_PATH) -f
	-@rm -f $@


.PHONY:clean
clean:
	-@rm -f $(LW_LIB) *.o

