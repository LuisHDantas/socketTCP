#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_SERVER "127.0.0.1"

using namespace std;

int main() {
    int socket_client;
    struct sockaddr_in serv_addr;

    // Criar socket
    //AF_INET: Indica que eh IPv4
    //SOCK_STREAM: Indica conexão confiavel
    //0: usaremos TCP.
    if((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("[CLIENT.CPP]Erro ao criar socket\n");
        exit(EXIT_FAILURE);
    }

    // Definir endereço do servidor
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(PORT);

    // Converter endereço IP
    if(inet_pton(AF_INET, IP_SERVER, &serv_addr.sin_addr) <= 0){
        perror("[CLIENT.CPP] endereço IP inválido\n");
        exit(EXIT_FAILURE);
    }

    // Conectar ao servidor
    if(connect(socket_client, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("[CLIENT.CPP] Conexecao com servidor falhou.\n");
        exit(EXIT_FAILURE);
    }

    cout << "Conectado ao servidor na porta " << PORT << '\n';

    char buffer_server[BUFFER_SIZE] = {0}; //Recebe resposta do servidor
    string message;//envia mensagem para servidor
    // Enviar mensagens para o servidor
    while(true){
        cout << "Digite a mensagem para enviar ao servidor: " << '\n';
        getline(cin, message);
        if(message.size() < BUFFER_SIZE){
            //Fecha conexão
            if(message == "/exit") break;

            //Envia mensagem ao servidor
            send(socket_client, message.c_str(), message.length(), 0);

            //Recebe mensagem do servidor
            recv(socket_client, buffer_server, BUFFER_SIZE, 0);
            cout << "Mensagem do Servidor: "<< buffer_server << '\n';

            //Limpa buffer
            memset(buffer_server, 0, BUFFER_SIZE);
        }else{
            cout << "Mensagem deve ter no máximo 1024 caracteres" << '\n';
        }
    }

    // Fechar a conexão
    close(socket_client);

    return 0;
}
