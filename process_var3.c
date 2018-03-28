#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc, char const *argv[]) {

	char c = '\0';
	int  fd = open(argv[1], O_RDONLY);
	int  read_size = 1;

	if (fd == -1) {
		int len = strlen(strerror(errno));
		write(STDOUT_FILENO, strerror(errno), len);
		return EXIT_FAILURE;
	}

	if(fork() == 0){ 
		while(read_size) {

			read_size = read(fd, &c, sizeof(char));
			c = toupper(c);
			write(STDOUT_FILENO, &c, sizeof(char));
			usleep(1);
		}
	}

	while(read_size) {
		read_size = read(fd, &c, sizeof(char));
		c = tolower(c);
		write(STDOUT_FILENO, &c, sizeof(char));
		usleep(1);
	} 

	close(fd);

	write(STDOUT_FILENO, "\n", sizeof(char));

	return EXIT_SUCCESS;
}
