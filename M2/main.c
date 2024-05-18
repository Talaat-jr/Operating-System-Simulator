#include <stdio.h>
#include <stdlib.h>
#define PCB_SIZE 6
#define NUM_VARIABLES_PER_PROCESS 3

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
    int number_of_populated_cells;
    Pair cells[60];
} Memory;

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

void init_variables_section(Memory *memory)
{
    for (int i = 0; i < NUM_VARIABLES_PER_PROCESS; i++)
        (memory->number_of_populated_cells)++;
}

void init_process_instructoins(Memory *memory, char *program_file_path)
{
    int num_lines;
    char **lines = read_program(program_file_path, &num_lines);
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

        memory->cells[(memory->number_of_populated_cells)++] = (Pair){instructoin_number, instruction};
    }

    clear_buffer(lines, num_lines);
}

void add_process(Memory *memory, char *program_file_path, int process_id, int clk)
{
    init_PCB(memory, process_id);
    init_variables_section(memory);
    init_process_instructoins(memory, program_file_path);
}

void print_mem(Memory *memory)
{
    for (int i = 0; i < 60; i++)
    {
        printf("%s: %s\n", (memory->cells[i]).name, (memory->cells[i]).value);
    }
}

Memory memory; // globally defined to be automatically initialized

void main()
{

    int processs_id = 0;

    add_process(&memory, "Program_1.txt", processs_id++, 0);
    add_process(&memory, "Program_2.txt", processs_id++, 0);
    add_process(&memory, "Program_3.txt", processs_id++, 0);
    print_mem(&memory);
}