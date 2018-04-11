#include <stdlib.h>
#include <sys/socket.h>
#include <memory.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#define BUFSIZ 1024
#define LOGIN_NAME_MAX 32
#define HOST_NAME_MAX 32
#define SMTP_PORT 25
#define MAILTO 1

char HELO_COMMAND[] = "EHLO test\n";
char MAIL_FROM[] = "MAIL FROM:<%s@%s>\t\n";
char RCPT_TO[] = "RCPT TO:<%s>\r\n";
char DATA[] = "DATA\r\n";
char SUBJECT[] = "Subject: %s\n";
char FROM[] = "From: %s@%s\n";
char TO[] = "To: %s\n";
char MESSAGE[] = "%s\r\n";
char DOT[] = ".\r\n";
char QUIT[] = "quit\r\n";

void runSmtpCommand(char *message, int s, int wait) {
    char response[1024];
    memset(response, 0, sizeof(response));
    send(s, message, strlen(message),0);
    printf("Sending: %s\n", message);
   
    recv(s, response, sizeof(response), 0);
    
    printf("Got response : %s\n", response);
}

int main(int argc, char* argv[]) {
    struct hostent *d_addr;
    struct sockaddr_in addr;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    d_addr = gethostbyname("rk6lab.bmstu.ru");
    
    if(!d_addr) {
    	printf("%s\n", strerror(errno));
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *((unsigned long *) d_addr->h_addr);
    addr.sin_port = htons(SMTP_PORT); // порт

    if(connect(s, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("%s\n", strerror(errno));
    }

    char response[1024];
    memset(response, 0, sizeof(response));
    recv(s, response, sizeof(response), 0);
    printf("%s", response);

    char login[LOGIN_NAME_MAX];
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    getlogin_r(login, LOGIN_NAME_MAX);

    // Посылка EHLO
    runSmtpCommand(HELO_COMMAND, s, 0);

    // Посылка MAIL FROM
    char* buf = (char*) calloc (BUFSIZ, sizeof(char));
    sprintf(buf, MAIL_FROM, login, hostname);
    runSmtpCommand(buf, s, 0);

    // Посылка RCPT TO
    buf = (char*) calloc (BUFSIZ, sizeof(char));
    sprintf(buf, RCPT_TO, argv[1]);
    runSmtpCommand(buf, s, 0);

    // Посылка DATA
    runSmtpCommand(DATA, s, 0);

    // Подготовка заголовка
    char finalMessage[BUFSIZ] = "";
    char tempFrom[100];
    sprintf(tempFrom, FROM, login, hostname);
    strcat(finalMessage, tempFrom);
    
    char tempTo[100];
    sprintf(tempTo, TO, argv[1]);
    strcat(finalMessage, tempTo);
    
    printf("Subject: \n");
    char subject[1000];
    int i = 0;
    char n;
    while ((n=getchar()) != '\n') {
        subject[i++] = n;
    }
    char tempSubject[BUFSIZ];
    sprintf(tempSubject, SUBJECT, subject);
    strcat(finalMessage, tempSubject);
    // Подготовка письма
    char message[BUFSIZ];
    i = 0;
    while ((n=getchar()) != EOF) {
        message[i++] = n;
    }
    char tempMessage[BUFSIZ];
    sprintf(tempMessage, MESSAGE, message);
    strcat(finalMessage, tempMessage);
    strcat(finalMessage, DOT);
    // Посылка письма
    runSmtpCommand(finalMessage, s, 1);

    // Посылка QUIT
    runSmtpCommand(QUIT, s, 0);
    close(s);
    return 0;
}

