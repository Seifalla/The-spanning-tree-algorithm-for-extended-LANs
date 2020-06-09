#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NUMBEROFNETWORKS 20
#define NUMBEROFBRIDGES 30

struct bridge{

	int id;
        int nop;
	int type;
	int bcp;
	int configuration[3]; // Node best configuration
	char networks[NUMBEROFNETWORKS];
	int ports[NUMBEROFNETWORKS];
	int pcon[NUMBEROFNETWORKS][3]; // best configuration for each port
	int pmessage[NUMBEROFNETWORKS][3];
	int rcvd[NUMBEROFNETWORKS];
};

void send(struct bridge *b, int sender, int nob, int message);
void ports(struct bridge *b, int nob);
void receive(struct bridge *b, int nob);



int main (int argc, char **argv){

    struct bridge b[NUMBEROFBRIDGES];
    int file;
    char buffer[1];
    int bol = 0;
    int bno = 0;
    int eof = 1;
    if ((file = open(argv[1], O_RDONLY)) < 0) {
		perror(argv[1]);
		exit(-1);
    }
    read(file,buffer,1);  // read the first character in the file (assumes the bridge ID is one-digit-long)

    while(eof != 0){
	
	b[bno].nop = 0;
	    
	b[bno].id = atoi(buffer); // convert to an integer and store

// store the initial configuration

	b[bno].configuration[0] = b[bno].id;
	b[bno].configuration[1] = 0;
	b[bno].configuration[2] = b[bno].id;//edited

	read(file,buffer,1);  // read the remainder of the line, skipping empty spaces

	while(buffer[0] != '\n'){

	    if(buffer[0] != ' '){
		
		b[bno].networks[b[bno].nop] = buffer[0]; // store the network id 
		b[bno].ports[b[bno].nop] = 1; // open the port

// store the initial configuration

		b[bno].pcon[b[bno].nop][0] = b[bno].id;
		b[bno].pcon[b[bno].nop][1] = 0;
		b[bno].pcon[b[bno].nop][2] = b[bno].id;

// count the networks

		b[bno].nop = b[bno].nop + 1;
	    }
	    else{}
	    read(file,buffer,1);
	}
	bno = bno + 1;
	eof = read(file,buffer,1);
    }

    int sender;
    int sender1;
    sender = rand()%bno;
    send(b, sender, bno, 1);
    receive(b, bno);
    ports(b, bno);
    sender1 = sender;

    for(int n = 0; n < 100; n = n + 1){

	while(sender1 == sender){sender1 = (rand()%bno);} // generate a different random number so that no bridge sends twice in a row
	send(b, sender1, bno, 1);
    	receive(b, bno);
    	ports(b, bno);
	sender = sender1;// edited
    }

    for(int i = 0; i < bno; i = i + 1){

	printf("Bridge Configuration %d:\t", b[i].id);
	printf("%d\t%d\t%d\n", b[i].configuration[0], b[i].configuration[1], b[i].configuration[2]);

	for(int j = 0; j < b[i].nop; j = j + 1){

		printf("Network %c:\t port %d\t open %d\n", b[i].networks[j], j, b[i].ports[j]);
	}
    }
    return 0 ;
}


void send(struct bridge *b, int sender, int nob, int message){

	for(int port = 0; port < b[sender].nop; port = port + 1){ // go through the ports of the sender

			if(b[sender].ports[port] != 0){ // check if the port under consideration is open

				for(int receiver = 0; receiver < nob; receiver = receiver + 1){ //go through all bridges

				   if(receiver != sender){

					for(int portno  = 0; portno < b[receiver].nop; portno = portno + 1){

						if(b[sender].networks[port] == b[receiver].networks[portno]){ // check if the sender and the bridge under 
														      //consideration are connected to the same network
	
// check if the receiver's port connected to the network is open

							if(b[receiver].ports[portno] != 0){

								for(int i = 0; i < 3; i = i + 1)
									if(message == 1)
										b[receiver].pmessage[portno][i] = b[sender].pcon[port][i];
									else
										b[receiver].pmessage[portno][i] = b[sender].pmessage[port][i];
	
								b[receiver].rcvd[portno] = 1;
							}
						}
					}
				    }
				    else{}
				}
			}
	}
}

void receive(struct bridge *b, int nob){

	for(int bridge = 0; bridge < nob; bridge = bridge + 1){// go through bridges

		for(int port = 0; port < b[bridge].nop; port = port + 1) // go through the ports of each bridge

/*Upon receiving a message over a link, the node will determine whether the message is better than its configurations. There are three cases:
1. it can ignore the message, because the message is worse than its best configuration for that port 1;
2. it needs to change the best configuration at that port, because the message received is better for that port; and
3. it needs to change its own node best configuration (and the port it got the node best configuration), if < R, d + 1, S > is better than the current node best configuration, assuming the received message is < R, d, S >. This can lead to changing the best configuration of its other ports;
*/

			if(b[bridge].rcvd[port] == 1){ // if there is a message received over the port, check if it is better than the current configuration
		
				if((b[bridge].pmessage[port][0] < b[bridge].pcon[port][0]) || ((b[bridge].pmessage[port][0] == b[bridge].pcon[port][0]) 
					&& (b[bridge].pmessage[port][1] < b[bridge].pcon[port][1])) || ((b[bridge].pmessage[port][0] == b[bridge].pcon[port][0]) 
					&& (b[bridge].pmessage[port][1] == b[bridge].pcon[port][1]) && (b[bridge].pmessage[port][2] < b[bridge].pcon[port][2]))){

					for(int i = 0; i < 3; i = i + 1)
						b[bridge].pcon[port][i] = b[bridge].pmessage[port][i];
				
					b[bridge].pcon[port][1] = b[bridge].pcon[port][1] + 1;

					if((b[bridge].pcon[port][0] < b[bridge].configuration[0]) || ((b[bridge].pcon[port][0] == b[bridge].configuration[0]) 
						&& (b[bridge].pcon[port][1] < b[bridge].configuration[1])) || ((b[bridge].pcon[port][0] == b[bridge].configuration[0]) 
						&& (b[bridge].pcon[port][1] == b[bridge].configuration[1]) && (b[bridge].pcon[port][2] < b[bridge].configuration[2]))){

						for(int i = 0; i < 3; i = i + 1)
							b[bridge].configuration[i] = b[bridge].pcon[port][i];

						for(int port1 = 0; port1 < b[bridge].nop; port1 = port1 + 1){

							b[bridge].pcon[port1][0] = b[bridge].configuration[0];
						}
							b[bridge].bcp = port;
					}
					else{}
 				}
                                else{}
			}
	}
}

void ports(struct bridge *b, int nob){

	for(int bridge = 0; bridge < nob; bridge = bridge + 1){ // go through bridges

		for(int port = 0; port < b[bridge].nop; port = port + 1){ // go through the ports of each bridge

/*
For every node, it can determine that a port should be open, if 1) the sender ID in the best configuration of the port is its own; and 2) the port is the one from which it got its node best configuration.
All other ports should be closed.
*/

			if(b[bridge].pcon[port][2] == b[bridge].id || port == b[bridge].bcp)
				b[bridge].ports[port] = 1;
			else
				b[bridge].ports[port] = 0;
		}
	}
}



