#include<string.h>
#include<stdlib.h>
#include<stdio.h>

const int INITIAL_MALLOC = 10;

typedef enum {
    DOMESTIC,
    INTERNATIONAL,
} StudentType;

typedef struct {
    char *first_name;
    char *last_name;
    int birth_year;
    int birth_month;
    int birth_day;
    float gpa;
} DomesticStudent;

typedef struct {
    char *first_name;
    char *last_name;
    int birth_year;
    int birth_month;
    int birth_day;
    float gpa;
    int TOEFL;
} InternationalStudent;

typedef union {
    InternationalStudent international;
    DomesticStudent domestic;
} StudentUnion;

// Stores student type and student info so all students can be stored together
typedef struct {
    StudentType type;
    StudentUnion student;
} Student;

/*
Takes fp and line array pointer and loads the file line by line to an array
Must manage realloction as the input size is variable
Also keep track of line_count
Returns pointer to string array
No format error handling
*/
char** read_lines(FILE *input_fp, int size, int *line_count) {
    int current_capacity = INITIAL_MALLOC;
    char **lines = (char **) malloc(sizeof(char *) * INITIAL_MALLOC);
    if (lines == NULL) {
        perror("Failed to allocate.");
        exit(EXIT_FAILURE);
    }
    FILE *newline_fp = input_fp;

    while (1) {
        if (*line_count >= current_capacity) {
            current_capacity *= 2;
            char **temp = realloc(lines, sizeof(char *) * current_capacity);
            if (temp == NULL) {
                free(lines);
                perror("Failed to allocate.");
                exit(EXIT_FAILURE);
            }
            lines = temp;
        }
        
        int line_size = 0;
        while (1) {
            char c = fgetc(newline_fp);
            if (feof(newline_fp) || c == '\n') {
                break;
            }
            line_size++;
        }
        char *line = (char *) malloc(sizeof(char) * line_size);
        if (fgets(line, line_size, input_fp) == NULL) {
            break;
        }
        lines[*line_count] = line;
        (*line_count)++;
    }

    for (int i = 0; i < *line_count; i++) {
        printf("%s\n", lines[i]);
    }

    return lines;
}

/*
Pass in the output fp and a string literal to be written to output file
*/
void output_error(FILE *output_fp, char error[]) {

    fprintf(output_fp, "%s", error);
}

/*
Takes lines and returns a pointer to a student array (unsorted)
Also takes output fp to handle errors by calling output_error()
*/
Student* generate_students_from_lines(char **lines, int line_count, int *student_count, FILE *output_fp) {
    Student *students = (Student *) malloc(sizeof(Student) * INITIAL_MALLOC);
    return students;
}

/*
Outputs domestic (option 1)
*/
void output_domestic(FILE *output_fp, Student *students, int student_count) {

}

/*
Outputs international (option 2)
*/
void output_international(FILE *output_fp, Student *students, int student_count) {

}

/*
Outputs both (option3)
*/
void output_both(FILE *output_fp, Student *students, int student_count) {

}

/*
Takes student array and sorts it in place
*/
void merge_sort(Student *student, int student_count) {

}

/*
Returns 1 if student a is less than student b
Returns -1 if student b is less than student a
Returns 0 if student a is equal to student b
*/
int student_comparator(Student a, Student b) {
    return 1;
}

/*
Takes a month as string and returns int for comparison
Ex. "Jan" => 1, "Feb" => 2, etc.

EXIT_FAILURE if invalid month
*/
int month_to_int(char month[]) {
    char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    for (int i = 0; i < 12; i++) {
        if (strcmp(months[i], month) == 0) {
            return i + 1;  // Arrays are 0-based, months are 1-based
        }
    }

    perror("Inavlid month was given.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    // A numbers of everyone. AXXXX_AXXXX_AXXX format.
    char *A_Num = "A01340526_A01062792_A01300754";

    // Outputs A number file
    FILE *a_num_fp = fopen(A_Num, "w");
    if (a_num_fp == NULL) {
        printf("Failed to create the output file.\n");
        return 1;
    }
    fclose(a_num_fp);

    // Validating arguments
    if (argc != 4) {
        printf("Usage: %s <input_file> <a_num_fp> <option>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Validating option argument
    int option = atoi(argv[3]);
    if (option < 1 || option > 3) {
        printf("Usage: <option> must be an intger between 1 and 3 (inclusive)\n");
        return 1;
    }

    FILE *input_fp = fopen(argv[1], "r"); // Input file
    int size = 0; // Input size in bytes

    FILE *output_fp = fopen(argv[2], "w"); // Output file

    // Checks if files exist
    if (input_fp == NULL || output_fp == NULL) {
        perror("Error opening one or more files.\n");
        return EXIT_FAILURE;
    }
    // Checks if empty input file and assigns size
    if (NULL != input_fp) {
        fseek (input_fp, 0, SEEK_END);
        size = ftell(input_fp);
        if (0 == size) {
            return EXIT_FAILURE;
        }
        rewind(input_fp);
    }

    int line_count = 0;
    char **lines = read_lines(input_fp, size, &line_count);
    int student_count = 0;
    Student *students = generate_students_from_lines(lines, line_count, &student_count, output_fp);
    merge_sort(students, student_count);

    // Output to file based on option
    switch (option) {
        case 1: {
            output_domestic(output_fp, students, student_count);
            break;
        }
        case 2: {
            output_international(output_fp, students, student_count);
            break;
        }
        case 3: {
            output_both(output_fp, students, student_count);
            break;
        }
    }

    // Free and close
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
    free(students);
    fclose(input_fp);
    fclose(output_fp);
    return 0;
}