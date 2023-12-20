#include <stdio.h>
#include <syslog.h>

int main(int argc, char * argv[]){
	openlog(NULL, 0, LOG_USER);
	FILE *stream;

	if (argc < 2){
		syslog(LOG_ERR, "Not enough arguments! %d", argc);
		return 1;
	}
	stream = fopen(argv[1], "w+");
	if (stream == NULL){
		syslog(LOG_ERR, "File could not be created at %s", argv[1]);
		return 1;
	}
	else{
		syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
		if( fputs(argv[2], stream) == EOF ){
			syslog(LOG_ERR, "%s could not be written!", argv[2]);
			return 1;
		}
	}
	return 0;
}
