all: iCP12
	$(CC) -o $< $<.c
	strip $<
	
clean: 
	rm -f *.o *~ iCP12 
