#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_NUMBERS 1000 // Her dosyadan okunabilecek maksimum sayı

// Rastgele sayı üret
int generateRandomNumber() {
    return rand() % 1000 + 1; // 1 ile 1000 arasında rastgele sayı üret
}

// Dosyaya rastgele sayılar yaz ve en büyüğü bul
void writeRandomAndFindMax(const char *filename, int k) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Dosya açma hatası");
        exit(EXIT_FAILURE);
    }

    int max = 0;

    srand(time(NULL) ^ getpid()); // Rastgele sayılar için seed ayarla (process ID'ye göre farklılık)

    // Rastgele sayılar üret, dosyaya yaz ve en büyüğü bul
    for (int i = 0; i < k; i++) {
        int num = generateRandomNumber();
        fprintf(file, "%d\n", num);
        if (num > max) {
            max = num;
        }
    }

    fclose(file);

    // En büyük değeri intermediate.txt dosyasına ekle
    file = fopen("intermediate.txt", "a");
    if (file == NULL) {
        perror("Ara dosya açma hatası");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d\n", max);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Kullanım: %s <k> <N> <infile1> ...<infileN> <outfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int k = atoi(argv[1]); // k değeri
    int N = atoi(argv[2]); // N değeri


    if (N > 5) {
        printf("Hata: N değeri 5'ten büyük olamaz!\n");
        return EXIT_FAILURE;
    }

    if (N != argc - 4) {
        printf("Hata: N değeri dosya sayısıyla uyumlu değil!\n");
        return EXIT_FAILURE;
    }

    if (k > 1000) {
        printf("Hata: k değeri 1000'den büyük olamaz!\n");
        return EXIT_FAILURE;
    }

    clock_t start, end;
    double cpu_time_used;

    start = clock(); // Başlangıç zamanını kaydet

    // N adet child process oluşturma
    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork hatası");
            return EXIT_FAILURE;
        } else if (pid == 0) { // Child process'in işlemleri
            writeRandomAndFindMax(argv[i + 3], k); // Dosyaya rastgele sayılar yaz ve en büyüğü bul
            exit(EXIT_SUCCESS);
        }
    }

    // Tüm child processlerin bitmesini bekle
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    // Intermediate dosyaları okuyup ana process'in işlemleri
    FILE *outFile = fopen(argv[argc - 1], "w");
    if (outFile == NULL) {
        perror("Output dosyası açma hatası");
        return EXIT_FAILURE;
    }

    FILE *intermediateFile = fopen("intermediate.txt", "r");
    if (intermediateFile == NULL) {
        perror("Ara dosya açma hatası");
        return EXIT_FAILURE;
    }

    int num;
    int numbersWritten = 0;

    // Ara dosyadan en büyük sayıları oku ve output dosyasına yaz
    while (numbersWritten < N && fscanf(intermediateFile, "%d", &num) == 1) {
        fprintf(outFile, "%d\n", num);
        numbersWritten++;
    }

    fclose(intermediateFile);
    fclose(outFile);

    // Ara dosyayı sil
    if (remove("intermediate.txt") != 0) {
        perror("Ara dosya silme hatası");
        return EXIT_FAILURE;
    }


    end = clock(); // Bitiş zamanını kaydet

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // Toplam süreyi hesapla
    printf("Geçen süre: %f saniye\n", cpu_time_used);

    return EXIT_SUCCESS;
}
