all: receiver sender
	mkdir test_directory
receiver: receiver.cpp packet.h
	g++ -o receiver receiver.cpp
sender: sender.cpp packet.h
	g++ -o sender sender.cpp
clean: 
	rm receiver
	rm sender
	rm -r test_directory