#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
//#include <glib.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <syslog.h>

#include <sys/ipc.h>
#include <sys/shm.h>
//#include <mysql.h>
#include "xlog.h"
#include "xlog_client.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define BUFFER_SIZE 40960


int main(int argc, char **argv)
{
        char       buf[BUFFER];
        char       fork_f_buf[BUFFER];
        char       conf_line[1024];
        FILE       *fp;
        const char *filename = "xlog.conf";
        if (argv[1]) {
                filename = argv[1];
        }

        syslog (LOG_NOTICE, "Xlog client daemon started.");
        char *conf_path = "/etc/xlog.conf";
        int oc;
        int daemon = 0;
        while((oc = getopt(argc, argv, "c:dh")) != -1)
        {
                switch(oc)
                {
                        case 'c':
                                conf_path = optarg;
                                break;
                        case 'd':
                                daemon = 1;
                                break;
                        case 'h':
                                show_help();
                                break;
                        case '?':
                                show_help();
                                break;
                        default:
                                show_help();
                }
        }

        //filename = argv[1];
        int ch;
        int count = 0;
        if (!(fp = fopen(conf_path, "r"))) {
                fprintf(stderr, _("Cannot open configure file \"%s\" for read\n"), conf_path);
                exit(1);
        }

      //统计日志文件数目，后面会生产相应个数的子进程，每个子进程负责分析一个日志文件
        while(!feof(fp)) {
                fgets(conf_line, 1024, fp);
                if (strstr(conf_line, "[project]") != NULL) {
                        count++;
                }
        }
        fclose(fp);
        int status,i,j;

        int shmid;
        key_t key_shm = ftok("/tmp", 147);
        shmid= shmget(key_shm, sizeof(project_arr), IPC_CREAT);
        if(shmid== -1){                            // 申请共享内存失败
                  printf("create share memory failed : %s\n", strerror(errno));
                  exit(-1);
        }
        struct conf_project *p_shmaddr;
        p_shmaddr =  (conf_project *) shmat(shmid, NULL, 0);              // 映射到父进程之中的一个地址
        memset(p_shmaddr,0, sizeof(project_arr));      // 初始化共享内存
        memcpy(p_shmaddr, project_arr, sizeof(project_arr));                      // 拷贝共享数据到共享内存
	while(1) {
	 	for(i=0;i<count;i++) {
                	printf("%d %s %d %d %d %d\n",i, project_arr[i].path, p_shmaddr[i].count_total, p_shmaddr[i].count,  p_shmaddr[i].count_ok, p_shmaddr[i].count_ignore);
		}
       		sleep(1);
	}
}

