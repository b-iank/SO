#include "../so/so.h"

#include <math.h>

pthread_mutex_t mutex_scheduler;
pthread_mutex_t mutex_memory;

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

void P(SEMAPHORE *semaphore, PROCESS *process) {
    // sem_wait(&semaphores->mutex);
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
    // sem_post(&semaphores->mutex);
}

void V(SEMAPHORE *semaphore) {
    // sem_wait(&semaphores->mutex);
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
    // sem_post(&semaphores->mutex);
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
        so_alert("Arquivo nao encontrado");
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

    while (process->priority > aux->priority && aux->next != head) {
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

PROCESS * read_synthetic_program(FILE *fp) {
    PROCESS *process;
    long int code_section;
    int qnt_codes;

    process = malloc(sizeof(PROCESS));

    char name[255];

    // Le cabecalho
    fscanf(fp, "%s %d %d %d\n", name, &process->segment_id, &process->priority,
           &process->segment_size);

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
            so_alert("Numero maximo de semaphores alcancado");
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
    code_section = ftell(fp);
    qnt_codes = count_codes(fp) + 1;

    fseek(fp, code_section, SEEK_SET);


    CODE *code = (CODE *) malloc(sizeof(CODE) * (qnt_codes));

    if (!code) {
        so_alert("Sem memoria");
        exit(EXIT_FAILURE);
    }

    process->qnt_code = qnt_codes;

    char op[51];
    int i = 0;
    while (fgets(op, 51, fp) != NULL) {
        if (op[0] == 'P' || op[0] == 'V') {
            if (!semaphore_process_exists(op[2], process)) {
                char error_msg[255];
                sprintf(error_msg, "O semaphores %c nao existe - Processo %s nao criado", op[2], name);
                so_error(error_msg);
                free(code);
                free(process);
                return NULL;
            }
            code[i].op = op[0] == 'P' ? SEM_P : SEM_V;
            code[i].sem = op[2];
        } else {
            char *left_op = malloc(sizeof(char) * 6);
            int right_op;

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
                char error_msg[255];
                sprintf(error_msg, "Operacao %s invalida - Processo %s nao criado", left_op, name);
                so_error(error_msg);
                free(code);
                free(process);
                return NULL;
            }
            code[i].value = right_op;
            code[i].sem = '#';
        }
        i++;
    }

    process->code = code;
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
                num_lines = num_lines + 1;
            line = getc(fp);
        }
    } else
        num_lines = 0;
    return num_lines;
}

void process_finish(PROCESS *process) {
    if (process) {
        pthread_mutex_lock(&mutex_scheduler);
        interrupt_control(PROCESS_INTERRUPT, (void *) FINISHED);

        segment_free(process->segment_id, process->id);
        remove_scheduler(process);
        pthread_mutex_unlock(&mutex_scheduler);

        free(process->code);
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
    pthread_mutex_lock(&mutex_memory);
    SEGMENT *segmento = &kernel->segment_table.segments[find_segment(process->segment_id)];
    if (segmento->qnt_page_memory < segmento->qnt_page) {
        int request = MEMORY_PAGE_SIZE * (segmento->qnt_page - segmento->qnt_page_memory); // em kbytes
        int remaining = (kernel->segment_table.remaining_memory) - request * K; // memoria remaining em bytes

        if (kernel->segment_table.remaining_memory <= 0)
            page_swap(segmento->qnt_page * MEMORY_PAGE_SIZE); // em kbytes
        else if (remaining < 0)
            page_swap(segmento->qnt_page * MEMORY_PAGE_SIZE - kernel->segment_table.remaining_memory / K);
        load_memory_page(segmento, request);
    }
    pthread_mutex_unlock(&mutex_memory);

    CODE *code = &process->code[process->pc];
    printf("%s - %d\n", process->name, process->pc);
    switch (code->op) {
        case EXEC: {
            if (process->remaining_time < code->value) {
                code->value = code->value - process->remaining_time;
                interrupt_control(PROCESS_INTERRUPT, (void *) QUANTUM_COMPLETED);
            } else {
                process->remaining_time = process->remaining_time - code->value;
                process->pc++;
            }
            break;
        }
        case SEM_P: {
            sys_call(SEMAPHORE_P, find_semaphore(code->sem));

            if (process->state != BLOCKED)
                process->remaining_time = MAX(0, process->remaining_time - 200);
            process->pc++;
            break;
        }
        case SEM_V: {
            sys_call(SEMAPHORE_V, find_semaphore(code->sem));
            process->remaining_time = MAX(0, process->remaining_time - 200);
            process->pc++;
            break;
        }
        default:
            process->pc++;
    }
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
SEGMENT_TABLE segment_table_init() {
    SEGMENT_TABLE segment_table;
    segment_table.segments = NULL;
    segment_table.qnt_segments = 0;
    segment_table.remaining_memory = 1073741824; // (1024^3) em bytes. 1048576 KB. 1024 MB. 1 GB. A pagina é 8 KB.

    return segment_table;
}

void load_memory_page(SEGMENT *segment, int request) {
    SEGMENT_TABLE *tabelaSegmentos = &kernel->segment_table;
    tabelaSegmentos->remaining_memory -= request * K;
    segment->qnt_page_memory = segment->qnt_page;
}

void load_memory_request(PROCESS *process) {
    pthread_mutex_lock(&mutex_memory);
    SEGMENT_TABLE *segment_table = &kernel->segment_table;

    if (find_segment(process->segment_id) == -1) { // Segmento não existe -> cria
        SEGMENT *segment = malloc(sizeof(SEGMENT));
        segment->id = process->segment_id;
        segment->qnt_page = (int) ceil((double) ((double) (process->segment_size) / (MEMORY_PAGE_SIZE)));
        segment->second_chance = 1;

        const int restante = segment_table->remaining_memory - process->segment_size * K;

        if (segment_table->remaining_memory <= 0)
            page_swap(segment->qnt_page * MEMORY_PAGE_SIZE);
        else if (restante < 0) // TODO: testar esse else if
            page_swap(segment->qnt_page * MEMORY_PAGE_SIZE - segment_table->remaining_memory / K);

        load_memory_page(segment, segment->qnt_page * MEMORY_PAGE_SIZE);
        add_segment_table(segment);
    }
    pthread_mutex_unlock(&mutex_memory);
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
        segment_table->remaining_memory =
                MIN(MAX_MEMORY_SIZE,
                    segment_table->remaining_memory + K * (segment.qnt_page_memory * MEMORY_PAGE_SIZE));

        for (; index < qnt - 1; index++)
            segment_table->segments[index] = segment_table->segments[index + 1];

        // Realoca a tabela de segments para quantidade - 1
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
    while (process->priority > aux_process->priority && aux->next != head) {
        prev = aux;
        aux = aux->next;
        aux_process = aux->process;
    }

    new->next = aux;
    prev->next = new;

    if (process->priority < head->process->priority)
        scheduler.head = new;
    if (new->next == scheduler.head)
        scheduler.tail = new;

    return scheduler;
}

SCHEDULER schedule_process(int flag) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESS_SCHEDULER *scheduled = scheduler.scheduled, *new = NULL;

    PROCESS_SCHEDULER *aux = NULL;
    if (scheduled) aux = scheduled->next;
    while (aux && aux->process->state != READY && aux->next != scheduled)
        aux = aux->next;

    if (aux && aux->process->state == READY) {
        new = aux;
        new->process->remaining_time = new->process->max_time;
    }

    if (scheduled) {
        if (flag == SEMAPHORE_BLOCKED)
            scheduled->process->state = BLOCKED;
        else if (flag == FINISHED) {
            scheduled->process->state = DONE;
            if (new == scheduled)
                new = NULL;
        } else // QUANTUM COMPLETED OU NONE
            scheduled->process->state = READY;
    } else if (scheduler.head) {
        new = scheduler.head;
        new->process->remaining_time = new->process->max_time;
    }

    if (new)
        new->process->state = RUNNING;
    scheduler.scheduled = new;
    return scheduler;
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
    PCB pcb = kernel->pcb;
    PROCESS *process = pcb.head;
    if (process) {
        printf("┌───────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-5s │ %-50s │ %-10s │ %-10s │\n", "ID", "Nome", "Estado", "Prioridade");
        printf("└───────────────────────────────────────────────────────────────────────────────────┘\n");
        print_process(process->id, process->name, process->state, process->priority);
        while (process->next != pcb.head) {
            process = process->next;
            print_process(process->id, process->name, process->state, process->priority);
        }
    } else {
        so_alert("┌────────────────────────────────────────┐");
        so_alert("│ NAO EXISTE PROCESSOS NA PCB NO MOMENTO │");
        so_alert("└────────────────────────────────────────┘");
    }

    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    while (!getchar());
}

void print_segment_table() {
    SEGMENT_TABLE table = kernel->segment_table;

    if (table.qnt_segments > 0) {
        printf("┌─────────────────────────────┐\n");
        printf("│ %-27s │\n", "SEGMENTOS");
        printf("└─────────────────────────────┘\n");
        printf("│ %-5s │ %-10s │\n", "ID", "Quantidade Páginas");
        for (int i = 0; i < table.qnt_segments; i++)
            print_segment(table.segments[i].id, table.segments[i].qnt_page_memory);
    } else {
        so_alert("┌──────────────────────────────────────────┐");
        so_alert("│ NAO EXISTE SEGMENTOS ALOCADOS NO MOMENTO │");
        so_alert("└──────────────────────────────────────────┘");
    }

    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    while (!getchar());
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

    return kernel;
}

void sys_call(char function, void *arg) {
    switch (function) {
        case PROCESS_CREATE: {
            process_create((char *) arg);
            break;
        }
        case PROCESS_FINISH: {
            process_finish((PROCESS *) arg);
            break;
        }
        case MEMORY_LOAD_REQUEST: {
            load_memory_request((PROCESS *) arg);
            interrupt_control(MEMORY_LOAD_FINISH, (PROCESS *) arg);
            break;
        }
        case SEMAPHORE_P: {
            P((SEMAPHORE *) arg, kernel->scheduler.scheduled->process);
            break;
        }
        case SEMAPHORE_V: {
            V((SEMAPHORE *) arg);
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
            kernel->scheduler = schedule_process((int) arg);
            break;
        }
        case MEMORY_LOAD_FINISH: {
            PROCESS *process = (PROCESS *) arg;

            kernel->pcb = add_process(process); // Adiciona o process na PCB
            process->state = READY;

            pthread_mutex_lock(&mutex_scheduler);

            kernel->scheduler = add_process_scheduler(process);

            interrupt_control(PROCESS_INTERRUPT, NONE);

            pthread_mutex_unlock(&mutex_scheduler);
            break;
        }
        default: {
            char error_msg[255];
            sprintf(error_msg, "O process %c nao esta definido", function);
            so_alert(error_msg);
        }
    }
}

void process_sleep() {
    interrupt_control(PROCESS_INTERRUPT, (void *) SEMAPHORE_BLOCKED);
}

void process_wakeup(PROCESS *processo) {
    processo->state = READY;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES CPU -------------------------------------------
void cpu_init() {
    pthread_t cpu_id;
    pthread_attr_t cpu_attr;

    pthread_mutex_init(&mutex_scheduler, NULL);
    pthread_mutex_init(&mutex_memory, NULL);

    pthread_attr_init(&cpu_attr);
    pthread_attr_setscope(&cpu_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&cpu_id, NULL, (void *) cpu, NULL);
}

_Noreturn void cpu() {
    while (!kernel);

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start);

    while (1) {
        if (!kernel->scheduler.head)
            continue;
        else if (!kernel->scheduler.scheduled) {
            pthread_mutex_lock(&mutex_scheduler);
            kernel->scheduler = schedule_process(NONE);
            pthread_mutex_unlock(&mutex_scheduler);
        } else {
            do {
                clock_gettime(CLOCK_REALTIME, &end);
                const int elapsed = (end.tv_sec - start.tv_sec) * ONE_SECOND_NS + (end.tv_nsec - start.tv_nsec);

                if (elapsed >= ONE_SECOND_NS) {
                    start = end;
                    pthread_mutex_lock(&mutex_scheduler);
                    run_process(kernel->scheduler.scheduled->process);
                    pthread_mutex_unlock(&mutex_scheduler);
                }
            } while (kernel->scheduler.scheduled != NULL && kernel->scheduler.scheduled->process->remaining_time > 0 &&
                     kernel->scheduler.scheduled->process->pc < kernel->scheduler.scheduled->process->qnt_code);

            if (kernel->scheduler.scheduled == NULL)
                continue;

            if (kernel->scheduler.scheduled->process->pc >= kernel->scheduler.scheduled->process->qnt_code) {
                sys_call(PROCESS_FINISH, (void *) kernel->scheduler.scheduled->process);
            } else {
                pthread_mutex_lock(&mutex_scheduler);
                interrupt_control(PROCESS_INTERRUPT, (void *) QUANTUM_COMPLETED);
                pthread_mutex_unlock(&mutex_scheduler);
            }
        }
    }
}
// ---------------------------------------------------------------------------------------------
