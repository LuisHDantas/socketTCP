#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

void write_file(int new_socket, char *filename);

int main(){
    int socket_server, socket_client;
    struct sockaddr_in address_server;
    int addrlen = sizeof(address_server);
    char buffer_message[BUFFER_SIZE] = {0};

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

    // Aceitar uma conexão com o client
    if((socket_client = accept(socket_server, (struct sockaddr *)&address_server, (socklen_t*)&addrlen)) < 0){
        perror("accept");
        close(socket_server);
        exit(EXIT_FAILURE);
    }

    cout << "Servidor conectado a um cliente\n\n";

    const string OK_message = "[SERVER] ok.";
    const string OK_file = "[SERVER] Arquivo Salvo.";
    const string CLOSE_message = "[SERVER] close.";
    const string ERROR_message = "[SERVER] erro.";
    // Receber e imprimir mensagens do cliente
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
                    cout << "\n[SERVER] Recebendo arquivo: " << filename << '\n';
                    write_file(socket_client, filename);
                    cout << "Arquivo " << filename << " recebido com sucesso.\n\n";

                    //Envia resposta ao cliente 
                    send(socket_client, OK_file.c_str(), OK_file.length(), 0);
                }
            } else {
                cout << "Client: " << buffer_message << '\n';
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

    // Fechar a conexão
    close(socket_client);
    close(socket_server);

    return 0;
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
