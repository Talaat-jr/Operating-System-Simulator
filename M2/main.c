#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PCB_SIZE 6
#define NUM_VARIABLES_PER_PROCESS 3
#define NUM_QUEUES 4
#define STACK_SIZE 3

// globally defined to be automatically initialized
int terminate = 0;

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
        // removing the new line character
        if (line[strlen(line) - 2] == '\r')
            line[strlen(line) - 2] = '\0';
        strcpy(lines[num_allocated_lines], line);
        num_allocated_lines++;
    }

    *num_lines = num_allocated_lines;
    fclose(file);
    return lines;
}

char **split_instruction(char *str, int *num_words)
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

void clear_buffer(char **buffer, int num_items)
{
    for (int j = 0; j < num_items; j++)
        free(buffer[j]);

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
    int number_of_populated_cells;
    Pair cells[60];
} Memory;

typedef struct
{
    int arrival_time;
    int number_of_instructions;
    Pair *pid;
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
} Queue;

typedef struct
{
    int file_free;
    int Process_using;
    Queue *file_Blocked_Queue;
} FileMutex;

typedef struct
{
    int userInput_free;
    int Process_using;
    Queue *userInput_Blocked_Queue;
} UserInputMutex;

typedef struct
{
    int userOutput_free;
    int Process_using;
    Queue *userOutput_Blocked_Queue;
} UserOutputMutex;

typedef struct
{
    ProcessNeededInformation *processes[100]; // Array to hold processes in the queue
    int count;                                // Number of processes in the queue
} SchedulingQueue;

typedef struct
{
    SchedulingQueue level_queues[NUM_QUEUES];
    SchedulingQueue ready_queue;
    ProcessNeededInformation *running_process;
    int quantum;
} MLFQ;

// globally defined to be automatically initialized
Memory memory;
FileMutex file_mutex;
UserInputMutex userInput_mutex;
UserOutputMutex userOutput_mutex;
MLFQ scheduler_queue;
Queue *general_queue;

// Function prototypes to avoid implicit declaration
ProcessNeededInformation *pull_executable_process();
void push_to_ready_queue(ProcessNeededInformation *process);

void create(Queue *q)
{
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

int isempty(Queue *q)
{
    if (q->count == 0)
        return 1;
    return 0;
}

int full(Queue *q)
{
    if (q->count == STACK_SIZE)
    {
        return 1;
    }
    return 0;
}

void enqueue(Queue *q, ProcessNeededInformation *p)
{
    node *temp;
    temp = (node *)malloc(sizeof(node));

    temp->data = p;
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
        q->rear->next = (struct node *)temp;
        q->rear = temp;
        q->count++;
    }
}

ProcessNeededInformation *dequeuehelper(Queue *q)
{
    if (isempty(q))
    {
        printf("Not possible. Underflow.");
        return NULL;
    }
    node *p;
    p = (node *)malloc(sizeof(node));
    p = q->front;

    q->front = (node *)(q->front->next);
    q->count--;

    ProcessNeededInformation *x = (p->data);
    return x;
}

ProcessNeededInformation *dequeue(Queue *q)
{
    int maxPriority = 99;
    ProcessNeededInformation *temp = NULL;

    for (int i = 0; i < q->count; i++)
    {
        if (isempty(q))
        {
            printf("Queue is empty");
            return NULL;
        }
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

ProcessNeededInformation *dequeue_element(Queue *q, ProcessNeededInformation *p)
{
    if (isempty(q))
    {
        printf("Not possible. Underflow.");
        return NULL;
    }

    for (int i = 0; i < q->count; i++)
    {
        ProcessNeededInformation *temp = dequeuehelper(q);
        if (temp->pid->value == p->pid->value)
        {
            return temp;
        }
        enqueue(q, temp);
    }
}

int semWaitFile(ProcessNeededInformation *p)
{
    if (file_mutex.file_free == 1)
    {
        file_mutex.Process_using = to_int(p->pid->value);
        file_mutex.file_free = 0;
        printf("files resource has been locked suceesfully by process: %s \n", p->pid->value);
        return 1;
    }
    else
    {
        p->state->value = "Blocked";
        enqueue(general_queue, p);
        enqueue(file_mutex.file_Blocked_Queue, p);
        printf("Request rejected, process:%s has been blcoked as process: %i is currently using the file resource \n", p->pid->value, file_mutex.Process_using);
        return 0;
    }
    return 0;
}

int semSignalFile(ProcessNeededInformation *p)
{
    if (to_int(p->pid->value) != file_mutex.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(file_mutex.file_Blocked_Queue))
        {
            printf("process: %s unlocked the file resource sucessfully. The resource is currently free to use \n", p->pid->value);
            file_mutex.file_free = 1;
            file_mutex.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %s unlocked the file resource sucessfully. \n", p->pid->value);
            ProcessNeededInformation *temp;
            temp = dequeue(file_mutex.file_Blocked_Queue);
            dequeue_element(general_queue, temp);
            push_to_ready_queue(temp);
            file_mutex.Process_using = to_int(temp->pid->value);
            temp->state->value = "Ready";
            printf("process: %s is currently using the file resource. \n", temp->pid->value);
            return 1;
        }
    }
}

int semWaituserInput(ProcessNeededInformation *p)
{
    if (userInput_mutex.userInput_free)
    {
        userInput_mutex.Process_using = to_int(p->pid->value);
        userInput_mutex.userInput_free = 0;
        printf("Input resource has been locked suceesfully by process: %s \n", p->pid->value);
        return 1;
    }

    p->state->value = "Blocked";
    enqueue(general_queue, p);
    enqueue(userInput_mutex.userInput_Blocked_Queue, p);
    printf("Request rejected, process: %s has been blcoked as process: %i is currently using the input resource \n", p->pid->value, userInput_mutex.Process_using);
    return 0;
}

int semSignaluserInput(ProcessNeededInformation *p)
{
    if (to_int(p->pid->value) != userInput_mutex.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(userInput_mutex.userInput_Blocked_Queue))
        {
            printf("process: %s unlocked the input resource sucessfully. The resource is currently free to use \n", p->pid->value);
            userInput_mutex.userInput_free = 1;
            userInput_mutex.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %s unlocked sucessfully.", p->pid->value);
            ProcessNeededInformation *temp;
            temp = dequeue(userInput_mutex.userInput_Blocked_Queue);
            dequeue_element(general_queue, temp);
            push_to_ready_queue(temp);
            userInput_mutex.Process_using = to_int(temp->pid->value);
            temp->state->value = "Ready";
            printf("process: %s is currently using the user input resource.", temp->pid->value);
            return 1;
        }
    }
}

int semWaituserOutput(ProcessNeededInformation *p)
{
    if (userOutput_mutex.userOutput_free)
    {
        userOutput_mutex.Process_using = to_int(p->pid->value);
        userOutput_mutex.userOutput_free = 0;
        printf("Output resource has been locked suceesfully by process: %s", p->pid->value);
        return 1;
    }
    else
    {
        p->state->value = "Blocked";
        enqueue(general_queue, p);
        enqueue(userOutput_mutex.userOutput_Blocked_Queue, p);
        printf("Request rejected, process: %s has been blcoked as process: %i is currently using the output resource \n", p->pid->value, userOutput_mutex.Process_using);
        return 0;
    }
}

int semSignaluserOutput(ProcessNeededInformation *p)
{
    if (to_int(p->pid->value) != userOutput_mutex.Process_using)
    {
        printf("Rejected as you are not the process that blocked this resource \n");
        return 0;
    }
    else
    {
        if (isempty(userOutput_mutex.userOutput_Blocked_Queue))
        {
            printf("process: %s unlocked the output resource sucessfully. The resource is currently free to use \n", p->pid->value);
            userOutput_mutex.userOutput_free = 1;
            userOutput_mutex.Process_using = -1;
            return 1;
        }
        else
        {
            printf("process: %s unlocked sucessfully.", p->pid->value);
            ProcessNeededInformation *temp;
            temp = dequeue(userOutput_mutex.userOutput_Blocked_Queue);
            dequeue_element(general_queue, temp);
            push_to_ready_queue(temp);
            userOutput_mutex.Process_using = to_int(temp->pid->value);
            temp->state->value = "Ready";
            printf("process: %s is currently using the user output resource.", temp->pid->value);
            return 1;
        }
    }
}

void init_resources()
{
    file_mutex.file_free = 1;
    file_mutex.Process_using = -1;
    file_mutex.file_Blocked_Queue = malloc(sizeof(file_mutex.file_Blocked_Queue));
    create((file_mutex.file_Blocked_Queue));

    userInput_mutex.userInput_free = 1;
    userInput_mutex.Process_using = -1;
    userInput_mutex.userInput_Blocked_Queue = malloc(sizeof userInput_mutex.userInput_Blocked_Queue);
    create(userInput_mutex.userInput_Blocked_Queue);

    userOutput_mutex.userOutput_free = 1;
    userOutput_mutex.Process_using = -1;
    userOutput_mutex.userOutput_Blocked_Queue = malloc(sizeof userOutput_mutex.userOutput_Blocked_Queue);
    create(userOutput_mutex.userOutput_Blocked_Queue);

    general_queue = malloc(sizeof general_queue);
    create(general_queue);
}

void print(ProcessNeededInformation *p, char *variable_name)
{
    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (strcmp(current_variable->name, variable_name) == 0)
        {
            printf("%s\n", current_variable->value);
            break;
        }
    }
}

void print_from_to_helper(int from, int to)
{
    for (int i = from; i <= to; i++)
    {
        printf("%d ", i);
    }
    printf("\n");
}

void print_from_to(ProcessNeededInformation *p, char *variable_name1, char *variable_name2)
{
    int from = -1, to = -1;
    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (current_variable->name == NULL)
            continue;
        if (strcmp(current_variable->name, variable_name1) == 0)
        {
            from = to_int(current_variable->value);
        }
        if (strcmp(current_variable->name, variable_name2) == 0)
        {
            to = to_int(current_variable->value);
        }
    }

    if (from == -1 || to == -1)
    {
        printf("Variable not found\n");
        return;
    }
    print_from_to_helper(from, to);
}

void assign(ProcessNeededInformation *p, char *variable_name, char *value)
{
    // TODO: check if the var
    char *temp = malloc(10 * sizeof(char));
    strcpy(temp, variable_name);

    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (current_variable->name == NULL)
        {
            current_variable->name = temp;
            current_variable->value = value;
            break;
        }
        else if (strcmp(current_variable->name, variable_name) == 0)
        {
            current_variable->value = value;
            break;
        }
    }
}

void writeFile(ProcessNeededInformation *p, char *file_name, char *variable_name)
{

    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (current_variable->name == NULL)
            continue;
        if (strcmp(current_variable->name, file_name) == 0)
        {
            file_name = current_variable->value;
        }
    }

    FILE *file;
    FILE *file2 = fopen(file_name, "r");
    if (file2 != NULL)
    {
        printf("This file already exist\n");
        return;
    }

    // TODO: should I check if file already exists??

    file = fopen(file_name, "w");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (strcmp(current_variable->name, variable_name) == 0)
        {
            fprintf(file, "%s", current_variable->value);
            break;
        }
    }

    // TODO: should I check if variable already exists??

    fclose(file);
    return; // file written successfully
}

char *readFile(ProcessNeededInformation *p, char *file_name)
{
    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
    {
        Pair *current_variable = (p->start_of_variables_section + i);
        if (current_variable->name == NULL)
            continue;
        if (strcmp(current_variable->name, file_name) == 0)
        {
            file_name = current_variable->value;
        }
    }

    FILE *filePointer;
    char *fileContent;
    long fileSize;

    // Open the file in binary mode
    filePointer = fopen(file_name, "rb");
    if (filePointer == NULL)
    {
        perror("Failed to open the file");
        return fileContent;
    }

    // Seek to the end of the file to determine its size
    if (fseek(filePointer, 0, SEEK_END) != 0)
    {
        perror("Failed to seek to the end of the file");
        fclose(filePointer);
        return fileContent;
    }

    fileSize = ftell(filePointer); // Get the current file pointer position, which is the file size
    if (fileSize == -1)
    {
        perror("Failed to get the file size");
        fclose(filePointer);
        return fileContent;
    }

    rewind(filePointer); // Go back to the beginning of the file

    // Allocate memory to hold the file content plus a null terminator
    fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL)
    {
        perror("Failed to allocate memory");
        fclose(filePointer);
        return fileContent;
    }

    // Read the file content into the allocated memory
    size_t bytesRead = fread(fileContent, 1, fileSize, filePointer);
    if (bytesRead != fileSize)
    {
        perror("Failed to read the file content");
        free(fileContent);
        fclose(filePointer);
        return fileContent;
    }

    // Null-terminate the file content
    fileContent[fileSize] = '\0';

    // Close the file
    fclose(filePointer);

    return fileContent;
}

char *takeInput()
{
    printf("Enter a value: ");
    char str[100];
    scanf(" %[^\n]", str); // Fixing the scanf format string

    // Allocate memory dynamically for the input string
    char *result = malloc(strlen(str) + 1);
    if (result != NULL)
    {
        strcpy(result, str); // Copy the input to the allocated memory
    }
    return result;
}

void execute_instruction(ProcessNeededInformation *p, char *instruction)
{
    char **words;
    int num_words;
    words = split_instruction(instruction, &num_words);
    if (strcmp(words[0], "semWait") == 0)
    {
        if (strcmp(words[1], "file") == 0)
        {
            semWaitFile(p);
        }
        else if (strcmp(words[1], "userInput") == 0)
        {
            semWaituserInput(p);
        }
        else if (strcmp(words[1], "userOutput") == 0)
        {
            semWaituserOutput(p);
        }
    }
    else if (strcmp(words[0], "semSignal") == 0)
    {
        if (strcmp(words[1], "file") == 0)
        {
            semSignalFile(p);
        }
        else if (strcmp(words[1], "userInput") == 0)
        {
            semSignaluserInput(p);
        }
        else if (strcmp(words[1], "userOutput") == 0)
        {
            semSignaluserOutput(p);
        }
    }
    else if (strcmp(words[0], "print") == 0)
    {
        print(p, words[1]);
    }
    else if (strcmp(words[0], "printFromTo") == 0)
    {
        print_from_to(p, words[1], words[2]);
    }
    else if (strcmp(words[0], "writeFile") == 0)
    {
        writeFile(p, words[1], words[2]);
    }
    else if (strcmp(words[0], "assign") == 0)
    {
        char *value;
        int is_file_read = ((num_words == 4) && (strcmp(words[2], "readFile") == 0));
        int is_input = ((num_words == 3) && (strcmp(words[2], "input") == 0));

        if (is_file_read)
            value = readFile(p, words[3]);
        else if (is_input)
            value = takeInput();
        else
            value = words[2];

        assign(p, words[1], value);
    }
    else if (strcmp(words[0], "readFile") == 0)
    {
        // TODO: meaningless to only read file without doing anything with it
    }
    clear_buffer(words, num_words);
}

void kernel()
{

    ProcessNeededInformation *p = pull_executable_process();
    if (p == NULL)
    {
        printf("No process to execute\n");
        terminate = 1;
        return;
    }

    int pc = to_int(p->pc->value);
    char *instruction = memory.cells[pc].value;
    printf("Executing instruction: %s\n", instruction);

    execute_instruction(p, instruction);
    p->pc->value = to_string(pc + 1);
    p->number_of_instructions--;

    if (p->number_of_instructions <= 0)
    {
        p->state->value = "Terminated";
    }
}

void print_PNI(ProcessNeededInformation *process)
{
    printf("Process ID : %s\n", process->pid->value);
    printf("Priority: %s\n", process->priority->value);
    printf("State: %s\n", process->state->value);
    printf("PC: %s\n", process->pc->value);
    printf("Arrival Time: %i\n", process->arrival_time);
}

void print_MLFQ()
{

    printf("******************************************************** Ready Queue ********************************************************\n");
    for (int i = 0; i < scheduler_queue.ready_queue.count; i++)
    {
        print_PNI(scheduler_queue.ready_queue.processes[i]);
    }

    for (int i = 0; i < NUM_QUEUES; i++)
    {
        printf("********************************************************** Queue %d **********************************************************\n", i);
        if (scheduler_queue.level_queues[i].count > 0)
        {
            int const count = scheduler_queue.level_queues[i].count;
            for (int j = 0; j < count; j++)
            {
                print_PNI(scheduler_queue.level_queues[i].processes[j]);
            }
        }
        printf("*****************************************************************************************************************************\n");
    }
    printf("****************************************************** Running Process ******************************************************\n");
    if (scheduler_queue.running_process != NULL)
    {
        print_PNI(scheduler_queue.running_process);
    }
    printf("Remaining Quantum : %i\n", scheduler_queue.quantum);
    printf("*****************************************************************************************************************************\n");
}

ProcessNeededInformation *scheduling_dequeue(SchedulingQueue *queue)
{
    if (queue->count == 0)
        return NULL;
    ProcessNeededInformation *process = queue->processes[0];
    for (int i = 1; i < queue->count; i++)
    {
        queue->processes[i - 1] = queue->processes[i];
    }
    queue->count--;
    return process;
}

void scheduling_enqueue(SchedulingQueue *queue, ProcessNeededInformation *process)
{
    queue->processes[queue->count++] = process;
}

void push_to_ready_queue(ProcessNeededInformation *process)
{
    scheduling_enqueue(&(scheduler_queue.ready_queue), process);
}

void add_ready_processes()
{
    int ready_queue_size = scheduler_queue.ready_queue.count;
    for (int i = 0; i < ready_queue_size; i++)
    {
        ProcessNeededInformation *process = scheduling_dequeue(&(scheduler_queue.ready_queue));
        int priority_level = to_int(process->priority->value);
        scheduling_enqueue(&(scheduler_queue.level_queues[priority_level]), process);
    }
}

ProcessNeededInformation *get_highest_priority_process(int *quantum)
{
    for (int i = 0; i < NUM_QUEUES; i++)
    {
        if (scheduler_queue.level_queues[i].count > 0)
        {
            *quantum = (1 << i) - 1; // -1 to consider that this process is going to be executed atleast once after being pulled
            return scheduling_dequeue(&(scheduler_queue.level_queues[i]));
        }
    }
    *quantum = 0;
    return NULL;
}

ProcessNeededInformation *pull_executable_process()
{
    add_ready_processes();
    if (scheduler_queue.running_process == NULL)
    {
        ProcessNeededInformation *process = get_highest_priority_process(&(scheduler_queue.quantum));
        if (process != NULL)
        {
            printf("Process which will be running:\n");
            print_PNI(process);
        }
        return scheduler_queue.running_process = process;
    }

    if (scheduler_queue.running_process->number_of_instructions <= 0)
    {
        scheduler_queue.running_process->state->value = "Terminated";
        printf("Process that has been Terminated:\n");
        print_PNI(scheduler_queue.running_process);
    }

    else if (scheduler_queue.quantum == 0)
    {
        int priority_level = to_int(scheduler_queue.running_process->priority->value);
        if (priority_level < 3)
        {
            priority_level++;
            scheduler_queue.running_process->priority->value = to_string(priority_level);
        }
        if (strcmp(scheduler_queue.running_process->state->value, "Blocked") != 0)
            scheduling_enqueue(&(scheduler_queue.level_queues[priority_level]), scheduler_queue.running_process);
    }
    else
    {
        if (strcmp(scheduler_queue.running_process->state->value, "Blocked") == 0)
        {
            printf("Process that has been blocked:\n");
            print_PNI(scheduler_queue.running_process);
        }
        else
        {
            scheduler_queue.quantum--;
            return scheduler_queue.running_process;
        }
    }
    ProcessNeededInformation *process = get_highest_priority_process(&(scheduler_queue.quantum));
    if (process != NULL)
    {
        printf("Process which will be running:\n");
        print_PNI(process);
    }
    return scheduler_queue.running_process = process;
}

void init_PCB(int process_id, ProcessNeededInformation *pni)
{

    pni->pid = &memory.cells[(memory.number_of_populated_cells)];
    pni->state = &memory.cells[(memory.number_of_populated_cells) + 1];
    pni->priority = &memory.cells[(memory.number_of_populated_cells) + 2];
    pni->pc = &memory.cells[(memory.number_of_populated_cells) + 3];

    Pair pcb[PCB_SIZE] =
        {
            {"ID", to_string(process_id)},
            {"State", "Ready"},
            {"Priority", "0"},
            {"PC", to_string(memory.number_of_populated_cells + 9)},
            {"Lower Bound", to_string(memory.number_of_populated_cells)},
            {"Upper Bound", to_string(memory.number_of_populated_cells)}};

    for (int i = 0; i < PCB_SIZE; i++)
        memory.cells[(memory.number_of_populated_cells)++] = pcb[i];
}

void init_variables_section(ProcessNeededInformation *pni)
{
    pni->start_of_variables_section = &memory.cells[(memory.number_of_populated_cells)];

    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
        (memory.number_of_populated_cells)++;
}

void init_process_instructoins(char *program_file_path, ProcessNeededInformation *pni)
{
    int num_lines;
    char **lines = read_program(program_file_path, &num_lines);
    if (lines == NULL)
    {
        printf("Error reading program file\n");
        return;
    }

    const int upper_bound_index = (memory.number_of_populated_cells) - 4;
    memory.cells[upper_bound_index].value = to_string((memory.number_of_populated_cells) + num_lines - 1);
    pni->number_of_instructions = num_lines;

    for (int i = 0; i < num_lines; i++)
    {
        char *instructoin_number = malloc(10 * sizeof(char));
        char *instruction = malloc((strlen(lines[i]) + 5) * sizeof(char));
        sprintf(instructoin_number, "Inst_%d", i);
        sprintf(instruction, "%s", lines[i]);

        memory.cells[(memory.number_of_populated_cells)++] = (Pair){instructoin_number, instruction};
    }

    clear_buffer(lines, num_lines);
}

void add_process(char *program_file_path, int process_id, int clk)
{
    ProcessNeededInformation *pni = malloc(sizeof(ProcessNeededInformation));
    pni->arrival_time = clk;

    init_PCB(process_id, pni);
    init_variables_section(pni);
    init_process_instructoins(program_file_path, pni);

    push_to_ready_queue(pni);
}

void print_mem()
{
    for (int i = 0; i < 60; i++)
    {
        printf("Cell %i => %s: %s | ", i, (memory.cells[i]).name, (memory.cells[i]).value);
        if ((i != 0) && (i % 6 == 0))
            printf("\n");
    }
    printf("\n___________________________________________________________________________________________________________________________________________________________________________________________________________\n\n");
}

void print_general_queue()
{
    printf("******************************************************* General Queue *******************************************************\n");

    for (int i = 0; i < general_queue->count; i++)
    {
        ProcessNeededInformation *temp = dequeuehelper(general_queue);
        print_PNI(temp);
        enqueue(general_queue, temp);
    }

    printf("*****************************************************************************************************************************\n");
}

void main()
{
    int pid = 0;
    int clk = 1;
    init_resources();

    while (1)
    {
        printf("Clock cycle: %i\n", clk);
        printf("Do you want to add a process? (y/n): ");
        char response;
        scanf(" %c", &response);
        if (response == 'y')
        {
            char program_file_path[100];
            printf("Enter the program file name: ");
            scanf(" %s", program_file_path);
            add_process(program_file_path, pid++, clk);
        }
        kernel();
        print_MLFQ();
        print_general_queue();
        print_mem();
        clk++;
        if (terminate)
        {
            printf("No more processes to execute \n");
            printf("Do you want to exit? (y/n): ");
            scanf(" %c", &response);
            if (response == 'y')
                break;
            else
            {
                terminate = 0;
            }
        }
    }
}