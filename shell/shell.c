/**
 * shell
 * CS 241 - Fall 2021
 */
#include "format.h"
#include "shell.h"
#include "vector.h"

#include "sstring.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <dirent.h> 

extern char *optarg;

static vector* ProcAll;
static vector* HisAll;
static char* HisFile = NULL;
static FILE* input = NULL;
static int reDir = 0;


typedef struct process {
    char *command;
    pid_t pid;
} process;

int LoOr(char* cmd) {
    char* Loc = strstr(cmd, "||");
    size_t calLen = strlen(cmd);
    size_t firLen = Loc - cmd;
    size_t sedLen = calLen - (firLen + 3);
    char firC [firLen];
    char sndC [sedLen + 1];
    strncpy(firC, cmd, firLen);
    strncpy(sndC, (Loc + 3), sedLen);
    firC[firLen - 1] = '\0';
    sndC[sedLen] = '\0';
    int first_status;
    if ((first_status = execute(firC, 1))) {
        return execute(sndC, 1);
    }
    return 0;
}

int OutReDir(char* cmd) {
    char* Loc = strstr(cmd, ">");
    size_t calLen = strlen(cmd);
    size_t firLen = Loc - cmd;
    size_t sedLen = calLen - (firLen + 2);
    char firC [firLen];
    char sndC [sedLen + 1];
    strncpy(firC, cmd, firLen);
    strncpy(sndC, (Loc + 2), sedLen);
    firC[firLen - 1] = '\0';
    sndC[sedLen] = '\0';
    FILE* fd = fopen(sndC, "w");
    int fd_num = fileno(fd);
    int original = dup(fileno(stdout));
    fflush(stdout);
    dup2(fd_num, fileno(stdout));
    execute(firC, 1);
    fflush(stdout);
    close(fd_num);
    dup2(original, fileno(stdout));
    reDir = 0;
    return 0;
}
int LoAnd(char* cmd) {
    char* Loc = strstr(cmd, "&&");
    size_t calLen = strlen(cmd);
    size_t firLen = Loc - cmd;
    size_t sedLen = calLen - (firLen + 3);
    char firC [firLen];
    char sndC [sedLen + 1];
    strncpy(firC, cmd, firLen);
    strncpy(sndC, (Loc + 3), sedLen);
    firC[firLen - 1] = '\0';
    sndC[sedLen] = '\0';
    int first_status;
    if (!(first_status = execute(firC, 1))) {
        return execute(sndC, 1);
    }

    return 1;
}
int AppReDir(char* cmd) {
    char* Loc = strstr(cmd, ">>");
    size_t calLen = strlen(cmd);
    size_t firLen = Loc - cmd;
    size_t sedLen = calLen - (firLen + 3);
    char firC [firLen];
    char sndC [sedLen + 1];
    strncpy(firC, cmd, firLen);
    strncpy(sndC, (Loc + 3), sedLen);
    firC[firLen - 1] = '\0';
    sndC[sedLen] = '\0';
    // run
    FILE* fd = fopen(sndC, "a");
    int fd_num = fileno(fd);
    int original = dup(fileno(stdout));
    fflush(stdout);
    dup2(fd_num, fileno(stdout));
    execute(firC, 1);
    fflush(stdout);
    close(fd_num);
    dup2(original, fileno(stdout));
    reDir = 0;
    return 0;
}
int SepLo(char* cmd) {
    char* Loc = strstr(cmd, ";");
    size_t calLen = strlen(cmd);
    size_t firLen = Loc - cmd;
    size_t sedLen = calLen - (firLen + 2);
    char firC [firLen + 1];
    char sndC [sedLen + 1];
    strncpy(firC, cmd, firLen);
    strncpy(sndC, (Loc + 2), sedLen);
    firC[firLen] = '\0';
    sndC[sedLen] = '\0';
    
    int result1 = execute(firC, 1);
    int result2 = execute(sndC, 1);

    return result1 && result2;
}


void ExitShell(int status) {
    if (HisFile != NULL) {
        FILE *HisFD = fopen(HisFile, "w");
        for (size_t i = 0; i < vector_size(HisAll); ++i) {
            fprintf(HisFD, "%s\n", (char*)vector_get(HisAll, i));
        }
        fclose(HisFD);
    }
    if (HisAll) {
        vector_destroy(HisAll);
    }
    if (ProcAll) {
        vector_destroy(ProcAll);
    }
    if (input != stdin) {
        fclose(input);
    }
    exit(status);
}


char* hdlrHis(char* file) {
    FILE* fd = fopen(get_full_path(file), "r");
    if (fd) {
        char* buff = NULL;
        size_t len = 0;
        while(getline(&buff, &len, fd) != -1) {
            if (strlen(buff) > 0 && buff[strlen(buff) - 1] == '\n') {
                buff[strlen(buff) - 1] = '\0';
                vector_push_back(HisAll, buff);
            }
        }
        free(buff);
        fclose(fd);
        return get_full_path(file);
    } 
    else {
        fd = fopen(file, "w");
    }
    fclose(fd);
    return get_full_path(file);
} 


int cd(char* dir) {
    int result = chdir(dir);
    if (result) {
        print_no_directory(dir);
        return 1;
    }
    return 0;
}

void signal_handler(int sig_name) {
    if(sig_name == SIGINT) {
    } 
}

process* CPro(pid_t pid, char* comm) {
    process* temp = malloc(sizeof(process));
    temp -> pid = pid;
    temp -> command = malloc(sizeof(char) * (strlen(comm) + 1));
    strcpy(temp -> command, comm);
    return temp;
}

int ExeTer(char*cmd) {
    pid_t pid = fork();
    process* p = CPro(pid, cmd);
    vector_push_back(ProcAll, p);
    sstring* s = cstr_to_sstring(cmd);
    vector* splt = sstring_split(s, ' ');
    size_t size = vector_size(splt);
    char* fst = vector_get(splt, 0);
    char* lst = vector_get(splt, size - 1);
    int temp = 0;
    if (size >= 2) {
       if (!strcmp(lst, "&")) {
            temp = 1;
            vector_set(splt, size - 1, NULL); 
        }
    }
    char* arr[size + 1];  
    for (size_t i = 0;i < size; i++) {
        arr[i] = vector_get(splt, i);
    }
    arr[size] = NULL;

    if (pid > 0) {
        if (!reDir) {
            print_command_executed(pid);
        }
        int status = 0;
        if (temp) {
            waitpid(pid, &status, WNOHANG);
        } else {
            pid_t pid_w = waitpid(pid, &status, 0);
            if (pid_w == -1) {
                print_wait_failed();
            } else if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    return 1;
                }
                fflush(stdout);
            } else if (WIFSIGNALED(status)) {
            }
            sstring_destroy(s);
            vector_destroy(splt);
            return status;
        }
    } else if (!pid) {
        if (temp) {
            if (setpgid(getpid(), getpid()) == -1) {
                print_setpgid_failed();
                fflush(stdout);
                ExitShell(1);
            }
        }
        fflush(stdout);
        execvp(fst, arr);
        print_exec_failed(cmd);
        exit(1);
    }else if (pid == -1) {
        print_fork_failed();
        ExitShell(1);
        return 1;
    }

    sstring_destroy(s);
    vector_destroy(splt);
    return 1;
}

void print_shell() {
    print_process_info_header();
    for (size_t i = 0; i < vector_size(ProcAll); i++) {
        process* ptr = vector_get(ProcAll, i);
        if(kill(ptr -> pid,0) != -1){
            process_info PI;
            PI.command = ptr -> command;
            PI.pid = ptr -> pid;
            char arr[100];
            snprintf(arr, sizeof(arr), "/proc/%d/stat", ptr->pid);
            FILE* fd = fopen(arr, "r");
            unsigned long long st = 0;
            unsigned long time = 0;
            unsigned long long btime = 0;
            char strT[20];

            if (fd) {
                char* buff = NULL;
                size_t len;
                ssize_t bytes_read = getdelim( &buff, &len, '\0', fd);
                if ( bytes_read != -1) {
                    sstring* s = cstr_to_sstring(buff);
                    vector* count = sstring_split(s, ' ');
                    long int nthreads = 0;
                    sscanf(vector_get(count, 19), "%ld", &nthreads);
                    PI.nthreads = nthreads;
                    unsigned long int vsize = 0;
                    sscanf(vector_get(count, 22), "%lu", &vsize);
                    PI.vsize = vsize / 1024;
                    char state;
                    sscanf(vector_get(count, 2), "%c", &state);
                    PI.state = state;
                    unsigned long utime = 0;
                    unsigned long stime = 0;
                    sscanf(vector_get(count, 14), "%lu", &stime);
                    sscanf(vector_get(count, 13), "%lu", &utime);
                    time = utime / sysconf(_SC_CLK_TCK) + stime / sysconf(_SC_CLK_TCK);
                    unsigned long seconds = time % 60;
                    unsigned long mins = (time - seconds) / 60;
                    execution_time_to_string(strT, 20, mins, seconds);
                    PI.time_str = strT;                 
                    sscanf(vector_get(count, 21), "%llu", &st);
                    vector_destroy(count);
                    sstring_destroy(s);
                }
            }
          
            fclose(fd);
            FILE* fd2 = fopen("/proc/stat", "r");
            if (fd2) {
                char* buff2 = NULL;
                size_t len;
                ssize_t bytes_read = getdelim( &buff2, &len, '\0', fd2);
                if ( bytes_read != -1) {
                    sstring* s = cstr_to_sstring(buff2);
                    vector* lines = sstring_split(s, '\n');
                    
                    for (size_t i = 0; i < vector_size(lines); i++) {
                        if (!strncmp("btime", vector_get(lines, i), 5)) {
                            char temp[10];
                            sscanf(vector_get(lines, i), "%s %llu", temp, &btime);
                        }
                    }
                    vector_destroy(lines);
                    sstring_destroy(s);
                }
            }
            fclose(fd2);
            
            char start_str[20];
            time_t start_time_final = btime + (st / sysconf(_SC_CLK_TCK));
            time_struct_to_string(start_str, 20, localtime(&start_time_final));
            PI.start_str = start_str;
            print_process_info(&PI);  
        }
    }
}

size_t ProInx(pid_t pid) {
    ssize_t temp = -1;
    for (size_t i = 0;i < vector_size(ProcAll); i++) {
        process* ptr = vector_get(ProcAll, i);
        if (ptr -> pid == pid) {
            temp = i;
            break;
        }
    }
    return temp;
}


int execute(char* cmd, int logic) {
    if ((strstr(cmd, "&&"))) {
        vector_push_back(HisAll, cmd);
        return LoAnd(cmd);
    } else if ((strstr(cmd, "||"))) {
        vector_push_back(HisAll, cmd);
        return LoOr(cmd);
    } else if ((strstr(cmd, ";"))) {
        vector_push_back(HisAll, cmd);
        return SepLo(cmd);
    } else if ((strstr(cmd, ">>"))) {
        reDir = 1;
        vector_push_back(HisAll, cmd);
        return AppReDir(cmd);
    } else if ((strstr(cmd, ">"))) {
        reDir = 1;
        vector_push_back(HisAll, cmd);
        return OutReDir(cmd);
    } else if ((strstr(cmd, "<"))) {
        vector_push_back(HisAll, cmd);
        return 0;
    }
    
    sstring* s = cstr_to_sstring(cmd);
    vector* splt = sstring_split(s, ' ');
    int vld = 0;
    size_t size = vector_size(splt);
    char* firS = vector_get(splt, 0);


    if (size) {
        if(!strcmp(firS, "cd")) {
            if (size > 1) {
                vld = 1;
                if (!logic) {
                    vector_push_back(HisAll, cmd);
                }
                int result = cd(vector_get(splt, 1));
                sstring_destroy(s);
                vector_destroy(splt);
                return result;
            }
        } else if (firS[0] == '#') {
            if (size == 1 && strlen(firS) != 1) {
                vld = 1;
                size_t index = atoi(firS + 1);
                if (index < vector_size(HisAll)) {
                    char* exec_cmd = vector_get(HisAll, index);
                    print_command(exec_cmd);
                    sstring_destroy(s);
                    vector_destroy(splt);
                    return execute(exec_cmd, 0);
                } 
                print_invalid_index();
                sstring_destroy(s);
                vector_destroy(splt);
                return 1;
            }
        } else if (!strcmp(firS, "!history")) {
            if (size == 1) {
                vld = 1;
                for (size_t i = 0; i < vector_size(HisAll); i++) {
                    print_history_line(i, vector_get(HisAll, i));
                }
                sstring_destroy(s);
                vector_destroy(splt);
                return 0;
            }
        } else if (!strcmp(firS, "kill")) {
            if (!logic) {
                vector_push_back(HisAll, cmd);
            }
            if (size == 2) {
                vld = 1;
                pid_t target = atoi(vector_get(splt, 1));
                ssize_t index = ProInx(target);
                if (index == -1) {
                    print_no_process_found(target);
                    sstring_destroy(s);
                    vector_destroy(splt);
                    return 1;
                }
                kill(target, SIGKILL);
                print_killed_process(target, ((process*) vector_get(ProcAll, index)) -> command);
                sstring_destroy(s);
                vector_destroy(splt);
                return 0;
            }
        }  else if (!strcmp(firS, "stop")) {
            if (!logic) {
                vector_push_back(HisAll, cmd);
            }
            if (size == 2) {
                vld = 1;
                pid_t target = atoi(vector_get(splt, 1));
                ssize_t index = ProInx(target);
                if (index == -1) {
                    print_no_process_found(target);
                    sstring_destroy(s);
                    vector_destroy(splt);
                    return 1;
                }
                kill(target, SIGSTOP);
                print_stopped_process(target, ((process*) vector_get(ProcAll, index)) -> command);
                sstring_destroy(s);
                vector_destroy(splt);
                return 0;
            }
        } else if (firS[0] == '!') {
            if (size >= 1) {
                vld = 1;
                char* prefix = firS + 1;
                for (size_t i = vector_size(HisAll); i > 0; --i) {
                    char* another_cmd = vector_get(HisAll, i - 1);
                    if (!strncmp(another_cmd, prefix, strlen(prefix))) {
                        print_command(another_cmd);
                        sstring_destroy(s);
                        vector_destroy(splt);
                        return execute(another_cmd, 0);
                    }
                }
                print_no_history_match();
                sstring_destroy(s);
                vector_destroy(splt);
                return 1;
            }
        }else if (!strcmp(firS, "print_shell")) {
            if (!logic) {
                vector_push_back(HisAll, cmd);
            }
            if (size == 1) {
                vld = 1;
                print_shell();
                return 0;
            }
        } else if (!strcmp(firS, "cont")) {
            if (!logic) {
                vector_push_back(HisAll, cmd);
            }
            if (size == 2) {
                vld = 1;
                pid_t target = atoi(vector_get(splt, 1));
                ssize_t index = ProInx(target);
                if (index == -1) {
                    print_no_process_found(target);
                    sstring_destroy(s);
                    vector_destroy(splt);
                    return 1;
                }
                kill(target, SIGCONT);
                print_continued_process(target, ((process*) vector_get(ProcAll, index)) -> command);
                sstring_destroy(s);
                vector_destroy(splt);
                return 0;
            }
        } else if (!strcmp(firS, "exit")) {
            if (size == 1) {
                vld = 1;
                ExitShell(0);
            }
        } else {
            if (!logic) {
                vector_push_back(HisAll, cmd);
            }
            vld = 1;
            sstring_destroy(s);
            vector_destroy(splt);
            fflush(stdout);
            return ExeTer(cmd);
        }
       
    }

    if (!vld) {
        print_invalid_command(cmd);
    }
    sstring_destroy(s);
    vector_destroy(splt);
    return 1;
}





int shell(int argc, char *argv[]) {
    if (!( argc == 5 || argc == 1 || argc == 3)) {
        print_usage();
        exit(1);
    }
    signal(SIGINT, signal_handler);
    int pid = getpid();
    process* shell = CPro(pid, argv[0]);
    ProcAll = shallow_vector_create();
    vector_push_back(ProcAll, shell);
    HisAll = string_vector_create();
    char* cwd = malloc(256);
    input = stdin;
    if (!getcwd(cwd, 256)) {
        ExitShell(1);
    }
    print_prompt(getcwd(cwd, 256), pid);
    int arg;
    while((arg = getopt(argc, argv, "f:h:")) != -1) {
        if (arg == 'f') {
            FILE* script = fopen(optarg, "r");
            if (!script) {
                print_script_file_error();
                ExitShell(1);
            }
            input = script;
        } 
        else if (arg == 'h') {
            HisFile = hdlrHis(optarg);
        } 
        else {
            print_usage();
            ExitShell(1);
        }
    }
    char* buff = NULL;
    size_t length = 0;
    while (getline(&buff, &length, input) != -1) {
        int status; 
        for (size_t i = 0;i < vector_size(ProcAll); i++) {
            process* p = vector_get(ProcAll, i);
            waitpid(p -> pid, &status, WNOHANG); 
        }
        if (!strcmp(buff, "\n")) {
        } else {
            if (strlen(buff) > 0 && buff[strlen(buff) - 1] == '\n') {
                buff[strlen(buff) -1] = '\0';
            }
            char* copy = malloc(strlen(buff));
            strcpy(copy, buff);
            execute(copy, 0);
        }

        if (!getcwd(cwd, 256)) {
            ExitShell(1);
        }
        print_prompt(getcwd(cwd, 256), pid);
        fflush(stdout);
    }
    free(buff);
    ExitShell(0);
    return 0;
}