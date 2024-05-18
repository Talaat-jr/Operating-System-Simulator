#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PCB_SIZE 6
#define NUM_VARIABLES_PER_PROCESS 3
#define NUM_QUEUES 4

// globally defined to be automatically initialized

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
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        strcpy(lines[num_allocated_lines], line);
        num_allocated_lines++;
    }

    *num_lines = num_allocated_lines;
    fclose(file);
    return lines;
}

char **split_instruction(const char *str, int *num_words)
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
    ProcessNeededInformation *processes[100]; // Array to hold processes in the queue
    int count;                                // Number of processes in the queue
} Queue;

typedef struct
{
    Queue level_queues[NUM_QUEUES];
    Queue ready_queue;
    ProcessNeededInformation *running_process;
    int quantum;
} MLFQ;

Memory memory;
MLFQ scheduler_queue;

ProcessNeededInformation *dequeue(Queue *queue)
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

void enqueue(Queue *queue, ProcessNeededInformation *process)
{
    queue->processes[queue->count++] = process;
}

void push_to_ready_queue(ProcessNeededInformation *process)
{
    enqueue(&(scheduler_queue.ready_queue), process);
}

void add_ready_processes()
{
    int ready_queue_size = scheduler_queue.ready_queue.count;
    for (int i = 0; i < ready_queue_size; i++)
    {
        ProcessNeededInformation *process = dequeue(&(scheduler_queue.ready_queue));
        int priority_level = to_int(process->priority->value);
        enqueue(&(scheduler_queue.level_queues[priority_level]), process);
    }
}

ProcessNeededInformation *get_highest_priority_process(int *quantum)
{
    for (int i = 0; i < NUM_QUEUES; i++)
    {
        if (scheduler_queue.level_queues[i].count > 0)
        {
            *quantum = 1 << i;
            return dequeue(&(scheduler_queue.level_queues[i]));
        }
    }
    *quantum = 0;
    return NULL;
}

ProcessNeededInformation *pull_executable_process()
{
    add_ready_processes();
    if (scheduler_queue.quantum == 0)
    {
        if (scheduler_queue.running_process != NULL)
        {
            scheduler_queue.running_process->priority->value = to_string(to_int(scheduler_queue.running_process->priority->value) + 1);
        }

        scheduler_queue.running_process = get_highest_priority_process(&(scheduler_queue.quantum));
    }
    else
    {
        if (strcmp(scheduler_queue.running_process->state->value, "blocked") == 0)
        {
            scheduler_queue.running_process = get_highest_priority_process(&(scheduler_queue.quantum));
        }
        else
        {
            scheduler_queue.quantum--;
        }
    }
}

void print_PNI(ProcessNeededInformation *process)
{
    printf("Process ID : %s\n", process->pid->value);
    printf("Priority: %s\n", process->priority->value);
}

void print_MLFQ()
{
    printf("----------------------------------- Ready Queue ---------------------------------\n");
    for (int i = 0; i < scheduler_queue.ready_queue.count; i++)
    {
        print_PNI(scheduler_queue.ready_queue.processes[i]);
    }
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < NUM_QUEUES; i++)
    {
        printf("----------------------------------- Queue %d ---------------------------------\n", i);
        if (scheduler_queue.level_queues[i].count > 0)
        {
            int const count = scheduler_queue.level_queues[i].count;
            for (int j = 0; j < count; j++)
            {
                print_PNI(scheduler_queue.level_queues[i].processes[j]);
            }
        }
        printf("--------------------------------------------------------------------------------\n");
    }
}

void init_PCB(int process_id, ProcessNeededInformation *process)
{

    process->pid = &memory.cells[(memory.number_of_populated_cells)];
    process->priority = &memory.cells[(memory.number_of_populated_cells) + 2];

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

void init_variables_section()
{
    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
        (memory.number_of_populated_cells)++;
}

void init_process_instructoins(char *program_file_path)
{
    int num_lines;
    char **lines = read_program(program_file_path, &num_lines);
    const int upper_bound_index = (memory.number_of_populated_cells) - 4;
    memory.cells[upper_bound_index].value = to_string((memory.number_of_populated_cells) + num_lines - 1);
    if (lines == NULL)
    {
        printf("Error reading program file\n");
        return;
    }

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
    ProcessNeededInformation *p = malloc(sizeof(ProcessNeededInformation));
    init_PCB(process_id, p);
    init_variables_section(memory);
    init_process_instructoins(program_file_path);
    push_to_ready_queue(p);
}

void print_mem()
{
    for (int i = 0; i < 60; i++)
    {
        printf("%s: %s\n", (memory.cells[i]).name, (memory.cells[i]).value);
    }
}

void main()
{

    add_process("Program_1.txt", 0, 0);
    add_process("Program_2.txt", 1, 0);
    print_mem();
    pull_executable_process();
    print_MLFQ();
}
