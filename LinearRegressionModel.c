#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 30

typedef struct {
    double value;
    int frequency;
} FrequencyPair;
double calculate_mean(double data[], int size) {
    double sum = 0.0;
    for(int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}
int compare_doubles(const void *a, const void *b) {
    double diff = *(double*)a - *(double*)b;
    return (diff > 0) - (diff < 0);
}
double calculate_median(double data[], int size) {
    double sorted[size];
    for(int i = 0; i < size; i++) {
        sorted[i] = data[i];
    }
    qsort(sorted, size, sizeof(double), compare_doubles);

    if(size % 2 == 0) {
        return (sorted[size/2 - 1] + sorted[size/2]) / 2.0;
    } else {
        return sorted[size/2];
    }
}
double calculate_mode(double data[], int size) {
    double sorted[size];
    for(int i = 0; i < size; i++) {
        sorted[i] = data[i];
    }
    qsort(sorted, size, sizeof(double), compare_doubles);
    FrequencyPair freq[size];
    int freq_count = 0;

    int current_freq = 1;
    for(int i = 1; i < size; i++) {
        if(sorted[i] == sorted[i-1]) {
            current_freq++;
        } else {
            freq[freq_count].value = sorted[i-1];
            freq[freq_count].frequency = current_freq;
            freq_count++;
            current_freq = 1;
        }
    }
    freq[freq_count].value = sorted[size-1];
    freq[freq_count].frequency = current_freq;
    freq_count++;
    int max_freq = 0;
    double mode = sorted[0];
    for(int i = 0; i < freq_count; i++) {
        if(freq[i].frequency > max_freq) {
            max_freq = freq[i].frequency;
            mode = freq[i].value;
        }
    }

    return mode;
}
double calculate_sd(double data[], int size) {
    double mean = calculate_mean(data, size);
    double sum_squared_diff = 0.0;

    for(int i = 0; i < size; i++) {
        sum_squared_diff += pow(data[i] - mean, 2);
    }

    return sqrt(sum_squared_diff / size);
}
void linear_regression(double x[], double y[], int size, double *slope, double *intercept) {
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;

    for(int i = 0; i < size; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_xx += x[i] * x[i];
    }

    *slope = (size * sum_xy - sum_x * sum_y) / (size * sum_xx - sum_x * sum_x);
    *intercept = (sum_y - *slope * sum_x) / size;
}
void confidence_interval(double data[], int size, double population_sd, double *lower, double *upper) {
    double mean = calculate_mean(data, size);
    double margin = 1.96 * (population_sd / sqrt(size));
    *lower = mean - margin;
    *upper = mean + margin;
}
void create_plot(double dates[], double nav[], int size, double slope, double intercept) {
    FILE *python = popen("python", "w");
    if(python == NULL) {
        printf("Failed to open Python pipe\n");
        return;
    }

    fprintf(python, "import matplotlib.pyplot as plt\n");
    fprintf(python, "import numpy as np\n");
    fprintf(python, "dates = [");
    for(int i = 0; i < size; i++) {
        fprintf(python, "%f,", dates[i]);
    }
    fprintf(python, "]\n");

    fprintf(python, "nav = [");
    for(int i = 0; i < size; i++) {
        fprintf(python, "%f,", nav[i]);
    }
    fprintf(python, "]\n");
    fprintf(python,
        "plt.figure(figsize=(12,8))\n"
        "plt.scatter(dates, nav, color='blue', label='Actual NAV')\n"
        "x = np.linspace(min(dates), max(dates), 100)\n"
        "y = %f * x + %f\n"
        "plt.plot(x, y, color='red', label='Best Fit Line')\n"
        "plt.xlabel('Date')\n"
        "plt.ylabel('NAV Value')\n"
        "plt.title('SBI NAV Values with Best Fit Line')\n"
        "plt.legend()\n"
        "plt.grid(True)\n"
        "plt.show()\n",
        slope, intercept
    );

    fflush(python);
    pclose(python);
}

int main() {
    FILE *navf;
    char line[100];
    double nav[SIZE];
    double dates[SIZE];
    int count = 0;
    navf = fopen("NavData.csv", "r");
    if(navf == NULL) {
        perror("Error opening the file");
        return EXIT_FAILURE;
    }
    while (fgets(line, sizeof(line), navf) != NULL && count < SIZE) {
        char *endptr;
        double value = strtod(line, &endptr);
        if (endptr != line) {
            nav[count] = value;
            dates[count] = count + 1;
            count++;
        }
    }
    fclose(navf);
    if(count != SIZE) {
        printf("Error: Expected %d values but got %d values from the file.\n", SIZE, count);
        return EXIT_FAILURE;
    }
    double mean = calculate_mean(nav, SIZE);
    double median = calculate_median(nav, SIZE);
    double mode = calculate_mode(nav, SIZE);
    double sd = calculate_sd(nav, SIZE);
    double slope, intercept;
    linear_regression(dates, nav, SIZE, &slope, &intercept);
    double lower_ci, upper_ci;
    confidence_interval(nav, SIZE, 2.3, &lower_ci, &upper_ci);
    double estimated_nav = slope * 30 + intercept;
    printf("Statistical Analysis of SBI NAV Values:\n");
    printf("Mean: %.2f\n", mean);
    printf("Median: %.2f\n", median);
    printf("Mode: %.2f\n", mode);
    printf("Standard Deviation: %.2f\n", sd);
    printf("\n95%% Confidence Interval: (%.2f, %.2f)\n", lower_ci, upper_ci);
    printf("\nEstimated NAV for November 25: %.2f\n", estimated_nav);
    create_plot(dates, nav, SIZE, slope, intercept);

    return 0;
}
