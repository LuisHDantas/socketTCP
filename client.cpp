#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_SERVER "127.0.0.1"

#define BRANCO "\033[0;37m"
#define AZUL "\033[0;34m"

using namespace std;

void send_file(string filename, int socket);
bool extractFileName(const string &input, string &filename);

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

    string username;
    cout << "\nDigite seu nome de Usuário: ";
    getline(cin, username);

    send(socket_client, username.c_str(), strlen(username.c_str()), 0);
    usleep(100000);
    
    username = AZUL+username;

    char buffer_server[BUFFER_SIZE] = {0}; //Recebe resposta do servidor
    string message;//envia mensagem para servidor
    // Enviar mensagens para o servidor
    while(true){
        cout << "\n" << username << ": ";
        cout << BRANCO;
        getline(cin, message);
        if(message.size() < BUFFER_SIZE){
            //Fecha conexão
            if(message == "/exit") break;
            
            string filename;
            if(extractFileName(message, filename)){
                //indica ao servidor que vamos enviar um arquivo
                send(socket_client, "/file", 5, 0);
                usleep(100000);
                send(socket_client, filename.c_str(), strlen(filename.c_str()), 0);
                usleep(100000);
                // Enviar separador para sinalizar o fim do nome do arquivo
                send_file(filename, socket_client);
                usleep(100000);

                // Enviar sinal de término do arquivo
                send(socket_client, "/endfile", 8, 0);

                cout << BRANCO << "[CLIENT] Arquivo " << filename << " enviado com sucesso.\n"; 
            }else{//Envia mensagem ao servidor
                send(socket_client, message.c_str(), message.length(), 0);
            }
            //Recebe mensagem do servidor
            recv(socket_client, buffer_server, BUFFER_SIZE, 0);
            cout << BRANCO << buffer_server << '\n';

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

void send_file(string filename, int socket){
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen(filename.c_str(), "r");

    while(fgets(buffer, BUFFER_SIZE, fp) != NULL){
        if (send(socket, buffer, strlen(buffer), 0) == -1) {
            perror("Erro ao enviar o arquivo.");
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, BUFFER_SIZE);
    } 

    fclose(fp);
}


bool extractFileName(const string& input, string& fileName){
    const string prefix = "/file ";

    // Verifica se a string começa com "/file "
    if(input.substr(0, prefix.size()) == prefix){
        // Extrai o nome do arquivo
        fileName = input.substr(prefix.size());
        return true;
    }
    return false;
}