#ifndef LOG_H
#define LOG_H

#include <semaphore.h>

#include "list.h"

extern sem_t log_sem;
extern sem_t mem_sem;
extern sem_t disk_sem;
extern sem_t refresh_sem;
extern sem_t io_sem;
extern sem_t res_acq_sem;

// Processos
typedef struct {
    int is_proc;
    char* name;
    int remaining;
    int id;
    int pc;
    int f_op_count;
} proc_log_info_t;
void process_log_init();
void process_log(const char* process_name, const int remaining, const int pc, const int sid, const int f_op_count);
void process_np_log();


typedef struct DiskLog {
    int r_req_count; // quantidade de requests de leitura
    int w_req_count; // quantidade de requests de escrita
    int forward_dir; // braço indo de dentro pra fora ou vice-versa
    int curr_track; // track atual
    int angular_v; // velocidade angular
    int pending_requests_size; // requests pendentes
} disk_log_t;

typedef struct {
    char *proc_name;
    int proc_id;
    int track; // track que está lendo/escrevendo
    int is_read; // 1 é operação de leitura, 0 é escrita
    int turnaround; // tempo de finalização - tempo de request
} disk_log_info_t;

void disk_log_init();
void disk_log(const char* process_name, const int proc_id,
              const int track, const int read, const int turnaround);

// Entrada e saída
typedef struct {
    long w_bytes; // bytes escritos nos arquivos
    long r_bytes; // bytes lidos nos arquivos
    int p_time; // tempo que um processo está escrevendo
} io_log_t;

typedef enum IoLogType {
    IO_LOG_PRINT,
    IO_LOG_FILE_SYSTEM,
    IO_LOG_DISK_REQUEST
} io_log_type_t;

typedef struct IoLogPrint {
    char* proc_name;
    int duration; // print duration (measured in u.t. that is unit of time).
} io_log_print_t;

typedef enum IoLogFileSystemFlag {
    IO_LOG_FS_READ = 1,
    IO_LOG_FS_WRITE = 2,

    IO_LOG_FS_F_OPEN = 4,
    IO_LOG_FS_F_CLOSE = 8,
} io_log_fs_flag_t;

typedef struct IoLogFileSystem {
    char* proc_name;
    int inumber;
    io_log_fs_flag_t opt;
} io_log_fs_t;

typedef struct IoLogDiskRequest {
    char* proc_name;
    int read; // read or write
} io_log_disk_req_t;

typedef struct {
    io_log_type_t type;
    io_log_print_t* print_log;
    io_log_fs_t* fs_log;
    io_log_disk_req_t* disk_req_log;
} io_log_info_t;

void io_log_init();
void io_fs_log(const char* process_name, const int inumber, const io_log_fs_flag_t opt);
void io_disk_log(const char* process_name, const int read);
void io_print_log(const char* process_name, const int duration);

///////
typedef struct {
    const char* proc_name;
    const char* sem_name;
    int acq;
    int blocked;
} res_acq_log_t;

void res_acq_log_init();
void res_acq_log(const char* process_name, const char* sem_name, const int acq, const int blocked);

/* Global Variables */

extern list_t* process_log_list;
extern list_t* disk_log_list;
extern list_t* io_log_list;
extern list_t* res_acq_log_list;

extern disk_log_t* disk_general_log;
extern io_log_t* io_general_log;

#endif // LOG_H
