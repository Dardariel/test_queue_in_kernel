all:
	gcc QueueWriteConsole.c -o QueueWriteConsole
clean:
	rm -rf QueueWriteConsole
	
install:
	install -d /opt/QueueWriteConsole
	install -m 0777 -o root -g root -D ./QueueWriteConsole /opt/QueueWriteConsole
	ln -s /opt/QueueWriteConsole/QueueWriteConsole /usr/bin/QueueWriteConsole
