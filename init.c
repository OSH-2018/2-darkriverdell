#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

void exe_cmd(char **cmds, int exe_cmd_num, int num_of_cmd, int* write_fd)
{
    int read_fd[2];
    pid_t pid;
    int i;
    char* args[128];
    args[0] = cmds[exe_cmd_num];
    for (i = 0; *args[i]; i++)
        for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
            if (*args[i+1] == ' ') {
                *args[i+1] = '\0';
                args[i+1]++;
                break;
            }

    args[i] = NULL;
    if (exe_cmd_num != 0) {
        pipe(read_fd); 
        pid = fork();
        if (pid == 0) {
            exe_cmd(cmds, exe_cmd_num-1, num_of_cmd, read_fd);
        }
        else {
	    close(read_fd[1]);
            dup2(read_fd[0], STDIN_FILENO);
            close(read_fd[0]);
            wait(NULL);
        }
    }
    else
        ;
    
    if (exe_cmd_num != num_of_cmd-1) {
        close(write_fd[0]);
        dup2(write_fd[1], STDOUT_FILENO);
        close(write_fd[1]);
    }
    execvp(args[0], args);
}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    char *cmds[128];
    int num_of_cmd;
    while (1) {
        /* 提示符 */
        printf("# ");
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
             
        /* 拆解命令行 */
        cmds[0] = cmd;
        for (i = 0; *cmds[i]; i++)
            for (cmds[i+1] = cmds[i] + 1; *cmds[i+1]; cmds[i+1]++)
                if (*cmds[i+1] == '|') {
                    *cmds[i+1] = '\0';
                    cmds[i+1]++;
                    break;
                }
        cmds[i] = NULL;
        num_of_cmd = i;


        /* 拆解命令行 */
        args[0] = cmds[0];
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    break;
                }
        args[i] = NULL;
        

        /* 没有输入命令 */
        if (!args[0])
            continue;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0],"export") == 0) {
             if (args[1]) {
                 char *name = args[1];
                 for (i = 0; args[1][i] != '='; i++)
                     ;
                 char *value = args[1]+i+1;
                 setenv(name, value, 1);
             }
	     continue;
        }

        if (strcmp(args[0], "exit") == 0)
            return 0;

        /* 外部命令 */
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            exe_cmd(cmds, num_of_cmd-1, num_of_cmd, NULL);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        wait(NULL);
    }
}
