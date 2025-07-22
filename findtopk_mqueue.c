#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAX_NUMBERS 1000 // Her dosyadan okunabilecek maksimum sayı
#define MAX_MSG_SIZE 256 // Maksimum mesaj boyutu

// Rastgele sayı üret
int generateRandomNumber() {
    return rand() % 1000 + 1; // 1 ile 1000 arasında rastgele sayı üret
}

// Dosyaya rastgele sayılar yaz ve en büyüğü bul
void writeRandomAndFindMax(const char *filename, int k, mqd_t parent_mq) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Dosya açma hatası");
        exit(EXIT_FAILURE);
    }

    int max = 0;

    srand(time(NULL) ^ getpid());

    for (int i = 0; i < k; i++) {
        int num = generateRandomNumber();
        fprintf(file, "%d\n", num); // Rastgele sayıyı dosyaya yaz
        if (num > max) {
            max = num;
        }
    }

    fclose(file);

    char msg[MAX_MSG_SIZE];
    sprintf(msg, "%d", max);

    if (mq_send(parent_mq, msg, strlen(msg), 0) == -1) {
        perror("mq_send hatası");
        exit(EXIT_FAILURE);
    }
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

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t parent_mq = mq_open("/findtopk_mqueue", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);

    if (parent_mq == (mqd_t)-1) {
        perror("mq_open hatası");
        return EXIT_FAILURE;
    }

    clock_t start, end;
    double cpu_time_used;

    start = clock(); // Başlangıç zamanını kaydet

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork hatası");
            return EXIT_FAILURE;
        } else if (pid == 0) {
            writeRandomAndFindMax(argv[i + 3], k, parent_mq);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    int numbersReceived = 0;
    int maxNumbers[N];

    struct mq_attr attr_rcv;
    mq_getattr(parent_mq, &attr_rcv);
    char rcv_msg[attr_rcv.mq_msgsize];

    while (numbersReceived < N) {
        if (mq_receive(parent_mq, rcv_msg, attr_rcv.mq_msgsize, NULL) == -1) {
            perror("mq_receive hatası");
            return EXIT_FAILURE;
        }
        maxNumbers[numbersReceived++] = atoi(rcv_msg);
    }

    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (maxNumbers[i] < maxNumbers[j]) {
                int temp = maxNumbers[i];
                maxNumbers[i] = maxNumbers[j];
                maxNumbers[j] = temp;
            }
        }
    }

    FILE *outFile = fopen(argv[argc - 1], "w");
    if (outFile == NULL) {
        perror("Output dosyası açma hatası");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < N; i++) {
        fprintf(outFile, "%d\n", maxNumbers[i]);
    }

    fclose(outFile);

    if (mq_close(parent_mq) == -1) {
        perror("mq_close hatası");
        return EXIT_FAILURE;
    }

    if (mq_unlink("/findtopk_mqueue") == -1) {
        perror("mq_unlink hatası");
        return EXIT_FAILURE;
    }

    end = clock(); // Bitiş zamanını kaydet
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC; // Toplam süreyi hesapla

    printf("Geçen süre: %f saniye\n", cpu_time_used);

    return EXIT_SUCCESS;
}
