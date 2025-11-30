#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <pthread.h>

#include <cctype>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#define null ((void *)0)

#define CANVAS_W 60
#define CANVAS_H 30

typedef struct {
    pthread_mutex_t mutex;
    char canvas[CANVAS_W * CANVAS_H];
} SharedData;

int runServer() {
    shm_unlink("/lab6_shared-data");
    int fd = shm_open("/lab6_shared-data", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    printf("%d %d\n", fd, errno);

    int result = ftruncate(fd, sizeof(SharedData));
    if(result != 0) {
        printf("ftruncate %d\n", errno);
    }

    void *data = mmap(null, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("%p %d\n", data, errno);

    SharedData *sharedData = (SharedData *)data;

    // NOTE: file descriptor can be safely closed
    // https://man7.org/linux/man-pages/man3/shm_open.3.html
    // close(fd); 

    printf("STARt LOOP\n");

    int time = 0;
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        printf("\e[2J\e[H");

        printf("Time: %d\n", time);
        time += 1;

        for(int y = 0; y < CANVAS_H; y++) {
            printf("[ ");
            for(int x = 0; x < CANVAS_W; x++) {
                int index = x + y * CANVAS_W;
                char c = sharedData->canvas[index];
                if(std::isprint(c)) {
                    printf("%c", c);
                }
                else {
                    printf(" ");
                }
            }
            printf(" ]\n");
        }
        std::cout << std::flush;
    }
}

int runClient() {
    int fd = shm_open("/lab6_shared-data", O_RDWR, S_IRUSR | S_IWUSR);
    printf("%d %d\n", fd, errno);

    int result = ftruncate(fd, sizeof(SharedData));
    if(result != 0) {
        printf("ftruncate %d\n", errno);
    }

    void *data = mmap(null, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("%p %d\n", data, errno);

    SharedData *sharedData = (SharedData *)data;

    std::cout << "Write a message: " << std::flush;
    std::string message;
    std::getline(std::cin, message);

    std::cout << "At what offset to write this message: " << std::flush;
    size_t offset;
    std::cin >> offset;

    if(offset >= CANVAS_W * CANVAS_H) {
        std::cout << "Invalid offset" << std::endl;
        return 1;
    }

    size_t length = message.size();
    if(offset + length > CANVAS_W * CANVAS_H) {
        std::cout << "Your message is too long, it will be truncated" << std::endl;

        length = CANVAS_W * CANVAS_H - offset;
    }

    memcpy(((char *)sharedData->canvas) + offset, message.data(), length);
    return 0;
}

int main(void) {
    std::cout << "Input 's' or 'c'" << std::endl;

    std::string input;
    std::getline(std::cin, input);

    if(false) {}
    else if(input == "s") {
        return runServer();
    }
    else if(input == "c") {
        return runClient();
    }
    else {
        std::cout << "Bad input" << std::endl;
        return 1;
    }
}
