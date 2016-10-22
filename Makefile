dht22: *.c
		g++ -fpermissive -Wno-write-strings -o dht22 *.c

install: dht22
		sudo cp dht22 /usr/local/bin/
		sudo chown root /usr/local/bin/dht22
		sudo chmod u+s /usr/local/bin/dht22
