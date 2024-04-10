#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

//Левый конец отрезка
#define A M_PI / 4

//Правый конец отрезка
#define B 3 * M_PI / 4

//Количество промежутков
#define K 100

//Количество слагаемых в ряде Тейлора
#define N 10

// Функция для вычисления факториала числа
unsigned long long factorial(unsigned int n) { 
    if (n == 0 || n == 1)
        return 1;
    else
        return n * factorial(n - 1);
}


// Функция для вычисления значения функции sin(x) методом разложения в ряд
double compute_sin(double x) {
    double result = 0;
    int i, sign = 1;
    for (i = 0; i < N; i++) {
        result += sign * pow(x, 2 * i + 1) / factorial(2 * i + 1);
        sign *= -1;
    }
    return result;
}

// Функция для вычисления значения функции cos(x) методом разложения в ряд
double compute_cos(double x) {
    double result = 0;
    int i, sign = 1;
    for (i = 0; i < N; i++) {
        result += sign * pow(x, 2 * i) / factorial(2 * i);
        sign *= -1;
    }
    return result;
}

// Функция для интегрирования методом трапеций
double integrate_trapezoidal(double a, double b, int k) {
    char* filename = "temp_file.txt";

    double h = (b - a) / k;
    double integral_result = 0;

    for (int i = 0; i <= k; i++) {
        double x = a + i * h;
        pid_t pid_sin, pid_cos;

        pid_sin = fork();
        if (pid_sin == 0) {
            // Процесс-потомок для sin(x)
            double result = compute_sin(x);
            FILE *file = fopen(filename, "w");
            fprintf(file, "sin(%lf)=%lf\n", x, result);
            fclose(file);
            exit(0);
        } else if (pid_sin < 0){
            perror("Error in forking sin process");
            exit(EXIT_FAILURE);
        }

        pid_cos = fork();
        if (pid_cos == 0) {
            // Процесс-потомок для cos(x)
            double result = compute_cos(x);
            FILE *file = fopen(filename, "a");
            fprintf(file, "cos(%lf)=%lf\n", x, result);
            fclose(file);
            exit(0);
        } else if (pid_cos < 0){
            perror("Error in forking cos process");
            exit(EXIT_FAILURE);
        }

        // Процесс-отец ожидает завершения процессов-потомков
        wait(NULL);
        wait(NULL);

        // Чтение результатов из временного файла
        double result_sin, result_cos;
        FILE *file = fopen(filename, "r");
        fscanf(file, "sin(%lf)=%lf\n", &x, &result_sin);
        fscanf(file, "cos(%lf)=%lf\n", &x, &result_cos);
        fclose(file);

        // Вычисление интеграла методом трапеций
        //integral_result += 0.5 * (result_sin + result_cos);
	if (i == 0 || i == k) {
            integral_result += 0.5 * (result_sin + result_cos);
        } else {
            integral_result += result_sin + result_cos;
        }
    }

    integral_result *= h;
    return integral_result;
}

int main() 
{
	if (A > B)
	{
	perror("Oshibka");
	exit(EXIT_FAILURE);
	}
    double integral_result = integrate_trapezoidal(A, B, K);
    printf("Integral result: %f\n", integral_result);

    return 0;
}

