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
    char *birth_year;
    char *birth_month;
    char *birth_day;
    char *gpa_str;
} DomesticStudent;

typedef struct {
    char *first_name;
    char *last_name;
    char *birth_year;
    char *birth_month;
    char *birth_day;
    char *gpa_str;
    char *TOEFL_score;
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

        if (line_size <= 2) return lines;

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
        size_t endline = strcspn(line, "\n");
        line[endline] = '\0';

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
    char months[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    for (int i = 0; i < 12; i++) {
        if (strcmp(months[i], month) == 0) {
            return i + 1;  // Arrays are 0-based, months are 1-based
        }
    }

    return -1;
}

int days_per_month(int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    return days[month - 1];
}

/*
Pass in the output fp and a string literal to be written to output file
*/
void output_error(FILE *output_fp, char error[]) {

    fprintf(output_fp, "ERROR: %s\n", error);
    exit(EXIT_FAILURE);
}

/*
Checks if the date exists in the real calendar, including leap year considerations
Assumes month, day [1-31], and year are valid on their own
*/
int valid_date(char *month, int day, int year) {
    int month_num = month_to_int(month);
    int day_amount = days_per_month(month_num);
    if (month_num == 2 && year % 4 == 0) return day <= 29; // Check leap year
    if (day > day_amount) return 0;
    return 1;
}

/*
Takes lines and returns a pointer to a student array (unsorted)
Also takes output fp to handle errors by calling output_error()
*/
Student parse_line(char *line, FILE *output_fp) {
    Student student;

    char *first_name;
    char *last_name;
    char *gpa_str;
    char *date;
    char *type;

    char *TOEFL_score_str;
    int TOEFL_score;
    
    char *month;
    char *day_str;
    char *year_str;
    int day;
    int year;

    char delimiter[] = " ";

    first_name = strtok(line, delimiter); // Handles first_name
    if (!first_name) output_error(output_fp, "Invalid first name");

    last_name = strtok(NULL, delimiter); // Handles last_name
    if (!last_name) output_error(output_fp, "Invalid last name");

    char date_delim[] = "-";
    date = strtok(NULL, delimiter);
    if (date) {
        for (int i = 0; i < strlen(date); i++)
            if (date[i] == '.') output_error(output_fp, "Date cannot contain a float");
            
        month = strtok_r(date, date_delim, &date); // Handles month
        if (!month || month_to_int(month) == -1) output_error(output_fp, "Invalid month");

        day_str = strtok_r(date, date_delim, &date); // Handles day
        if (day_str) {
            day = atoi(day_str);
            if (day > 31 || day < 1) output_error(output_fp, "Invalid day");
        } else output_error(output_fp, "Invalid day");

        year_str = strtok_r(date, date_delim, &date); // Handles year
        if (year_str) {
            year = atoi(year_str);
            if (year < 1950 || year > 2010) output_error(output_fp, "Year must be between 1950 and 2010 (inclusive)");
        } else output_error(output_fp, "Invalid year");

        // Checks if date is valid
        if (!valid_date(month, day, year)) output_error(output_fp, "Invalid date");
        
    } else output_error(output_fp, "Invalid date");

    gpa_str = strtok(NULL, delimiter); // Handles gpa
    if (gpa_str) {
        int dec_count = 0;
        for (int i = 0; i < strlen(gpa_str); i++) {
            if (gpa_str[i] != '.' && (gpa_str[i] < '0' || gpa_str[i] > '9')) output_error(output_fp, "GPA must be a float");
            if (gpa_str[i] == '.') dec_count++;
            if (dec_count > 1) output_error(output_fp, "Invalid GPA");
        }
        char *dec_place = strchr(gpa_str, '.');
        if (dec_count > 0 && strlen(gpa_str) - (int) (dec_place - gpa_str) - 1 > 3) output_error(output_fp, "Too many decimal places in GPA");
        float gpa = atof(gpa_str);
        float epsilon = 0.0001f;
        if (gpa > 4.3f + epsilon || gpa < 0.0f) output_error(output_fp, "GPA must be between 0.0 and 4.3");
    } else output_error(output_fp, "Invalid GPA");

    type = strtok(NULL, delimiter); // Handles type
    if (type) {
        if (strcmp(type, "I") && strcmp(type, "D")) output_error(output_fp, "Invalid type");
    } else output_error(output_fp, "Invalid type");

    TOEFL_score_str = strtok(NULL, delimiter); // Handles TOEFL
    if (TOEFL_score_str) {
        if (strcmp(type, "I") == 0) {
            int i = 0;
            while (TOEFL_score_str[i] != '\0') {
                if (TOEFL_score_str[i] < '0' || TOEFL_score_str[i] > '9') output_error(output_fp, "TOEFL must be an integer");
                i++;
            }
            TOEFL_score = atoi(TOEFL_score_str);
            if (TOEFL_score > 120 || TOEFL_score < 0) output_error(output_fp, "TOEFL must be an int between 0 and 120");
        } else output_error(output_fp, "Domestic students cannot have a TOEFL");
    } else if (strcmp(type, "I") == 0) output_error(output_fp, "Missing TOEFL");

    // Generates student
    if (strcmp(type, "I") == 0) {
        // Generate international student
        student.type = INTERNATIONAL;
        student.student.international.first_name = strdup(first_name);
        student.student.international.last_name = strdup(last_name);
        student.student.international.birth_year = strdup(year_str);
        student.student.international.birth_month = strdup(month);
        student.student.international.birth_day = strdup(day_str);
        student.student.international.gpa_str = strdup(gpa_str);
        student.student.international.TOEFL_score = strdup(TOEFL_score_str);
    } else {
        // Generate domestic student
        student.type = DOMESTIC;
        student.student.domestic.first_name = strdup(first_name);
        student.student.domestic.last_name = strdup(last_name);
        student.student.domestic.birth_year = strdup(year_str);
        student.student.domestic.birth_month = strdup(month);
        student.student.domestic.birth_day = strdup(day_str);
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
            char *birth_day = students[i].student.domestic.birth_day;
            char *birth_year = students[i].student.domestic.birth_year;
            char *gpa_str = students[i].student.domestic.gpa_str;

            fprintf(output_fp ,"%s %s %s-%s-%s %s D\n", first_name, last_name,
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
            char *birth_day = students[i].student.international.birth_day;
            char *birth_year = students[i].student.international.birth_year;
            char *gpa_str = students[i].student.international.gpa_str;
            char *TOEFL_score = students[i].student.international.TOEFL_score;

            fprintf(output_fp, "%s %s %s-%s-%s %s I %s\n", first_name, last_name,
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
            char *birth_day = students[i].student.international.birth_day;
            char *birth_year = students[i].student.international.birth_year;
            char *gpa_str = students[i].student.international.gpa_str;
            char *TOEFL_score = students[i].student.international.TOEFL_score;

            fprintf(output_fp, "%s %s %s-%s-%s %s I %s\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str, TOEFL_score);
        }
        if (students[i].type == DOMESTIC) {
            char *first_name = students[i].student.domestic.first_name;
            char *last_name = students[i].student.domestic.last_name;
            char *birth_month = students[i].student.domestic.birth_month;
            char *birth_day = students[i].student.domestic.birth_day;
            char *birth_year = students[i].student.domestic.birth_year;
            char *gpa_str = students[i].student.domestic.gpa_str;

            fprintf(output_fp ,"%s %s %s-%s-%s %s D\n", first_name, last_name,
                birth_month, birth_day, birth_year, gpa_str);
        }
    }
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
        a_birth_year = atoi(a.student.domestic.birth_year);
        a_birth_month = month_to_int(a.student.domestic.birth_month);
        a_birth_day = atoi(a.student.domestic.birth_day);
        a_last_name = (char *)malloc(strlen(a.student.domestic.last_name) + 1);
        a_first_name = (char *)malloc(strlen(a.student.domestic.first_name) + 1);
        strcpy(a_last_name, a.student.domestic.last_name);
        strcpy(a_first_name, a.student.domestic.first_name);
        a_gpa = a.student.domestic.gpa_str;
    } else {
        a_birth_year = atoi(a.student.international.birth_year);
        a_birth_month = month_to_int(a.student.international.birth_month);
        a_birth_day = atoi(a.student.international.birth_day);
        a_last_name = (char *)malloc(strlen(a.student.international.last_name) + 1);
        a_first_name = (char *)malloc(strlen(a.student.international.first_name) + 1);
        strcpy(a_last_name, a.student.international.last_name);
        strcpy(a_first_name, a.student.international.first_name);
        a_gpa = a.student.international.gpa_str;
        a_TOEFL = atoi(a.student.international.TOEFL_score);
    }
    if (b.type == DOMESTIC) {
        b_birth_year = atoi(b.student.domestic.birth_year);
        b_birth_month = month_to_int(b.student.domestic.birth_month);
        b_birth_day = atoi(b.student.domestic.birth_day);
        b_last_name = (char *)malloc(strlen(b.student.domestic.last_name) + 1);
        b_first_name = (char *)malloc(strlen(b.student.domestic.first_name) + 1);
        strcpy(b_last_name, b.student.domestic.last_name);
        strcpy(b_first_name, b.student.domestic.first_name);
        b_gpa = b.student.domestic.gpa_str;
    } else {
        b_birth_year = atoi(b.student.international.birth_year);
        b_birth_month = month_to_int(b.student.international.birth_month);
        b_birth_day = atoi(b.student.international.birth_day);
        b_last_name = (char *)malloc(strlen(b.student.international.last_name) + 1);
        b_first_name = (char *)malloc(strlen(b.student.international.first_name) + 1);
        strcpy(b_last_name, b.student.international.last_name);
        strcpy(b_first_name, b.student.international.first_name);
        b_gpa = b.student.international.gpa_str;
        b_TOEFL = atoi(b.student.international.TOEFL_score);
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
                                } else if (a_TOEFL > b_TOEFL) {
                                    return 1;
                                } else {
                                    return -1;
                                }
                            }
                        } else if (cmp_gpa > (-1 * epsilon)) {
                            return 1;
                        } else {
                            return -1;
                        }
                    } else if (strcmp(a_first_name, b_first_name) > 0) {
                        free(a_first_name);
                        free(b_first_name);
                        return 1;
                    } else {
                        free(a_first_name);
                        free(b_first_name);
                        return -1;
                    }
                } else if (strcmp(a_last_name, b_last_name) > 0) {
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

/*
Takes student array and sorts it in place
*/
void merge(Student *student, int start, int mid, int end) {
    int n1 = mid - start + 1;
    int n2 = end - mid;

    Student *left = (Student*)malloc(n1 * sizeof(Student));
    Student *right = (Student*)malloc(n2 * sizeof(Student));

    for (int i = 0; i < n1; i++) {
        left[i] = student[start + i];
    }
    for (int j = 0; j < n2; j++) {
        right[j] = student[mid + 1 + j];
    }

    int i = 0, j = 0, k = start;
    while (i < n1 && j < n2) {
        if (student_comparator(left[i], right[j]) <= 0) {
            student[k++] = left[i++];
        } else {
            student[k++] = right[j++];
        }
    }

    while (i < n1) {
        student[k++] = left[i++];
    }

    while (j < n2) {
        student[k++] = right[j++];
    }

    free(left);
    free(right);
}

void merge_sort(Student *student, int start, int end) {
    if (start < end) {
        int mid = start + (end - start) / 2;
        merge_sort(student, start, mid);
        merge_sort(student, mid + 1, end);
        merge(student, start, mid, end);
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
    if (output_fp == NULL) {
        perror("Error opening output file\n");
        return EXIT_FAILURE;
    }
    if (input_fp == NULL) {
        output_error(output_fp, "Cannot open input file");
    }

    // Checks if empty input file and assigns size
    if (NULL != input_fp) {
        fseek (input_fp, 0, SEEK_END);
        size = ftell(input_fp);
        if (0 == size) {
            output_error(output_fp, "Empty input file");
        }
        rewind(input_fp);
    }

    int line_count = 0;
    char **lines = read_lines(input_fp, size, &line_count);
    int student_count = 0;
    Student *students = generate_students_from_lines(lines, line_count, &student_count, output_fp);
    merge_sort(students, 0, student_count - 1);

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
        Student student = students[i];
        if (student.type == DOMESTIC) {
            free(student.student.domestic.first_name);
            free(student.student.domestic.last_name);
            free(student.student.domestic.birth_year);
            free(student.student.domestic.birth_month);
            free(student.student.domestic.birth_day);
            free(student.student.domestic.gpa_str);
        } else {
            free(student.student.international.first_name);
            free(student.student.international.last_name);
            free(student.student.international.birth_year);
            free(student.student.international.birth_month);
            free(student.student.international.birth_day);
            free(student.student.international.gpa_str);
            free(student.student.international.TOEFL_score);
        }
    }
    free(lines);
    free(students);
    fclose(input_fp);
    fclose(output_fp);
    return 0;
}