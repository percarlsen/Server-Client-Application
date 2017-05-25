#include "Server.h"
int loop = 1;
int main(int argc, char* argv[]){
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen;
    int request_sd, sd, port, reuse = 1;
    char* server_loc;
    if(argc != 2){ //check arguments
	printf("wrong input, try: \"./Server <port number>\"\n");
	exit(0);
    }
    port = strtol(argv[1], NULL, 10);
    if(port == 0 || port == LONG_MAX || port == LONG_MIN){ //check port number
	printf("invalid port number\n");
	exit(0);
    }
    request_sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);
    if(setsockopt(request_sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0){
	perror("setsockopt");
    }
    if(bind(request_sd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in))<0){
	perror("bind");
    }
    listen(request_sd, SOMAXCONN);
    clientaddrlen = sizeof(struct sockaddr_in);
    
    //take care of ctrl c in terminal:
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = (void*)break_loop;
    sigaction(SIGINT, &action, NULL);

    server_loc = malloc(LARGE_MLC);
    pwd(server_loc);

    pid_t pid;
    //loop that waits for input from client, and calls a method to handle the input
    while(loop == 1){
       	if((sd = accept(request_sd, (struct sockaddr*)&clientaddr, &clientaddrlen)) < 0){
	    printf("Accept failed\n");
	    perror("accept");
	}
	if((pid = fork()) == 0){ //let the child process do the work
	    close(request_sd);
	    while(loop == 1){
		//printf("CONNECTED NA\n");
		char* command = read_pack(sd);
		if(loop == 1) command_check(sd, request_sd, command, server_loc);
	    }
	}else{
	    close(sd);
	}
    }
    //program terminating, free and exit: 
    free(server_loc);
    close(request_sd);
    close(sd);
    exit(0);
    return 1;
}


void break_loop(/*int sd, int reqsd*/){
    /* When the server terminates, break loops in main and make sure that all
     * processes running at that time gets shut down (avoiding zombie processes).
     */
    loop = 0;
}

void command_check(int sd, int request_sd, char* c, char* server_loc){
    /*
     * Method that checks if the command is well formed,
     * and calls the correct function to execute the given command.
     * Return an error-message if it's not well formed.
     */
    int status;
    char* output = calloc(1, LARGE_MLC); //allocate and set to 0
    char* ret;
    char* fc = c; //buffer to free later, allows the program to edit "c"

    if(c[0] == '0' && c[1] == '0' && c[2] == '1'){ //ls
	status = ls(output);
	ret = fix_return_string(output, status);
	write_pack(sd, ret);
	free(ret);
    }else if(c[0] == '0' && c[1] == '0' && c[2] == '2'){ //pwd
	status = pwd(output);
	ret = fix_return_string(output, status);
	write_pack(sd, ret);
	free(ret);
    }else if(c[0] == '0' && c[1] == '0' && c[2] == '3'){ //cd
	c += 3; //remove "003 "
	status = cd(c, output, server_loc);
	ret = fix_return_string(output, status);
	write_pack(sd, ret);
	free(ret);
    }else if(c[0] == '0' && c[1] == '0' && c[2] == '4'){ //display file info
	c += 3; //remove "004 "
	status = file_info(c, output);
	ret = fix_return_string(output, status);
	write_pack(sd, ret);
	free(ret);
    }else if(c[0] == '0' && c[1] == '0' && c[2] == '5'){ //cat
	c += 3; //remove" 005 "
	file_info(c, output);
	if(strstr(output, "directory") != NULL 
	   && strstr(output, "no such file or directory") == NULL){ //unprintable file
	    strcat(output, ", and can not be printed");
	    ret = fix_return_string(output, 1);
	    write_pack(sd, ret);
	    free(ret);
	}else{//printable file
	    memset(output, 0, LARGE_MLC);
	    char* cat_file = cat(c);
	    write_pack(sd, cat_file);
	    free(cat_file);
	}
    }else if(c[0] == '9' && c[1] == '9' && c[2] == '9'){ //exit
	close(sd);
	free(fc);
	free(output);
	free(server_loc);
	exit(0);
    }else{//if this happens, something is wrong with the protocol
	write_pack(sd, "/ERRunexpected protocol error");
    }
    free(fc);
    free(output);
}
char* fix_return_string(char* ret, int stat){
    /* Method that prepares "ret" for sending back to client. Combines "/NRM"
     * or "/ERR" with the returning string so that the client can understand
     * the message.
     */
    char* fixed_ret = calloc(1, LARGE_MLC + 4);
    if(stat == 1){ //expected message
	strcat(fixed_ret, "/NRM");
    }else{ //unexpected message
	strcat(fixed_ret, "/ERR");
    }
    strcat(fixed_ret, ret);
    return fixed_ret;
}

int ls(char* files){
    /* Method that gets all files in the current directory, and set
     * "files" to point on them. Returns 1 if success, 0 else.
     */
    int file_count, i = 0;
    struct dirent** file_list;
    char* path = malloc(LARGE_MLC);
    if(pwd(path) != 1){
	strcat(files, "ls: failed while getting current working directory");
	return 0;
    }
    
    if((file_count = scandir(path, &file_list, NULL, alphasort)) < 0){ //error
	perror("scandir");
	strcat(files, "ls: failed while getting files");
	return 0;
    }if(file_count == 2){
	//empty folder, == 2 because of pointer to current and parent dir ("." and "..")
	strcat(files, "this directory is empty");
	return 1;
    }
    free(path);
    //success, return it to client
    while(i < file_count){
	if(strcmp(file_list[i] -> d_name, ".") != 0 && strcmp(file_list[i] -> d_name, "..") != 0){
	    //ignore current "." and parent ".." directories
	    strcat(files, file_list[i] -> d_name);
	    if(i != file_count-1){
		//add newline between each file
		strcat(files, "\n");
	    }
	}
	free(file_list[i]);
	i++;
    }
    free(file_list);
    return 1;
}

int pwd(char* path){
    /*
     * Method that gets the current path, and sets "path"
     * to point on it. Returns 1 if success, 0 else.
     */
    if (getcwd(path, LARGE_MLC) == NULL){
	perror("getcwd");
	strcat(path, "pwd: failed while getting current working directory");
	return 0;
    }
    return 1;
}

int cd(char* dir, char* new_dir, char* server_loc){
    /* Method that changes directory to the directory pointed on by dir.
     * new_dir is set to point to the new directory. Returns 1 if success,
     * else 0.
     */
    char* old_dir = malloc(LARGE_MLC);
    pwd(old_dir);

    if(chdir(dir) != 0){
	strcpy(new_dir, "no such directory");
	free(old_dir);
	return 1;
    }

    if(pwd(new_dir) != 1){ //error
	strcpy(new_dir, "cd: failed getting new working directory");
	free(old_dir);
	return 0;
    }

    if(strncmp(server_loc, new_dir, strlen(new_dir)) == 0 &&
       strcmp(server_loc, new_dir) != 0){ //illegal move
	chdir(old_dir); //move back to previous location
	memset(new_dir, 0, strlen(new_dir));
	strcpy(new_dir, "permission denied");
	free(old_dir);
	return 1;
    }
    free(old_dir);
    return 1;
}
    
int file_info(char* file, char* info){
    /* Method that gets file information about "file". Information about this file
     * is pointed to by "info". Returns 1 if sucess.
     */
    struct stat sb;
    if(stat(file, &sb) == -1){
	strcat(info, "no such file or directory, check your spelling");
	return 1;
    }
    strcat(info, file);
    strcat(info, " is a ");
    switch (sb.st_mode & S_IFMT) { //get the correct filetype and return it
    case S_IFBLK:  strcat(info, "block device");     break;
    case S_IFCHR:  strcat(info, "character device"); break;
    case S_IFDIR:  strcat(info, "directory");        break;
    case S_IFIFO:  strcat(info, "FIFO/pipe");        break;
    case S_IFLNK:  strcat(info, "symlink");          break;
    case S_IFREG:  strcat(info, "regular file");     break;
    case S_IFSOCK: strcat(info, "socket");           break;
    default:       strcat(info, "unknown");          break;
    }
    return 1;
}
char* cat(char* file){
     /* Method that returns a char pointer with the whole file that was read. Returns
     * an error msg if the something went wrong.
     * Note: this method allocates memory for the returned char pointer, must be freed
     * by the caller.
     */
    int i;
    long offset;
    FILE* f = fopen(file, "r");
    if(f == NULL){ //error in fopen
	char* errormsg = calloc(1, 100);
	strcat(errormsg, "/NRMcould not open file, check your spelling");
	return errormsg;
    }
    if(fseek(f, 0, SEEK_END) != 0){ // 
	char* errormsg = calloc(1, 100);
	strcat(errormsg, "/ERRcat: unexpected error while opening file"); 
	perror("fseek");
	return errormsg;
    }

    offset = ftell(f); //get total length
    rewind(f);
    char* read_file = calloc(1, offset + 100); //+100 for buf
    strcat(read_file, "/NRM");
    char* buf = malloc(100);
    memset(buf, 0, 100);
    while(fgets(buf, 100, f) != NULL){ 
	/*read file 100 by 100 bytes, replace unprintable chars, copy buf to read_file
	  and repeat as long as the file is not empty.*/
	i = 0;
	while(buf[i] != '\0'){
	    if(buf[i] != '\n' && buf[i] != '\t' && isprint(buf[i]) == 0){
		buf[i] = '.';
	    }
	    i++;
	}
	strcat(read_file, buf);
	memset(buf, 0, 100);
    }
    free(buf);
    fclose(f);
    return read_file;
}






