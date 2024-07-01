#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 8080
#define BUFFER_SIZE 1024

#define VERMELHO "\033[0;31m"
#define VERDE "\033[0;32m"
#define AMARELO "\033[0;33m"
#define AZUL "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CIANO "\033[0;36m"
#define BRANCO "\033[0;37m"

using namespace std;

struct ClientThreadParams {
    int client_socket;
    int contClient;
};

void write_file(int new_socket, char *filename);
void *handle_client(void *arg);

int main(){
    int socket_server, socket_client;
    struct sockaddr_in address_server;
    int addrlen = sizeof(address_server);

    // Criar socket
    //AF_INET: Indica que eh IPv4
    //SOCK_STREAM: Indica conexão confiavel
    //0: usaremos TCP.
    if((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Erro ao criar socket server.\n");
        exit(EXIT_FAILURE);
    }

    // Definir endereço e porta para o socket do servidor
    address_server.sin_family = AF_INET; //IPv4
    address_server.sin_addr.s_addr = INADDR_ANY; //Endereco IP: Localhost 127.0.0.1
    address_server.sin_port = htons(PORT); //Porta

    // Associa o socket a nossa struct address_server
    if(bind(socket_server, (struct sockaddr *)&address_server, sizeof(address_server)) < 0){
        perror("Associação entre socket e address_server falhou.\n");
        close(socket_server);
        exit(EXIT_FAILURE);
    }

    // Colocar o socket em modo de escuta
    // Neste caso aceita até 3 conexões em uma fila
    if(listen(socket_server, 3) < 0){
        perror("listen\n");
        close(socket_server);
        exit(EXIT_FAILURE);
    }

    cout << "Servidor em escuta na porta " << PORT << '\n';

    int contClients = 0;
    // Receber e imprimir mensagens do cliente
    while(true){
        // Aceitar uma conexão com o client
        if((socket_client = accept(socket_server, (struct sockaddr *)&address_server, (socklen_t*)&addrlen)) < 0){
            perror("accept");
            close(socket_server);
            exit(EXIT_FAILURE);
        }

        cout << "Servidor conectado a um cliente\n\n";
        

        pthread_t thread_id;
        int *pclient = (int*) malloc(sizeof(int)); 
        *pclient = socket_client;
        
        contClients++;

        ClientThreadParams *params = new ClientThreadParams;
        params->client_socket = *pclient;
        params->contClient = contClients;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)params) != 0) {
            perror("pthread_create");
            close(socket_client);
            free(pclient);
            delete params;  // Liberar memória da estrutura em caso de erro
        }
        pthread_detach(thread_id);

    }

    // Fechar a conexão
    close(socket_client);
    close(socket_server);

    return 0;
}

void *handle_client(void *arg){
    int socket_client = *(int *) arg;

    ClientThreadParams *params = (ClientThreadParams *)arg;
    int client_socket = params->client_socket;
    int contClient = params->contClient;
    delete params;  // Liberar memória da estrutura

    string cores[6] = {VERMELHO, VERDE, AMARELO, AZUL, MAGENTA, CIANO};

    char buffer_message[BUFFER_SIZE] = {0};

    const string OK_message = "[SERVER] ok.";
    const string OK_file = "[SERVER] Arquivo Salvo.";
    const string CLOSE_message = "[SERVER] close.";
    const string ERROR_message = "[SERVER] erro.";

    int bytes = recv(socket_client, buffer_message, BUFFER_SIZE, 0);
    int posCor = contClient % 6;
    char usernameCor[8+bytes+1];
    strcpy(usernameCor, cores[posCor].c_str());
    char username[bytes+1];
    if(bytes > 0){
        strncpy(username, buffer_message, bytes);
        username[bytes] = '\0';
        strcat(usernameCor, username);
    }
    
    while(true){
        //Limpa buffer
        memset(buffer_message, 0, BUFFER_SIZE);

        int bytes = recv(socket_client, buffer_message, BUFFER_SIZE, 0); //nenhuma flag será usada
        if(bytes > 0){
            //Verifica se esta recebendo um arquivo
            if(strncmp(buffer_message, "/file", 5) == 0){
                // Receber o nome do arquivo
                bytes = recv(socket_client, buffer_message, BUFFER_SIZE, 0);
                if (bytes > 0) {
                    char filename[BUFFER_SIZE];
                    strncpy(filename, buffer_message, bytes);
                    filename[bytes] = '\0';
                    cout << BRANCO << "\n[SERVER] Recebendo arquivo: " << filename << '\n';
                    write_file(socket_client, filename);
                    cout << BRANCO << "Arquivo " << filename << " recebido com sucesso.\n\n";

                    //Envia resposta ao cliente 
                    send(socket_client, OK_file.c_str(), OK_file.length(), 0);
                }
            } else {
                cout << usernameCor << ": " << BRANCO << buffer_message << '\n';
                //Envia resposta ao cliente
                send(socket_client, OK_message.c_str(), OK_message.length(), 0);
            }
        }else if (bytes == 0){
            cout << '\n';
            //Envia resposta de erro ao cliente
            send(socket_client, ERROR_message.c_str(), ERROR_message.length(), 0);
            break;
        }else{
            //Envia resposta de erro ao cliente
            send(socket_client, ERROR_message.c_str(), ERROR_message.length(), 0);
            perror("read");
            break;
        }
    }

    close(socket_client);
    return NULL;
}


void write_file(int new_socket, char *filename){
    int n;
    FILE *fp;
    char buffer[BUFFER_SIZE] = {0};
    char caminho[BUFFER_SIZE] = "serverArqs/";

    strcat(caminho, filename);
    fp = fopen(caminho, "w");
    if(fp == NULL){
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    while((n = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0){
        if (strncmp(buffer, "/endfile", 8) == 0) {
            break;  // Sinal de término do arquivo
        }

        if (fwrite(buffer, sizeof(char), n, fp) != n) {
            perror("Erro ao escrever no arquivo");
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    if(n < 0){
        perror("Erro ao receber o arquivo");
    }

    fclose(fp);
}
