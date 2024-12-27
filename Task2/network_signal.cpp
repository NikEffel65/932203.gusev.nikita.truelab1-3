#include <iostream>
#include <csignal>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int) {
    wasSigHup = 1;
}

int createListeningSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

int main() {
    const int port = 12345;
    int serverSocket = createListeningSocket(port);

    struct sigaction sa = {};
    sa.sa_handler = sigHupHandler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sa, nullptr);

    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    std::vector<int> clients;
    fd_set readfds;

    std::cout << "Server listening on port " << port << "..." << std::endl;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        int maxFd = serverSocket;

        for (int clientSocket : clients) {
            FD_SET(clientSocket, &readfds);
            if (clientSocket > maxFd) maxFd = clientSocket;
        }

        if (pselect(maxFd + 1, &readfds, nullptr, nullptr, nullptr, &origMask) == -1) {
            if (errno == EINTR) {
                if (wasSigHup) {
                    std::cout << "Received SIGHUP signal." << std::endl;
                    wasSigHup = 0;
                }
                continue;
            } else {
                perror("pselect");
                break;
            }
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket == -1) {
                perror("accept");
                continue;
            }

            if (clients.empty()) {
                std::cout << "New connection accepted." << std::endl;
                clients.push_back(clientSocket);
            } else {
                std::cout << "Connection refused." << std::endl;
                close(clientSocket);
            }
        }

        for (auto it = clients.begin(); it != clients.end();) {
            int clientSocket = *it;
            if (FD_ISSET(clientSocket, &readfds)) {
                char buffer[1024];
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (bytesRead > 0) {
                    std::cout << "Received " << bytesRead << " bytes." << std::endl;
                } else {
                    if (bytesRead == 0 || (bytesRead == -1 && errno != EINTR)) {
                        std::cout << "Connection closed by client." << std::endl;
                        close(clientSocket);
                        it = clients.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
    }

    close(serverSocket);
    for (int clientSocket : clients) {
        close(clientSocket);
    }

    return 0;
}
