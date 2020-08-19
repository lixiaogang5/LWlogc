###################################################################
# Function:Compile dynamic library.     	 					  #
# Version :1.0.0                          						  #
# Date    :2020-08-12                         					  #
# Author  :lixiaogang5                        					  #
# Company :HIKVISION                          					  #
# Contact :lxiaogang5@gmail.com                       			  #
###################################################################

EXEC_PRO:=LWlogc
$(EXEC_PRO):
	gcc main.c -lLWlogc -I ../ -L ./ -g -o $@

.PHONY:clean
clean:
	-@rm -f $(EXEC_PRO) *.o

