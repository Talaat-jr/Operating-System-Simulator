#include <stdio.h>
#include <stdlib.h>
#define PCB_SIZE 6
#define STACK_SIZE 2

// For parsing the program text file
char **read_program(const char *filename, int *num_lines)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return NULL;
    }

    char **lines;
    int max_lines = 16;
    int num_allocated_lines = 0;
    char line[1024];

    lines = malloc(max_lines * sizeof(char *));
    if (lines == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    // Read the lines from the file and store them in an array
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (num_allocated_lines == max_lines)
        {
            max_lines *= 2;
            lines = realloc(lines, max_lines * sizeof(char *));
            if (lines == NULL)
            {
                printf("Error reallocating memory\n");
                fclose(file);
                free(lines);
                return NULL;
            }
        }

        lines[num_allocated_lines] = malloc(strlen(line) + 1);
        if (lines[num_allocated_lines] == NULL)
        {
            printf("Error allocating memory\n");
            fclose(file);
            for (int i = 0; i < num_allocated_lines; i++)
            {
                free(lines[i]);
            }
            free(lines);
            return NULL;
        }

        strcpy(lines[num_allocated_lines], line);
        num_allocated_lines++;
    }

    *num_lines = num_allocated_lines;
    fclose(file);
    return lines;
}

char **spli1t_instruction(const char *str, int *num_words)
{
    char **words;
    int max_words = 16;
    int num_allocated_words = 0;
    char *word;

    words = malloc(max_words * sizeof(char *));
    if (words == NULL)
    {
        printf("Error allocating memory\n");
        return NULL;
    }

    word = strtok(str, " \t\n");
    while (word != NULL)
    {
        if (num_allocated_words == max_words)
        {
            max_words *= 2;
            words = realloc(words, max_words * sizeof(char *));
            if (words == NULL)
            {
                printf("Error reallocating memory\n");
                for (int i = 0; i < num_allocated_words; i++)
                {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
        }

        words[num_allocated_words] = malloc(strlen(word) + 1);
        if (words[num_allocated_words] == NULL)
        {
            printf("Error allocating memory\n");
            for (int i = 0; i < num_allocated_words; i++)
            {
                free(words[i]);
            }
            free(words);
            return NULL;
        }

        strcpy(words[num_allocated_words], word);
        num_allocated_words++;
        word = strtok(NULL, " \t\n");
    }

    *num_words = num_allocated_words;
    return words;
}

void clear_buffer(const char **buffer, int *num_items)
{
    for (int j = 0; j < num_items; j++)
    {
        free(buffer[j]);
    }
    free(buffer);
}

char *to_string(int number)
{
    //! maximum number of digits in an integer is 3 digits
    char *string = malloc(3 * sizeof(char)); // so that the pointer can point to the heap and not the stack, as local variables get destroied after the function ends
    sprintf(string, "%d", number);
    return string;
}

int to_int(char *string)
{
    return atoi(string);
}

typedef struct
{
    char *name, *value;
} Pair;

typedef struct
{
    int pid;
    int arrival_time;
    int number_of_instructions;
    Pair *pc;
    Pair *state;
    Pair *priority;
    Pair *start_of_variables_section;
} ProcessNeededInformation;

typedef struct
{
    ProcessNeededInformation *data;
    struct node *next;
} node;

typedef struct
{
    int count;
    node *rear;
    node *front;
} queue;

queue *General_Queue;

typedef struct
{
    int number_of_populated_cells;
    Pair cells[60];
} Memory;

typedef struct
{
    int file_free;
    int Process_using;
    queue *file_Blocked_Queue;
} file;

typedef struct
{
    int userInput_free;
    int Process_using;
    queue *userInput_Blocked_Queue;
} userInput;

typedef struct
{
    int userOutput_free;
    int Process_using;
    queue *userOutput_Blocked_Queue;
} userOutput;

file f;
userInput ui;
userOutput uo;

void init_resources()
{
    f.file_free = 1;
    f.Process_using = -1;
    f.file_Blocked_Queue = malloc(sizeof(f.file_Blocked_Queue));
    create((f.file_Blocked_Queue));

    ui.userInput_free = 1;
    ui.Process_using = -1;
    ui.userInput_Blocked_Queue = malloc(sizeof ui.userInput_Blocked_Queue);
    create(ui.userInput_Blocked_Queue);

    uo.userOutput_free = 1;
    uo.Process_using = -1;
    uo.userOutput_Blocked_Queue = malloc(sizeof uo.userOutput_Blocked_Queue);
    create(uo.userOutput_Blocked_Queue);

    General_Queue = malloc(sizeof General_Queue);
    create(General_Queue);
}

void init_PCB(Memory *memory, int process_id)
{
    Pair pcb[PCB_SIZE] =
        {
            {"ID", to_string(process_id)},
            {"State", "Ready"},
            {"Priority", "0"},
            {"PC", to_string(memory->number_of_populated_cells + 9)},
            {"Lower Bound", to_string(memory->number_of_populated_cells)},
            {"Upper Bound", to_string(memory->number_of_populated_cells)}};

    for (int i = 0; i < PCB_SIZE; i++)
        memory->cells[(memory->number_of_populated_cells)++] = pcb[i];
}

void add_process(Memory *memory, char *program_file_path, int process_id, int clk)
{
    init_PCB(memory, process_id);
}

void print_mem(Memory *memory)
{
    for (int i = 0; i < memory->number_of_populated_cells; i++)
    {
        printf("%s: %s\n", (memory->cells[i]).name, (memory->cells[i]).value);
    }
}

void WriteFile(ProcessNeededInformation * p, const char *file_name, const char *variable_name)
{
    FILE *file;
    FILE *file2 = fopen(file_name, "r");
    if (file2 != NULL)
    {
        printf("This file already exist\n");
        return NULL;
    }

    //TODO: should I check if file already exists??

    file = fopen(file_name, "w");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        if (strcmp(memory.cells[(p.start_of_variables_section.value) + i].name, variable_name) == 0)
        {
            fprintf(file, "%s", memory.cells[(p.start_of_variables_section.value) + i].value);
            break;
        }
    }

    //TODO: should I check if variable already exists??

    fclose(file);
    return; // file written successfully
}

void create(queue *q)
{
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

int isempty(queue *q)
{
    if (q->count == 0)
        return 1;
    return 0;
}

int full(queue *q)
{
    if (q->count == STACK_SIZE)
    {
        return 1;
    }
    return 0;
}

void enqueue(queue *q, ProcessNeededInformation *x)
{
    node *temp;
    temp = (node *)malloc(sizeof(node));

    temp->data = x;
    temp->next = NULL;

    if (full(q))
    {
        printf("Not possible. Overflow.");
    }
    else if (isempty(q))
    {
        q->front = q->rear = temp;
        q->count++;
    }
    else
    {
        q->rear->next = temp;
        q->rear = temp;
        q->count++;
    }
}

ProcessNeededInformation *dequeuehelper(queue *q)
{
    if (isempty(q))
    {
        printf("Not possible. Underflow.");
        return;
    }
    node *p;
    p = (node *)malloc(sizeof(node));
    p = q->front;

    q->front = q->front->next;
    q->count--;

    ProcessNeededInformation *x = (p->data);
    return x;
}

ProcessNeededInformation *dequeue(queue *q)
{
    int maxPriority = 99;
    ProcessNeededInformation *temp = NULL;

    for (int i = 0; i < q->count; i++)
    {
        temp = dequeuehelper(q);

        if (temp == NULL)
        {
            return NULL;
        }

        if (maxPriority > to_int(temp->priority->value))
        {
            maxPriority = to_int(temp->priority->value);
        }
        enqueue(q, temp);
    }

    ProcessNeededInformation *temp2;

    for (int i = 0; i < q->count; i++)
    {
        temp2 = dequeuehelper(q);
        if (to_int(temp2->priority->value) == maxPriority)
        {
            return temp2;
        }
        enqueue(q, temp2);
    }
}

int semWaitFile(ProcessNeededInformation *p)
{
    if (f.file_free == 1)
    {
        f.Process_using = p->pid;
        f.file_free = 0;
        printf("files resource has been locked suceesfully by process: %i \n", p->pid);
        return 1;
    }
    else
    {
        p->state->value = "blocked";
        enqueue(General_Queue, p);
        enqueue(f.file_Blocked_Queue, p);
        printf("Request rejected, process:%i has been blcoked as process: %i is currently using the file resource \n", p->pid, f.Process_using);
        return 0;
    }
    return 0;
}

int semSignalFile(ProcessNeededInformation *p)
{
    if (p->pid != f.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(f.file_Blocked_Queue))
        {
            printf("process: %i unlocked the file resource sucessfully. The resource is currently free to use \n", p->pid);
            f.file_free = 1;
            f.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %i unlocked the file resource sucessfully. \n", p->pid);
            ProcessNeededInformation *temp;
            temp = dequeue(f.file_Blocked_Queue);
            // pushToReady(temp);
            f.Process_using = temp->pid;
            temp->state->value = "Ready";
            printf("process: %i is currently using the file resource. \n", temp->pid);
            return 1;
        }
    }
}

int semWaituserInput(ProcessNeededInformation *p)
{
    if (ui.userInput_free)
    {
        ui.Process_using = p->pid;
        ui.userInput_free = 0;
        printf("Input resource has been locked suceesfully by process: %i \n", p->pid);
        return 1;
    }

    p->state->value = "blocked";
    enqueue(General_Queue, p);
    enqueue(ui.userInput_Blocked_Queue, p);
    printf("Request rejected, process: %i has been blcoked as process: %i is currently using the input resource \n", p->pid, ui.Process_using);
    return 0;
}

int semSignaluserInput(ProcessNeededInformation *p)
{
    if (p->pid != ui.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(ui.userInput_Blocked_Queue))
        {
            printf("process: %i unlocked the input resource sucessfully. The resource is currently free to use \n", p->pid);
            ui.userInput_free = 1;
            ui.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %i unlocked sucessfully.", p->pid);
            ProcessNeededInformation *temp;
            temp = dequeue(ui.userInput_Blocked_Queue);
            // pushToReady(temp);
            ui.Process_using = temp->pid;
            temp->state->value = "Ready";
            printf("process: %i is currently using the user input resource.", temp->pid);
            return 1;
        }
    }
}

int semWaituserOutput(ProcessNeededInformation *p)
{
    if (uo.userOutput_free)
    {
        uo.Process_using = p->pid;
        uo.userOutput_free = 0;
        printf("Output resource has been locked suceesfully by process: ", p->pid);
        return 1;
    }
    else
    {
        p->state->value = "blocked";
        enqueue(General_Queue, p);
        enqueue(uo.userOutput_Blocked_Queue, p);
        printf("Request rejected, process: %i has been blcoked as process: %i is currently using the output resource \n", p->pid, uo.Process_using);
        return 0;
    }
}

int semSignaluserOutput(ProcessNeededInformation *p)
{
    if (p->pid != uo.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(uo.userOutput_Blocked_Queue))
        {
            printf("process: %i unlocked the output resource sucessfully. The resource is currently free to use \n", p->pid);
            uo.userOutput_free = 1;
            uo.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %i unlocked sucessfully.", p->pid);
            ProcessNeededInformation *temp;
            temp = dequeue(uo.userOutput_Blocked_Queue);
            // pushToReady(temp);
            uo.Process_using = temp->pid;
            temp->state->value = "Ready";
            printf("process: %i is currently using the user output resource.", temp->pid);
            return 1;
        }
    }
}


int main()
{

    init_resources();

    ProcessNeededInformation p1;
    ProcessNeededInformation p2;

    Pair p = {"pc", "1"};
    Pair pp = {"start_of_variables_section", "1"};
    Pair ppp = {"state", "Ready"};
    Pair pppp = {"priority", "1"};

    Pair b = {"pc", "2"};
    Pair bb = {"start_of_variables_section", "2"};
    Pair bbb = {"state", "Ready"};
    Pair bbbb = {"priority", "2"};

    p1.arrival_time = 1;
    p1.number_of_instructions = 2;
    p1.pc = &p;
    p1.pid = 4;
    p1.priority = &pppp;
    p1.start_of_variables_section = &pp;
    p1.state = &ppp;

    p2.arrival_time = 3;
    p2.number_of_instructions = 2;
    p2.pc = &b;
    p2.pid = 1;
    p2.priority = &bbbb;
    p2.start_of_variables_section = &bb;
    p2.state = &bbb;

    // queue *q;
    // q = malloc(sizeof q);
    // create(q);

    // enqueue(q, &p1);
    // enqueue(q, &p2);

        // printf("%i", to_int(dequeue(q)->priority->value));

    // semWaitFile(&p1);

    // semWaitFile(&p2);

    // semSignalFile(&p1);

    return 0;
}