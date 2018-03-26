#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#define TIMEOUT_READ 1000000
#define SIZE_STR_QUEUE (64*1024)

#define DEVICE_NAME "/dev/qikchar"
#define FILE_NAME "/var/qikchar.txt"

// прототипы функций
void QueueReadDeamon(void);
//-----------------------------------------------------------
// точка входа
int main() {
  int pid = fork();
  switch(  pid ) {
  case 0:
    setsid();
    chdir("/");
    QueueReadDeamon();
    exit(0);
  case -1:
    printf("Fail: unable to fork\n");
    break;
  default:
   printf("%d\n", pid);
   break;
 }
 return 0;
}
//-----------------------------------------------------------

void QueueReadDeamon(void)
{
    int len, i;
    FILE *fout, *fin;
    char buf[SIZE_STR_QUEUE];
    
    openlog("QueueReadDeamon", 0, LOG_USER);
    syslog(LOG_NOTICE, "Daemon start");
    
    
    fout = fopen(DEVICE_NAME, "rt");
    if (fout==NULL)
    {
        syslog(LOG_NOTICE, "Error open device");
        closelog();
        return;
    }
    fin = fopen(FILE_NAME, "at");
    if (fin==NULL)
    {
        syslog(LOG_NOTICE, "Error open file");
        fclose(fout);
        closelog();
        return;
    }
    
    while(1)
    {
        len = fread(buf, 1, SIZE_STR_QUEUE-1, fout);
        if(len > 0)
        {
            for (i = 0; i < len; i++)
            {
                if (buf[i]==0)
                {
                    buf[i] = '\0';
                    break;
                }
            }
            buf[len] = 0;
            syslog(LOG_NOTICE, "read: %s", buf);
            fprintf(fin, "%s\n", buf);
            fflush(fin);
        }
        else
        {
            syslog(LOG_NOTICE, "read empty");
        }
        usleep(TIMEOUT_READ);
    }
    
    fclose(fout);
    fclose(fin);
    
    syslog(LOG_NOTICE, "Daemon stop");
    closelog();
    return;
}

