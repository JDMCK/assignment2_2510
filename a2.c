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
    char *birth_month;
    int birth_day;
    char *gpa_str;
} DomesticStudent;

typedef struct {
    char *first_name;
    char *last_name;
    int birth_year;
    char *birth_month;
    int birth_day;
    char *gpa_str;
    int TOEFL_score;
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

        int position = ftell(input_fp);
        if (position == -1) {
            perror("ftell failed.");
            break;
        }
        
        int line_size = 0;
        char c;
        while ((c = fgetc(input_fp)) != '\n' && c != EOF) {
            line_size++;
        }
        // Accounts for new line and null terminator
        line_size += 2;

        // Moves pointer back after counting size
        fseek(input_fp, position, SEEK_SET);

        char *line = (char *) malloc(sizeof(char) * line_size);
        if (line == NULL) {
            perror("Failed to allocate.");
            exit(EXIT_FAILURE);
        }
        if (fgets(line, line_size, input_fp) == NULL) {
            free(line);

            if (feof(input_fp)) {
                break;
            }
            continue;
        }
        line[strcspn(line, "\n")] = '\0';

        lines[*line_count] = line;
        (*line_count)++;

        if (c == EOF) {
            break;
        }
    }

    return lines;
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

    return -1;
}

/*
Pass in the output fp and a string literal to be written to output file
*/
void output_error(FILE *output_fp, char error[]) {

    fprintf(output_fp, "ERROR: %s", error);
    exit(EXIT_FAILURE);
}

/*
Takes lines and returns a pointer to a student array (unsorted)
Also takes output fp to handle errors by calling output_error()
*/
Student parse_line(char *line, FILE *output_fp) {
    Student student;

    // Temporaries to hold parts of the input
    char first_name[50];
    char last_name[50];
    char date[15] = "";
    char gpa_str[6] = "";
    char type;
    int TOEFL_score = -1;

    // Parse input assuming the last field can be 'I TOEFL' or 'D'
    sscanf(line, "%s %s %s %s %c %d", first_name, last_name, date, gpa_str, &type, &TOEFL_score);

    // Parse the birth date
    int birth_year, birth_day;
    char birth_month[4];
    sscanf(date, "%3s-%d-%d", birth_month, &birth_day, &birth_year);
    birth_month[3] = '\0';

    // Checks for invalid inputs
    
    // Checks for date
    if (date[0] == '\0') output_error(output_fp, "Invalid date");
    if (birth_year < 1950 || birth_year > 2010) output_error(output_fp, "Invalid birth year");
    if (month_to_int(birth_month) == -1) output_error(output_fp, "Invalid birth month");
    if (birth_day > 31) output_error(output_fp, "Invalid birth day");

    // Checks for GPA
    float epsilon = 0.0001f;
    float gpa;
    if (sscanf(gpa_str, "%f", &gpa) != 1 || gpa > 4.3f + epsilon || gpa < 0) output_error(output_fp, "Invalid GPA");

    // Checks type
    if (type != 'I' && type != 'D') output_error(output_fp, "Invalid type");

    // Checks TOEFL
    if (type == 'I')
        if (TOEFL_score > 120 || TOEFL_score < 0) output_error(output_fp, "Invalid TOEFL score");

    // Assign the common fields
    if (type == 'I') {
        student.type = INTERNATIONAL;
        student.student.international.first_name = strdup(first_name);
        student.student.international.last_name = strdup(last_name);
        student.student.international.birth_year = birth_year;
        student.student.international.birth_month = strdup(birth_month);
        student.student.international.birth_day = birth_day;
        student.student.international.gpa_str = strdup(gpa_str);
        student.student.international.TOEFL_score = TOEFL_score;
    } else {
        student.type = DOMESTIC;
        student.student.domestic.first_name = strdup(first_name);
        student.student.domestic.last_name = strdup(last_name);
        student.student.domestic.birth_year = birth_year;
        student.student.domestic.birth_month = strdup(birth_month);
        student.student.domestic.birth_day = birth_day;
        student.student.domestic.gpa_str = strdup(gpa_str);
    }

    return student;
}

Student* generate_students_from_lines(char **lines, int line_count, int *student_count, FILE *output_fp) {
    // Allocate memory for students array
    Student *students = malloc(sizeof(Student) * line_count);
    if (students == NULL) {
        perror("Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < line_count; i++) {
        // Parse each line and store in the students array
        students[i] = parse_line(lines[i], output_fp);
        (*student_count)++;
    }

    return students;
}

/*
Outputs domestic (option 1)
*/
void output_domestic(FILE *output_fp, Student *students, int student_count) {
    for (int i = 0; i < student_count; i++) {
        if (students[i].type == DOMESTIC) {
            char *first_name = students[i].student.domestic.first_name;
            char *last_name = students[i].student.domestic.last_name;
            char *birth_month = students[i].student.domestic.birth_month;
            int birth_day = students[i].student.domestic.birth_day;
            int birth_year = students[i].student.domestic.birth_year;
            char *gpa_str = students[i].student.domestic.gpa_str;

            fprintf(output_fp ,"%s %s %s-%d-%d %s D\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str);
        }
    }
}

/*
Outputs international (option 2)
*/
void output_international(FILE *output_fp, Student *students, int student_count) {
    for (int i = 0; i < student_count; i++) {
        if (students[i].type == INTERNATIONAL) {
            char *first_name = students[i].student.international.first_name;
            char *last_name = students[i].student.international.last_name;
            char *birth_month = students[i].student.international.birth_month;
            int birth_day = students[i].student.international.birth_day;
            int birth_year = students[i].student.international.birth_year;
            char *gpa_str = students[i].student.international.gpa_str;
            int TOEFL_score = students[i].student.international.TOEFL_score;

            fprintf(output_fp, "%s %s %s-%d-%d %s I %d\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str, TOEFL_score);
        }
    }
}

/*
Outputs both (option3)
*/
void output_both(FILE *output_fp, Student *students, int student_count) {
    for (int i = 0; i < student_count; i++) {
        if (students[i].type == INTERNATIONAL) {
            char *first_name = students[i].student.international.first_name;
            char *last_name = students[i].student.international.last_name;
            char *birth_month = students[i].student.international.birth_month;
            int birth_day = students[i].student.international.birth_day;
            int birth_year = students[i].student.international.birth_year;
            char *gpa_str = students[i].student.international.gpa_str;
            int TOEFL_score = students[i].student.international.TOEFL_score;

            fprintf(output_fp, "%s %s %s-%d-%d %s I %d\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str, TOEFL_score);
        }
        if (students[i].type == DOMESTIC) {
            char *first_name = students[i].student.domestic.first_name;
            char *last_name = students[i].student.domestic.last_name;
            char *birth_month = students[i].student.domestic.birth_month;
            int birth_day = students[i].student.domestic.birth_day;
            int birth_year = students[i].student.domestic.birth_year;
            char *gpa_str = students[i].student.domestic.gpa_str;

            fprintf(output_fp ,"%s %s %s-%d-%d %s D\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str);
        }
    }
}

/*
Takes student array and sorts it in place
*/
void merge_sort(Student *student, int student_count) {

}

void to_lower_case(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
        i++;
    }
}

/*
Returns 1 if student a is less than student b
Returns -1 if student b is less than student a
Returns 0 if student a is equal to student b
*/
int student_comparator(Student a, Student b) {
    // Initialize values
    int a_birth_year;
    int a_birth_month;
    int a_birth_day;
    char *a_last_name;
    char *a_first_name;
    char *a_gpa;
    int a_TOEFL = -1;

    int b_birth_year;
    int b_birth_month;
    int b_birth_day;
    char *b_last_name;
    char *b_first_name;
    char *b_gpa;
    int b_TOEFL = -1;

    // Set the values
    if (a.type == DOMESTIC) {
        a_birth_year = a.student.domestic.birth_year;
        a_birth_month = month_to_int(a.student.domestic.birth_month);
        a_birth_day = a.student.domestic.birth_day;
        a_last_name = (char *)malloc(strlen(a.student.domestic.last_name) + 1);
        a_first_name = (char *)malloc(strlen(a.student.domestic.first_name) + 1);
        strcpy(a_last_name, a.student.domestic.last_name);
        strcpy(a_first_name, a.student.domestic.first_name);
        a_gpa = a.student.domestic.gpa_str;
    } else {
        a_birth_year = a.student.international.birth_year;
        a_birth_month = month_to_int(a.student.international.birth_month);
        a_birth_day = a.student.international.birth_day;
        a_last_name = (char *)malloc(strlen(a.student.international.last_name) + 1);
        a_first_name = (char *)malloc(strlen(a.student.international.first_name) + 1);
        strcpy(a_last_name, a.student.international.last_name);
        strcpy(a_first_name, a.student.international.first_name);
        a_gpa = a.student.international.gpa_str;
        a_TOEFL = a.student.international.TOEFL_score;
    }
    if (b.type == DOMESTIC) {
        b_birth_year = b.student.domestic.birth_year;
        b_birth_month = month_to_int(b.student.domestic.birth_month);
        b_birth_day = b.student.domestic.birth_day;
        b_last_name = (char *)malloc(strlen(b.student.domestic.last_name) + 1);
        b_first_name = (char *)malloc(strlen(b.student.domestic.first_name) + 1);
        strcpy(b_last_name, b.student.domestic.last_name);
        strcpy(b_first_name, b.student.domestic.first_name);
        b_gpa = b.student.domestic.gpa_str;
    } else {
        b_birth_year = b.student.international.birth_year;
        b_birth_month = month_to_int(b.student.international.birth_month);
        b_birth_day = b.student.international.birth_day;
        b_last_name = (char *)malloc(strlen(b.student.international.last_name) + 1);
        b_first_name = (char *)malloc(strlen(b.student.international.first_name) + 1);
        strcpy(b_last_name, b.student.international.last_name);
        strcpy(b_first_name, b.student.international.first_name);
        b_gpa = b.student.international.gpa_str;
        b_TOEFL = b.student.international.TOEFL_score;
    }

    // Start comparisons
    // Birth year
    if (a_birth_year == b_birth_year) {
        // printf("Birth year is the same.\n");
        // Birth month
        if (a_birth_month == b_birth_month) {
            // printf("Birth month is the same.\n");
            // Birth day
            if (a_birth_day == b_birth_day) {
                // printf("Birth day is the same.\n");
                // Last Name
                to_lower_case(a_last_name);
                to_lower_case(b_last_name);
                if (strcmp(a_last_name, b_last_name) == 0) {
                    // printf("Last name is the same.\n");
                    free(a_last_name);
                    free(b_last_name);

                    // First Name
                    to_lower_case(a_first_name);
                    to_lower_case(b_first_name);
                    if (strcmp(a_first_name, b_first_name) == 0) {
                        free(a_first_name);
                        free(b_first_name);
                        // printf("First name is the same.\n");

                        // GPA
                        double cmp_gpa = atof(a_gpa) - atof(b_gpa);
                        double epsilon = 0.0001F;
                        if (cmp_gpa <= epsilon && cmp_gpa >= (-1 * epsilon)) {
                            // printf("GPA is the same.\n");
                            // Status
                            if (a_TOEFL == -1 && b_TOEFL == -1) {
                                return 0;
                            } else if (a_TOEFL != -1 && b_TOEFL == -1) {
                                return 1;
                            } else if (a_TOEFL == -1 && b_TOEFL != -1) {
                                return -1;
                            } else {
                                // printf("Both students are INTERNATIONAL.\n");
                                // TOEFL
                                if (a_TOEFL == b_TOEFL) {
                                    // printf("TOEFL score is the same.\n");
                                    return 0;
                                } else if (a_TOEFL < b_TOEFL) {
                                    return 1;
                                } else {
                                    return -1;
                                }
                            }
                        } else if (cmp_gpa < (-1 * epsilon)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    } else if (strcmp(a_first_name, b_first_name) < 0) {
                        free(a_first_name);
                        free(b_first_name);
                        return 1;
                    } else {
                        free(a_first_name);
                        free(b_first_name);
                        return -1;
                    }
                } else if (strcmp(a_last_name, b_last_name) < 0) {
                    free(a_last_name);
                    free(a_first_name);
                    free(b_last_name);
                    free(b_first_name);
                    return 1;
                } else {
                    free(a_last_name);
                    free(a_first_name);
                    free(b_last_name);
                    free(b_first_name);
                    return -1;
                }
            } else if (a_birth_day > b_birth_day) {
                return 1;
            } else {
                return -1;
            }
        } else if (a_birth_month > b_birth_month) {
            return 1;
        } else {
            return -1;
        }
    } else if (a_birth_year > b_birth_year) {
        return 1;
    } else {
        return -1;
    }
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

    // Uncomment to test two student inputs
    // int cmp_result = student_comparator(students[0], students[1]);
    // if (cmp_result == 0) {
    //     printf("Both students are equal!\n");
    // } else if (cmp_result == -1) {
    //     printf("First student should come before!\n");
    // } else {
    //     printf("Second student should come before!\n");
    // }
    // return 0;

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