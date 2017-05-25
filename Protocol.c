#include "Protocol.h"
void write_pack(int sd, char* c){
    /* Method that writes the message pointed on by c.
     */
    int size = strlen(c);
    int packs = (size + PACK_SIZE)/PACK_SIZE;
    int i;
    char* char_size = malloc(20);
    memset(char_size, 0, 20);
    sprintf(char_size, "#%d", size);
    //first write call with a number indicating how many bytes is coming in total:
    write(sd, char_size, 20);
    free(char_size);
    for(i = 0; i<packs; i++){
	//write i pack of size PACK_SIZE until the buffer is empty
	write(sd, c, PACK_SIZE);
	c += PACK_SIZE;
    }
}

char* read_pack(int sd){
    /* Method that reads and returns the incoming message. Returns an error
     * message if something went wrong.
     * PS: this method allocates memory, which must be free'd by the caller.
     */
    char* size = malloc(20);
    char* s;
    memset(size, 0, 20);
    int bytes, packs, i;
    //first read gets a number which indicates the amount of bytes coming in:
    read(sd, size, 20);
    if(size[0] != '#'){
	char* err = malloc(100);
	free(size);
	strcat(err, "/ERRprotocol not recognized");
	return err;
    }
    s = size;
    size += 1;
    if((bytes = strtol(size, NULL, 10)) == 0){
	fprintf(stderr,"error in strtol\n");
	exit(-1);
    }

    packs = ((bytes + PACK_SIZE)/PACK_SIZE);
    char* command = malloc(bytes + 1);
    memset(command, 0, bytes + 1);
    char* tmp = malloc(PACK_SIZE + 1);

    for(i = 0; i < packs; i++){
	memset(tmp, 0, PACK_SIZE +1);
	//read the whole message by blocks of PACK_SIZE byte:
	read(sd, tmp, PACK_SIZE);
	strcat(command, tmp);
    }

    free(s);
    free(tmp);
    return command;
}
