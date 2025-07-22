#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_NUMBERS 1000
#define MAX_FILES 5

typedef struct {
    const char *filename;
    int k;
} ThreadArgs;

// Rastgele sayı üret
int generateRandomNumber() {
    return rand() % 1000 + 1;
}

// Girdi dosyasına farklı rastgele sayılar yaz
void writeDifferentRandomNumbers(const char *filename, int k) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Dosya açma hatası");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) + rand()); // Seed'i güncelle

    for (int i = 0; i < k; i++) {
        int num = generateRandomNumber();
        fprintf(file, "%d\n", num);
    }

    fclose(file);
}

// Her dosya için en büyük sayıyı bul
void *findMax(void *args) {
    ThreadArgs *thread_args = (ThreadArgs *)args;
    FILE *file = fopen(thread_args->filename, "r");
    if (file == NULL) {
        perror("Dosya açma hatası");
        exit(EXIT_FAILURE);
    }

    int max = 0;
    int num;

    while (fscanf(file, "%d", &num) == 1) {
        if (num > max) {
            max = num;
        }
    }

    fclose(file);

    int *result = malloc(sizeof(int));
    *result = max;

    pthread_exit(result);
}

int main(int argc, char *argv[]) {

    clock_t start, end;
    double cpu_time_used;

    start = clock(); // Başlangıç zamanını kaydet

    if (argc < 5 || argc > MAX_FILES + 4) {
        printf("Kullanım: %s <k> <N> <infile1> ...<infileN> <outfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int k = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (N > MAX_FILES || N < 1 || k < 1 || k > MAX_NUMBERS) {
        printf("Hata: Geçersiz girişler!\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    // Her girdi dosyasına farklı rastgele sayılar yaz
    for (int i = 0; i < N; i++) {
        writeDifferentRandomNumbers(argv[i + 3], k);
    }

    pthread_t threads[N];
    ThreadArgs thread_args[N];

    // Her dosya için thread oluştur
    for (int i = 0; i < N; i++) {
        thread_args[i].filename = argv[i + 3];
        thread_args[i].k = k;

        if (pthread_create(&threads[i], NULL, findMax, (void *)&thread_args[i]) != 0) {
            perror("Thread oluşturma hatası");
            return EXIT_FAILURE;
        }
    }

    int max_results[N];

    // Thread'lerin bitmesini bekle ve en büyük sayıları al
    for (int i = 0; i < N; i++) {
        int *result;
        if (pthread_join(threads[i], (void **)&result) != 0) {
            perror("Thread birleştirme hatası");
            return EXIT_FAILURE;
        }
        max_results[i] = *result;
        free(result);
    }

    // En büyük sayıları çıkış dosyasına yaz
    FILE *outfile = fopen(argv[argc - 1], "w");
    if (outfile == NULL) {
        perror("Output dosyası açma hatası");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < N; i++) {
        fprintf(outfile, "%d\n", max_results[i]);
    }

    fclose(outfile);

    end = clock(); // Bitiş zamanını kaydet

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // Toplam süreyi hesapla
    printf("Geçen süre: %f saniye\n", cpu_time_used);

    return EXIT_SUCCESS;
}
