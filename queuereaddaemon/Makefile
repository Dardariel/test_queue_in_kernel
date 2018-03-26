all: 
	gcc queuereaddeamon.c -o QueueReadDeamon

clean:
	rm -rf QueueReadDeamon
	
install:
	install -d /opt/QueueReadDeamon
	install -m 0777 -o root -g root -D ./QueueReadDeamon /opt/QueueReadDeamon 
	install -m 0777 -o root -g root -D ./qrdd /etc/init.d 
	update-rc.d qrdd default