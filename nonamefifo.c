#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <wait.h>

int main() {
    int pipe_fd[2]; // Массив для хранения дескрипторов канала
    int secret_number, guess;
    int low, high; // Диапазон для угадывания
    int rounds = 10; // Количество итераций
    int current_round = 0; // Счетчик итераций

    // Создаем неименованный канал
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    while (current_round < rounds) {
        // Код для игрока 1 (родительский процесс)
        if (fork() == 0) {
            // Дочерний процесс (Игрок 1)
            close(pipe_fd[0]); // Закрываем чтение в канале

            // Генерируем случайное число от 1 до 100
            srand(time(NULL) + current_round); // Инициализация генератора с учетом текущего раунда
            secret_number = rand() % 100 + 1;

            // Записываем загаданное число в канал
            write(pipe_fd[1], &secret_number, sizeof(secret_number));
            printf("Игрок 1 загадал число: %d\n", secret_number);
            close(pipe_fd[1]); // Закрываем запись в канале
            exit(EXIT_SUCCESS);
        }

        wait(NULL); // Ждем завершения первого дочернего процесса

        // Код для игрока 2 (родительский процесс)
        if (fork() == 0) {
            // Второй дочерний процесс (Игрок 2)
            close(pipe_fd[1]); // Закрываем запись в канале

            // Читаем загаданное число от игрока 1
            read(pipe_fd[0], &secret_number, sizeof(secret_number));
            printf("Игрок 1 загадал число. Игрок 2, начинай угадывать!\n");

            low = 1; // Сброс нижней границы
            high = 100; // Сброс верхней границы

            // Автоматическое угадывание с использованием бинарного поиска
            int attempts = 0; // Счетчик попыток для текущего раунда
            while (1) {
                guess = low + (high - low) / 2; // Угадываем среднее значение
                printf("Игрок 2 пытается угадать: %d\n", guess);
                attempts++;

                if (guess == secret_number) {
                    printf(" Игрок 2 угадал число %d правильно за %d попыток!\n", guess, attempts);
                    break; // Выход из внутреннего цикла, если угадали
                } else if (guess < secret_number) {
                    printf(" Игрок 2 угадал слишком маленькое число.\n");
                    low = guess + 1; // Увеличиваем нижнюю границу
                } else {
                    printf(" Игрок 2 угадал слишком большое число.\n");
                    high = guess - 1; // Уменьшаем верхнюю границу
                }
                
                if (low > high) {
                    printf(" Игрок 2 не может угадать число. Проверьте диапазон.\n");
                    break; // Завершаем игру, если диапазон невалиден
                }
            }
            
            close(pipe_fd[0]); // Закрываем чтение в канале
            exit(EXIT_SUCCESS);
        }

        wait(NULL); // Ждем завершения второго дочернего процесса

        current_round++; // Увеличиваем счетчик раундов

        if (current_round >= rounds) break;

        printf("\n--- Раунд %d завершен! Теперь игроки меняются ролями. ---\n", current_round);

        pipe(pipe_fd); // Создаем новый канал для следующего раунда

        if (fork() == 0) {
            close(pipe_fd[0]); 
            
            srand(time(NULL) + current_round); 
            secret_number = rand() % 100 + 1;
            
            write(pipe_fd[1], &secret_number, sizeof(secret_number));
            printf("Игрок 2 загадал число: %d\n", secret_number);
            
            close(pipe_fd[1]);
            exit(EXIT_SUCCESS);
        }

        wait(NULL); 

        if (fork() == 0) {
            close(pipe_fd[1]);

            read(pipe_fd[0], &secret_number, sizeof(secret_number));
            printf("Игрок 2 загадал число. Игрок 1, начинай угадывать!\n");

            low = 1;
            high = 100;

            int attempts = 0;
            while (1) {
                guess = low + (high - low) / 2;
                printf("Игрок 1 пытается угадать: %d\n", guess);
                attempts++;

                if (guess == secret_number) {
                    printf(" Игрок 1 угадал число %d правильно за %d попыток!\n", guess, attempts);
                    break;
                } else if (guess < secret_number) {
                    printf(" Игрок 1 угадал слишком маленькое число.\n");
                    low = guess + 1;
                } else {
                    printf(" Игрок 1 угадал слишком большое число.\n");
                    high = guess - 1;
                }

                if (low > high) {
                    printf(" Игрок 1 не может угадать число. Проверьте диапазон.\n");
                    break;
                }
            }
            
            close(pipe_fd[0]);
            exit(EXIT_SUCCESS);
        }

        wait(NULL); 
    }

    printf(" Игра завершена! Игроки сыграли %d раундов.\n", rounds);

    return EXIT_SUCCESS;
}
