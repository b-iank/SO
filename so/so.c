#include "../so/so.h"

#include <math.h>
#include <unistd.h>

pthread_mutex_t mutex_memory;
pthread_mutex_t mutex_disk;
pthread_mutex_t mutex_create;
pthread_mutex_t mutex_finish;
pthread_mutex_t mutex_semaphore;

pthread_t semP_id;
pthread_t semV_id;
pthread_t interrupt_id;
pthread_t disk_request_id;
pthread_t disk_finish_id;
pthread_t print_request_id;
pthread_t print_finish_id;
pthread_t mem_load_request_id;
pthread_t mem_load_finish_id;
pthread_t process_create_id;
pthread_t process_finish_id;

int print = 0;

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
SEMAPHORE_TABLE semaphore_table_init() {
    SEMAPHORE_TABLE semaphore_table;
    semaphore_table.qnt_semaphore = 0;

    return semaphore_table;
}

void new_semaphore(char name) {
    SEMAPHORE *semaphore = find_semaphore(name);
    if (semaphore)
        return;
    semaphore = malloc(sizeof(SEMAPHORE));
    if (!semaphore) {
        so_error("Sem memoria");
        exit(EXIT_FAILURE);
    }

    semaphore->name = name;
    semaphore->S = 1;
    semaphore->qnt_blocked = 0;
    semaphore->id_blocked = NULL;
    sem_init(&semaphore->mutex, 0, 1);
    add_semaphore_table(semaphore);
}

void add_semaphore_table(SEMAPHORE *semaphore) {
    SEMAPHORE_TABLE *semaphore_table = &kernel->semaphore_table;
    int qnt = ++kernel->semaphore_table.qnt_semaphore;
    semaphore_table->semaphores = (SEMAPHORE *) realloc(semaphore_table->semaphores, (qnt * sizeof(SEMAPHORE)));
    if (!semaphore_table->semaphores) {
        so_error("Sem memoria");
        exit(EXIT_FAILURE);
    }

    kernel->semaphore_table.semaphores[qnt - 1] = *semaphore;
}

SEMAPHORE *find_semaphore(char semaphore_name) {
    SEMAPHORE_TABLE *semaphore_table = &kernel->semaphore_table;
    SEMAPHORE *sem = NULL;
    for (int i = 0; i < semaphore_table->qnt_semaphore && !sem; i++) {
        if (semaphore_table->semaphores[i].name == semaphore_name)
            sem = &semaphore_table->semaphores[i];
    }

    return sem;
}

int semaphore_process_exists(char semaphore_name, PROCESS *process) {
    for (int i = 0; i < process->qnt_semaphore; i++) {
        if (process->semaphore[i] == semaphore_name)
            return 1;
    }
    return 0;
}

void P(SEMAPHORE *semaphore) {
    PROCESS *process = kernel->scheduler.scheduled->process;

    semaphore->S--;
    if (semaphore->S < 0) {
        semaphore->id_blocked = (int *) realloc(semaphore->id_blocked, (++semaphore->qnt_blocked * sizeof(int)));
        if (!semaphore->id_blocked) {
            so_error("Sem memoria");
            exit(EXIT_FAILURE);
        }
        semaphore->id_blocked[semaphore->qnt_blocked - 1] = process->id;
        process_sleep();
    }
}

void V(SEMAPHORE *semaphore) {
    semaphore->S++;
    if (semaphore->S <= 0 && semaphore->qnt_blocked > 0) {
        semaphore->qnt_blocked--;
        int id = semaphore->id_blocked[0];
        for (int i = 0; i < semaphore->qnt_blocked; i++)
            semaphore->id_blocked[i] = semaphore->id_blocked[i + 1];
        PROCESS *blocked = find_process(id);
        if (!blocked)
            return;
        process_wakeup(blocked);
    }
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESS --------------------------------------
PCB PCB_init() {
    PCB pcb;
    pcb.head = NULL;
    pcb.tail = NULL;
    return pcb;
}

void process_create(char *file_name) {
    FILE *fp;
    PROCESS *process = NULL;

    if (!(fp = fopen(file_name, "r"))) {
        so_define(2, "Arquivo nao encontrado");
        return;
    }

    process = read_synthetic_program(fp);
    if (process)
        sys_call(MEMORY_LOAD_REQUEST, process);
}

PCB add_process(PROCESS *process) {
    PCB lista_aux = kernel->pcb;
    PROCESS *head = lista_aux.head, *aux = head, *anterior = lista_aux.tail;

    if (head == NULL) {
        lista_aux.head = process;
        lista_aux.tail = process;
        process->next = process;
        return lista_aux;
    }

    while (process->priority >= aux->priority && aux->next != head) {
        anterior = aux;
        aux = aux->next;
    }

    if (aux->next == head && process->priority >= aux->priority) {
        anterior = aux;
        aux = aux->next;
    }
    process->next = aux;
    anterior->next = process;

    if (process->priority < head->priority)
        lista_aux.head = process;
    if (process->next == lista_aux.head)
        lista_aux.tail = process;

    return lista_aux;
}

PROCESS *read_synthetic_program(FILE *fp) {
    PROCESS *process;

    process = malloc(sizeof(PROCESS));

    char name[255];

    // Le cabecalho
    fscanf(fp, "%s %d %d %d\n", name, &process->segment_id, &process->priority, &process->segment_size);

    strcpy(process->name, name);
    process->id = kernel->next_id++; // Pega o próximo id de process e soma a variável
    process->pc = 0;
    process->state = NEW;
    process->max_time = (QUANTUM_TIME / process->priority);
    process->qnt_semaphore = 0;
    while (1) {
        char semaphore_name;
        fscanf(fp, "%c", &semaphore_name);
        if (semaphore_name == '\n')
            break;

        if (process->qnt_semaphore == MAX_SEMAPHORE) {
            so_alert("Numero maximo de semaphores alcancado"); // TODO: tirar programas que executam com mais de 10
            continue;
        }
        if (semaphore_name == ' ')
            continue;

        if (semaphore_process_exists(semaphore_name, process)) {
            char *error_msg = malloc(21);
            sprintf(error_msg, "Semaforo %c ja existe", semaphore_name);
            so_alert(error_msg);
            continue;
        }

        process->semaphore[process->qnt_semaphore++] = semaphore_name;
        new_semaphore(semaphore_name);
    }
    fgetc(fp);

    process->qnt_code = 0;

    CODE *code = NULL;
    int code_count = 0;
    char op[51];

    while (fgets(op, 51, fp) != NULL) {
        if (op[0] == '\n') {
            continue;
        }

        code_count++;
        code = realloc(code, sizeof(CODE) * code_count);
        if (!code) {
            so_alert("Sem memoria");
            exit(EXIT_FAILURE);
        }

        if (op[0] == 'P' || op[0] == 'V') {
            if (!semaphore_process_exists(op[2], process)) {
                char error_msg[305];
                sprintf(error_msg, "O semaphores %c nao existe - Processo %s nao criado", op[2], name);
                so_define(3, error_msg);
                free(code);
                free(process);
                return NULL;
            }
            code[code_count - 1].op = op[0] == 'P' ? SEM_P : SEM_V;
            code[code_count - 1].sem = op[2];
        } else {
            char left_op[6];
            int right_op;
            int i = code_count - 1;

            sscanf(op, "%s %d", left_op, &right_op);

            if (strcmp(left_op, "exec") == 0)
                code[i].op = EXEC;
            else if (strcmp(left_op, "read") == 0)
                code[i].op = READ;
            else if (strcmp(left_op, "write") == 0)
                code[i].op = WRITE;
            else if (strcmp(left_op, "print") == 0)
                code[i].op = PRINT;
            else {
                char error_msg[305];
                sprintf(error_msg, "Operacao %s invalida - Processo %s nao criado", left_op, name);
                so_define(3, error_msg);
                free(code);
                free(process);
                return NULL;
            }
            code[i].value = right_op;
            code[i].sem = '#';
        }
    }

    process->code = code;
    process->qnt_code = code_count;
    process->arrival_time = kernel->time;
    fclose(fp);

    return process;
}

int count_codes(FILE *fp) {
    int num_lines = 0, line, size;
    size = ftell(fp);
    line = getc(fp);
    if (size != 0) {
        while (line != EOF) {
            if (line == '\n')
                num_lines++;
            line = getc(fp);
        }
    } else
        num_lines = 0;
    return num_lines;
}

void process_finish(PROCESS *process) {
    if (process && process->state != DONE) {
        interrupt_control(PROCESS_INTERRUPT, (void *) FINISHED);

        pthread_mutex_lock(&mutex_memory);
        segment_free(process->segment_id, process->id);
        remove_scheduler(process);

        free(process->code);
        pthread_mutex_unlock(&mutex_memory);
    }
}

PROCESS *find_process(int id) {
    PROCESS_SCHEDULER *aux = kernel->scheduler.head->next, *head = kernel->scheduler.head;
    if (head->process->id == id)
        return head->process;

    while (aux != head) {
        if (aux->process->id == id)
            return aux->process;
        aux = aux->next;
    }
    return NULL;
}

void run_process(PROCESS *process) {
    int a;
    pthread_mutex_lock(&mutex_memory);
    if ((a = find_segment(process->segment_id)) != -1) {
        SEGMENT *segment = &kernel->segment_table.segments[a];
        if (segment->qnt_page_memory < segment->qnt_page) {
            int request = MEMORY_PAGE_SIZE * (segment->qnt_page - segment->qnt_page_memory);
            page_request(process, segment, request);
            load_memory_page(segment, request);
        }
        pthread_mutex_unlock(&mutex_memory);
        if (process->pc >= process->qnt_code || process->remaining_time <= 0)
            return;

        CODE *code = &process->code[process->pc];
        if (print && code)
            print_code(process->name, code->op); // chegou aqui na execução
        switch (code->op) {
            case EXEC: {
                if (process->remaining_time < code->value) {
                    code->value = code->value - process->remaining_time;
                    process->remaining_time = 0;
                } else {
                    process->remaining_time = process->remaining_time - code->value;
                    process->pc++;
                }
                kernel->time += code->value;
                break;
            }
            case READ: {
                process->pc++;
                sys_call(DISK_READ_REQUEST, (void *) code->value);
                break;
            }
            case WRITE: {
                process->pc++;
                sys_call(DISK_WRITE_REQUEST, (void *) code->value);
                break;
            }
            case PRINT: {
                process->pc++;
                sys_call(PRINT_REQUEST, (void *) code->value);
                break;
            }
            case SEM_P: {
                process->pc++;
                sys_call(SEMAPHORE_P, find_semaphore(code->sem));

                if (process->state != BLOCKED)
                    process->remaining_time = MAX(0, process->remaining_time - 200);

                kernel->time += 200;
                break;
            }
            case SEM_V: {
                process->pc++;
                sys_call(SEMAPHORE_V, find_semaphore(code->sem));
                process->remaining_time = MAX(0, process->remaining_time - 200);
                kernel->time += 200;

                break;
            }
            default:
                process->pc++;
        }
    }
    else {
        pthread_mutex_unlock(&mutex_memory);
    }
}

// ---------------------------------- FUNÇÕES ENTRADA E SAIDA --------------------------------------
void print_request(ARGS *args) {
    PROCESS* process = args->args1;
    int *duration = (int *) args->args2;
    if (!kernel->printing_queue) {
        kernel->printing_queue = malloc(sizeof(char *) * 10);
    }

    kernel->print++; //
    int quant = kernel->print; //
    if (kernel->print > PRINTING_LIMIT) {
        free(kernel->printing_queue[0]);

        for (int i = 0; i < quant - 2; i++) { //
            kernel->printing_queue[i] = kernel->printing_queue[i+1];
        }
        kernel->print--; //
        quant--;
    }
    else {
        kernel->printing_queue[quant-1] = malloc(sizeof(char) * 100);
    }
    sprintf(kernel->printing_queue[quant-1], "Process '%s' imprimiu por %d u.t.", process->name, *duration);
}

void print_finish (PROCESS *process) {
    process_wakeup(process);
}

// ------------------------------------- FUNÇÕES DISCO -----------------------------------------
void disk_init() {
    pthread_t disk_id;
    pthread_attr_t disk_attr;

    pthread_attr_init(&disk_attr);
    pthread_attr_setscope(&disk_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&disk_id, NULL, (void*)disk, NULL);
}

DISK_SCHEDULER disk_scheduler_init() {
    DISK_SCHEDULER disk_scheduler;
    disk_scheduler.head = NULL;
    disk_scheduler.forward_dir = 1;
    disk_scheduler.curr_track = 0;
    return disk_scheduler;
}

void disk_request(ARGS *args) {
    PROCESS* process = args->args1;
    int *track = (int *) args->args2;
    int *read = (int *) args->args3;

    DISK_REQUEST * disk_req = create_disk_request();
    disk_req->process = process;
    disk_req->track = *track;
    disk_req->read = *read;
    disk_req->next = NULL;

    add_disk_request(disk_req);
}

void disk_finish (PROCESS *process) {
    process_wakeup(process);
}

DISK_REQUEST * create_disk_request() {
    DISK_REQUEST* disk_req = (DISK_REQUEST *)malloc(sizeof(DISK_REQUEST));

    if (!disk_req) {
        so_error("Sem memória!\n");
        exit(EXIT_FAILURE);
    }

    return disk_req;
}

void add_disk_request(DISK_REQUEST *new_disk_req) {
    DISK_SCHEDULER *disk_scheduler = &kernel->disk_scheduler;
    DISK_REQUEST *current = disk_scheduler->head;
    DISK_REQUEST *previous = NULL;

    while (current != NULL && current->track < new_disk_req->track) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {
        new_disk_req->next = disk_scheduler->head;
        disk_scheduler->head = new_disk_req;
    } else {
        new_disk_req->next = previous->next;
        previous->next = new_disk_req;
    }
}

void remove_disk_request(DISK_REQUEST *req) {
    DISK_SCHEDULER *disk_scheduler = &kernel->disk_scheduler;
    if (disk_scheduler->head == NULL || req == NULL) {
        so_alert("Erro de processamento!");
        return;
    }

    DISK_REQUEST* current = disk_scheduler->head;
    DISK_REQUEST* previous = NULL;

    if (current == req) {
        disk_scheduler->head = current->next;
        free(req);
        return;
    }

    while (current != NULL && current != req) {
        previous = current;
        current = current->next;
    }

    if (current == req) {
        previous->next = current->next;
        free(req);
    } else {
        so_error("Erro de disco!");
    }
}

static void read_write_disk(int track) {
    DISK_REQUEST *head = kernel->disk_scheduler.head;
    DISK_REQUEST *aux, *next_node;
    for (aux = head; aux != NULL;) {
        next_node = aux->next;
        DISK_REQUEST * disk_req = (DISK_REQUEST *) aux;

        if (disk_req->track == track) {
            pthread_mutex_lock(&mutex_create);
            interrupt_control(DISK_FINISH, disk_req->process);
            pthread_mutex_unlock(&mutex_create);

            remove_disk_request(aux);
        }

        aux = next_node;
    }
    // pthread_join(disk_finish_id, NULL);
}

_Noreturn void disk() {
    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start);
    while (1) {
        clock_gettime(CLOCK_REALTIME, &end);
        const int elapsed = (end.tv_sec - start.tv_sec) * 1000000000L
                            + (end.tv_nsec - start.tv_nsec);

        if (elapsed >= MILLISECONDS_100) {
            start = end;

            if (kernel->disk_scheduler.head){
                pthread_mutex_lock(&mutex_disk);
                read_write_disk(kernel->disk_scheduler.curr_track); // enquanto head == NULL nao faz nada
                pthread_mutex_unlock(&mutex_disk);

                pthread_mutex_lock(&mutex_disk);
                if (kernel->disk_scheduler.forward_dir) {
                    if (kernel->disk_scheduler.curr_track == DISK_TRACK_LIMIT)
                        kernel->disk_scheduler.forward_dir = 0;
                    else
                        kernel->disk_scheduler.curr_track++;
                } else {
                    if (kernel->disk_scheduler.curr_track == 0)
                        kernel->disk_scheduler.forward_dir = 1;
                    else
                        kernel->disk_scheduler.curr_track--;
                }
                pthread_mutex_unlock(&mutex_disk);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
SEGMENT_TABLE segment_table_init() {
    SEGMENT_TABLE segment_table;
    segment_table.segments = NULL;
    segment_table.qnt_segments = 0;
    segment_table.remaining_memory = 1073741824; // (1024^3) em bytes. 1048576 KB. 1024 MB. 1 GB. A pagina é 8 KB./

    return segment_table;
}

void load_memory_page(SEGMENT *segment, int request) {
    SEGMENT_TABLE *tabelaSegmentos = &kernel->segment_table;
    tabelaSegmentos->remaining_memory -= request * K;
    segment->qnt_page_memory = segment->qnt_page;
}

void memory_load_request(PROCESS *process) {
    pthread_mutex_lock(&mutex_memory); // ESSE
    if (find_segment(process->segment_id) == -1) { // Segmento não existe -> cria
        SEGMENT *segment = malloc(sizeof(SEGMENT));
        segment->id = process->segment_id;
        segment->qnt_page = (int) ceil((double) ((double) (process->segment_size) / (MEMORY_PAGE_SIZE)));
        segment->second_chance = 1;

        page_request(process, segment, process->segment_size);
        load_memory_page(segment, segment->qnt_page * MEMORY_PAGE_SIZE);
        add_segment_table(segment);
    }
    pthread_mutex_unlock(&mutex_memory);
}

void memory_load_finish (PROCESS *process) {
    pthread_mutex_lock(&mutex_create); //
    kernel->pcb = add_process(process);

    kernel->scheduler = add_process_scheduler(process);

    process->state = READY;
    interrupt_control(PROCESS_INTERRUPT, NONE);

    so_define(1, "Processo criado!");
    pthread_mutex_unlock(&mutex_create); //
}

void page_request(PROCESS *process, SEGMENT *segment, int request) {
    int remaining = (kernel->segment_table.remaining_memory) - request * K;
    // 1024 - 8 * 1 * 1024
    // -7.168
    // 6 * 8 = 24 kbytes - (1024/1024) 1 kbyte
    //
    if (kernel->segment_table.remaining_memory <= 0)
        page_swap(segment->qnt_page * MEMORY_PAGE_SIZE);
    else if (remaining < 0)
        page_swap((segment->qnt_page * MEMORY_PAGE_SIZE) - (int) (kernel->segment_table.remaining_memory / K));
}

void page_swap(int request) {
    SEGMENT_TABLE *seg_table = &kernel->segment_table;
    SEGMENT *segment;
    while (request != 0) {
        segment = &seg_table->segments[seg_table->curr];
        if (segment->second_chance && segment->qnt_page_memory > 0) {
            segment->second_chance = 0;
        } else {
            while (request != 0 && segment->qnt_page_memory > 0) {
                segment->qnt_page_memory--;
                seg_table->remaining_memory += MEMORY_PAGE_SIZE * K;
                request = MAX(0, request - MEMORY_PAGE_SIZE);
            }
            segment->second_chance = 1;
        }
        seg_table->curr = (seg_table->curr + 1) % seg_table->qnt_segments;
    }
}

void add_segment_table(SEGMENT *segment) {
    SEGMENT_TABLE *seg_table = &kernel->segment_table;
    int i = ++seg_table->qnt_segments;
    seg_table->segments = (SEGMENT *) realloc(seg_table->segments, sizeof(SEGMENT) * i);

    if (!seg_table->segments) {
        so_error("Sem memoria");
        exit(EXIT_FAILURE);
    }

    seg_table->segments[i - 1] = *segment;
}

void segment_free(int segment_id, int process_id) {
    SEGMENT_TABLE *segment_table = &kernel->segment_table;

    int flag = 0;
    PROCESS_SCHEDULER *aux = kernel->scheduler.head->next, *head = kernel->scheduler.head;

    if (head->process->id != process_id && head->process->segment_id == segment_id)
        flag = 1;
    while (aux != head && !flag) {
        if (aux->process->id != process_id && aux->process->segment_id == segment_id)
            flag = 1;
        aux = aux->next;
    }

    if (!flag) { // Se existir outro process que esteja utilizando do segment ele não será removido
        int qnt = segment_table->qnt_segments, index = find_segment(segment_id);
        segment_table->qnt_segments--;

        SEGMENT segment = segment_table->segments[index];
        segment_table->remaining_memory = MIN(
                MAX_MEMORY_SIZE, segment_table->remaining_memory + K * (segment.qnt_page_memory * MEMORY_PAGE_SIZE));

        for (; index < qnt - 1; index++)
            segment_table->segments[index] = segment_table->segments[index + 1];

        // Realoca a tabela de segments para quantidade - 1 HERE
        segment_table->segments = (SEGMENT *) realloc(segment_table->segments, sizeof(SEGMENT) * (qnt - 1));
        if (!segment_table->segments && segment_table->qnt_segments != 0) {
            so_error("Erro de memoria");
            exit(EXIT_FAILURE);
        }
    }
}

int find_segment(int segment_id) {
    SEGMENT_TABLE segment_table = kernel->segment_table;

    int qtd = segment_table.qnt_segments;
    for (int index = 0; index < qtd; index++) {
        if (segment_table.segments[index].id == segment_id)
            return index;
    }

    return -1;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES SCHEDULER -------------------------------------
SCHEDULER scheduler_init() {
    SCHEDULER scheduler;
    scheduler.head = NULL;
    scheduler.tail = NULL;
    scheduler.scheduled = NULL;
    return scheduler;
}

SCHEDULER add_process_scheduler(PROCESS *process) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESS_SCHEDULER *head = scheduler.head, *prev = scheduler.tail;

    PROCESS_SCHEDULER *new = malloc(sizeof(PROCESS_SCHEDULER));
    if (!new) {
        so_error("Sem memoria");
        exit(EXIT_FAILURE);
    }
    new->process = process;
    new->next = NULL;

    if (head == NULL) {
        scheduler.head = new;
        scheduler.tail = new;
        scheduler.head->next = scheduler.tail;
        scheduler.tail->next = scheduler.head;
        return scheduler;
    }

    PROCESS_SCHEDULER *aux = scheduler.head;
    PROCESS *aux_process = aux->process;
    while (process->priority >= aux_process->priority && aux->next != head) {
        prev = aux;
        aux = aux->next;
        aux_process = aux->process;
    }

    if (aux->next == head && process->priority >= aux_process->priority) {
        prev = aux;
        aux = aux->next;
    }

    new->next = aux;
    prev->next = new;

    if (process->priority < head->process->priority)
        scheduler.head = new;
    if (new->next == scheduler.head)
        scheduler.tail = new;

    return scheduler;
}

void schedule_process(int flag) {
    SCHEDULER *scheduler = &kernel->scheduler;
    PROCESS_SCHEDULER *scheduled = scheduler->scheduled, *new = NULL;

    PROCESS_SCHEDULER *aux = NULL;
    if (scheduled)
        aux = scheduled->next;
    while (aux && aux->process->state != READY && aux->next != scheduled)
        aux = aux->next;

    if (aux && aux->process->state == READY) {
        new = aux;
        new->process->remaining_time = new->process->max_time;
    }

    if (scheduled) {
        if (flag == SEMAPHORE_BLOCKED || flag == IO_REQUEST)
            scheduled->process->state = BLOCKED;
        else if (flag == FINISHED) {
            scheduled->process->state = DONE;
            if (new == scheduled)
                new = NULL;
        } else // QUANTUM COMPLETED OU NONE
            scheduled->process->state = READY;
    } else if (scheduler->head) {
        new = scheduler->head;
        new->process->remaining_time = new->process->max_time;
    }

    if (new && new->process->state != READY && scheduled->process->state == READY) {
        scheduled->process->state = RUNNING;
    }
    else if (new) {
        new->process->state = RUNNING;
        scheduler->scheduled = new;
    }
}

void remove_scheduler(PROCESS *process) {
    SCHEDULER *scheduler = &kernel->scheduler;
    PROCESS_SCHEDULER *removed = scheduler->head, *prev = scheduler->tail;
    while (removed->process->id != process->id) {
        prev = prev->next;
        removed = removed->next;
    }
    if (prev == removed)
        scheduler->head = scheduler->tail = NULL;
    else
        prev->next = removed->next;
    if (removed == scheduler->head)
        scheduler->head = removed->next;
    if (removed == scheduler->tail)
        scheduler->tail = prev;
    free(removed);
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES LOG -------------------------------------------
void print_pcb_processes() {
    char next;
    PCB pcb = kernel->pcb;
    PROCESS *process = pcb.head;
    CLEAR;
    if (process) {
        printf("┌──────┬────────────────────────────────────────────────────┬────────────┬────────────┬────────────────"
               "──┐\n");
        printf("│ %-4s │ %-50s │ %-10s │ %-10s │ %-16s │\n", "ID", "Nome", "Estado", "Prioridade", "Tempo de chegada");
        printf("├──────┼────────────────────────────────────────────────────┼────────────┼────────────┼────────────────"
               "──┤\n");
        print_process(process->id, process->name, process->state, process->priority, process->arrival_time);
        while (process->next != pcb.head) {
            process = process->next;
            print_process(process->id, process->name, process->state, process->priority, process->arrival_time);
        }
    } else {
        so_alert("┌─────────────────────────────────────────┐");
        so_alert("│ NAO EXISTEM PROCESSOS NA PCB NO MOMENTO │");
        so_alert("└─────────────────────────────────────────┘");
    }

    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    scanf("%c", &next);
    while (!getchar())
        ;
}

void print_running_process() {
    char next;
    CLEAR;
    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    if (kernel->scheduler.scheduled) {
        printf("\n┌────────────────────────────────┬──────────┐\n");
        printf("│ %-30s │ %-8s │\n", "Nome", "Operacao");
        printf("├────────────────────────────────┼──────────┤\n");
        print = 1;
    } else {
        so_alert("┌────────────────────────────────────────────┐");
        so_alert("│ NAO EXISTE PROCESSO EM EXECUCAO NO MOMENTO │");
        so_alert("└────────────────────────────────────────────┘");
    }

    scanf("%c", &next);
    while (!getchar())
        ;
    print = 0;
}

void print_code(char name[50], char op) {
    char op_str[10];
    switch (op) {
        case EXEC:
            strcpy(op_str, "EXEC");
            break;
        case WRITE:
            strcpy(op_str, "WRITE");
            break;
        case READ:
            strcpy(op_str, "READ");
            break;
        case SEM_P:
            strcpy(op_str, "SEM P");
            break;
        case SEM_V:
            strcpy(op_str, "SEM V");
            break;
        case PRINT:
            strcpy(op_str, "PRINT");
            break;
        default:
            break;
    }
    printf("│ %-30s │ %-8s │\n", name, op_str);
}

void print_segment_table() {
    SEGMENT_TABLE table = kernel->segment_table;
    char next;

    CLEAR;
    if (table.qnt_segments > 0) {
        printf("┌────────────────────────────┐\n");
        printf("│ %-26s │\n", "SEGMENTOS");
        printf("├───────┬────────────────────┤\n");
        printf("│ %-5s │ %-10s │\n", "ID", "Quantidade Paginas");
        printf("├───────┼────────────────────┤\n");
        for (int i = 0; i < table.qnt_segments; i++)
            print_segment(table.segments[i].id, table.segments[i].qnt_page_memory);
    } else {
        so_alert("┌───────────────────────────────────────────┐");
        so_alert("│ NAO EXISTEM SEGMENTOS ALOCADOS NO MOMENTO │");
        so_alert("└───────────────────────────────────────────┘");
    }

    printf("MEMORIA DISPONIVEL: %d%%\n", (kernel->segment_table.remaining_memory / MAX_MEMORY_SIZE) * 100);
    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    scanf("%c", &next);
    while (!getchar())
        ;
}

void print_printing_queue() {
    char **table = kernel->printing_queue;
    int quant = kernel->print;
    char next;

    CLEAR;
    if (quant && table) {
        printf("┌──────────────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-88s │\n", "FILA DE IMPRESSAO");
        printf("├──────────────────────────────────────────────────────────────────────────────────────────┤\n");
        for (int i = 0; i < quant; i++)
            printf("│ %-88s │\n", table[i]);
        printf("└──────────────────────────────────────────────────────────────────────────────────────────┘");
    } else {
        so_alert("┌────────────────────────────────────────────┐");
        so_alert("│     FILA DE IMPRESSAO VAZIA NO MOMENTO     │");
        so_alert("└────────────────────────────────────────────┘");
    }

    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    scanf("%c", &next);
    while (!getchar())
        ;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES KERNEL ----------------------------------------
KERNEL *kernel_init() {
    kernel = (KERNEL *) malloc(sizeof(KERNEL));

    if (!kernel) {
        so_error("Sem memoria");
        exit(EXIT_FAILURE);
    }

    kernel->pcb = PCB_init();
    kernel->next_id = 1;
    kernel->time = 0;
    kernel->segment_table = segment_table_init();
    kernel->scheduler = scheduler_init();
    kernel->semaphore_table = semaphore_table_init();
    kernel->disk_scheduler = disk_scheduler_init();
    kernel->printing_queue = NULL;
    kernel->print = 0;
    return kernel;
}

void sys_call(char function, void *arg) {
    switch (function) {
        case PROCESS_CREATE: {
            process_create((char *) arg);
            break;
        }
        case MEMORY_LOAD_REQUEST: {
            pthread_mutex_lock(&mutex_create);

            pthread_create(&mem_load_request_id, NULL, (void *)memory_load_request, (PROCESS *) arg);
            interrupt_control(MEMORY_LOAD_FINISH, (PROCESS *) arg);

//            pthread_join(mem_load_request_id, NULL);
//            pthread_join(mem_load_finish_id, NULL);
            // pthread_detach();
            pthread_mutex_unlock(&mutex_create);
            break;
        }
        case SEMAPHORE_P: {
            pthread_mutex_lock(&mutex_semaphore);
            pthread_create(&semP_id, NULL, (void *)P, (PROCESS *) arg);
            pthread_mutex_unlock(&mutex_semaphore);
            break;
        }
        case SEMAPHORE_V: {
            pthread_mutex_lock(&mutex_semaphore);
            pthread_create(&semV_id, NULL, (void *)V, (PROCESS *) arg);
            pthread_mutex_unlock(&mutex_semaphore);
            break;
        }
        case DISK_READ_REQUEST: {
            int track = (int) arg;
            PROCESS_SCHEDULER *process = kernel->scheduler.scheduled;

            interrupt_control(PROCESS_INTERRUPT, (void *) IO_REQUEST);

            pthread_mutex_lock(&mutex_disk);
            int aux = 1;
            ARGS *args = malloc(sizeof(ARGS));
            args->args1 = process->process;
            args->args2 = &track;
            args->args3 = &aux;

            pthread_create(&disk_request_id, NULL, (void *) disk_request, (void *) args);
            pthread_mutex_unlock(&mutex_disk);
            break;
        }
        case DISK_WRITE_REQUEST: {
            int track = (int) arg;
            PROCESS *process = kernel->scheduler.scheduled->process;

            interrupt_control(PROCESS_INTERRUPT, (void *) IO_REQUEST);

            pthread_mutex_lock(&mutex_disk);
            int aux = 0;
            ARGS *args = malloc(sizeof(ARGS));
            args->args1 = process;
            args->args2 = &track;
            args->args3 = &aux;

            pthread_create(&disk_request_id, NULL, (void *) disk_request, (void *) args);
            //pthread_join(disk_request_id, NULL);
            pthread_mutex_unlock(&mutex_disk);
            break;
        }
        case PRINT_REQUEST: {
            PROCESS *process = kernel->scheduler.scheduled->process;

            interrupt_control(PROCESS_INTERRUPT, (void *) IO_REQUEST);

            pthread_mutex_unlock(&mutex_create);
            int aux = (int) arg;
            ARGS *args = malloc(sizeof(ARGS));
            args->args1 = process;
            args->args2 = &aux;

            pthread_create(&print_request_id, NULL, (void *) print_request, (void *) args);
            interrupt_control(PRINT_FINISH, process);
            //pthread_join(print_request_id, NULL);
            pthread_mutex_lock(&mutex_create);
            break;
        }
        default: { // delete(System32);
            break;
        }
    }
}

void interrupt_control(char function, void *arg) {
    switch (function) {
        case PROCESS_INTERRUPT: {
            schedule_process((int) arg);
            break;
        }
        case PROCESS_FINISH: {
            pthread_create(&process_finish_id, NULL, (void *) process_finish, (PROCESS *) arg);
            //pthread_join(process_finish_id, NULL);
            break;
        }
        case MEMORY_LOAD_FINISH: {
            pthread_create(&mem_load_finish_id, NULL, (void *) memory_load_finish, (PROCESS *) arg);
            //pthread_join(mem_load_finish_id, NULL);
            break;
        }
        case DISK_FINISH: {
            pthread_create(&disk_finish_id, NULL, (void *)disk_finish, (PROCESS *)arg);
            //pthread_join(disk_finish_id, NULL);
            break;
        }
        case PRINT_FINISH: {
            pthread_create(&print_finish_id, NULL, (void *)print_finish, (PROCESS *) arg);
            //pthread_join(print_finish_id, NULL);
            break;
        }
        default: {
            char error_msg[305];
            sprintf(error_msg, "O process %c nao esta definido", function);
            so_define(2, error_msg);
        }
    }
}

void process_sleep() {
    interrupt_control(PROCESS_INTERRUPT, (void *) SEMAPHORE_BLOCKED);
}

void process_wakeup(PROCESS *processo) { processo->state = READY; }
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES CPU -------------------------------------------
void cpu_init() {
    pthread_t cpu_id;
    pthread_attr_t cpu_attr;

    pthread_mutex_init(&mutex_memory, NULL);
    pthread_mutex_init(&mutex_disk, NULL);
    pthread_mutex_init(&mutex_create, NULL);
    pthread_mutex_init(&mutex_finish, NULL);
    pthread_mutex_init(&mutex_semaphore, NULL);

    pthread_attr_init(&cpu_attr);
    pthread_attr_setscope(&cpu_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&cpu_id, NULL, (void *) cpu, NULL);
}

_Noreturn void cpu() {
    while (!kernel)
        ;

    double elapsed;
    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start);

    while (1) {
        if (!kernel->scheduler.head)
            continue;
        else if (!kernel->scheduler.scheduled) {
            pthread_mutex_lock(&mutex_create);
            interrupt_control(PROCESS_INTERRUPT, NONE);
            pthread_mutex_unlock(&mutex_create);
        } else {
            while (kernel->scheduler.scheduled != NULL && kernel->scheduler.scheduled->process->remaining_time > 0 &&
                   kernel->scheduler.scheduled->process->pc < kernel->scheduler.scheduled->process->qnt_code)
            {
                clock_gettime(CLOCK_REALTIME, &end);
                elapsed = difftime(end.tv_sec, start.tv_sec) + (double) (end.tv_nsec - start.tv_nsec) / ONE_SECOND_NS;

                pthread_mutex_lock(&mutex_finish);
                pthread_mutex_lock(&mutex_create);
                if (elapsed >= 0.75) {
                    start = end;
                    run_process(kernel->scheduler.scheduled->process);
                }
                pthread_mutex_unlock(&mutex_create);
                pthread_mutex_unlock(&mutex_finish);
            }

            pthread_mutex_lock(&mutex_create);
            if (kernel->scheduler.scheduled == NULL) {
                continue;
            }

            pthread_mutex_lock(&mutex_finish);
            if (kernel->scheduler.scheduled->process->pc >= kernel->scheduler.scheduled->process->qnt_code) {
                interrupt_control(PROCESS_FINISH, (void *) kernel->scheduler.scheduled->process);
            } else {
                interrupt_control(PROCESS_INTERRUPT, (void *) QUANTUM_COMPLETED);
            }
            pthread_mutex_unlock(&mutex_finish);
            pthread_mutex_unlock(&mutex_create);
        }
    }
}
// ---------------------------------------------------------------------------------------------
