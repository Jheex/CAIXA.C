#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale.h>
#include <windows.h>  // Inclui a função Sleep()

#define ARQUIVO_PRODUTOS "Data/produtos.txt"
#define PORT 9090
#define BUFFER_SIZE 1024
#define BACKLOG 3

typedef struct {
    char codigo[3];
    char nome[50];
    float peso;
    char tipo[10];
} Produto;

Produto produtos[100]; // Array para armazenar os produtos
int numProdutos = 0; // Contador de produtos


void printErrorAndExit(const char* msg) {
    printf("%s CÓDIGO DE ERRO: %d\n", msg, WSAGetLastError());
    WSACleanup();
    exit(1);
}

SOCKET createServerSocket() {
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printErrorAndExit("FALHA AO CRIAR O SOCKET.");
    }
    return server_socket;
}

void bindSocket(SOCKET server_socket, struct sockaddr_in* server_addr) {
    if (bind(server_socket, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
        printErrorAndExit("FALHA AO ASSUMIR O ENDEREÇO.");
    }
}

void listenForConnections(SOCKET server_socket) {
    if (listen(server_socket, BACKLOG) < 0) {
        printErrorAndExit("FALHA AO ESCUTAR POR CONEXÕES.");
    }
}

void handleClient(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    printf("Cliente conectado.\n");

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Adiciona o terminador de string
        printf("Mensagem recebida do cliente: %s\n", buffer);
        // Implementar a lógica de processamento aqui, se necessário
    }

    if (bytes_received == SOCKET_ERROR) {
        printf("Erro ao receber dados do cliente. Código de erro: %d\n", WSAGetLastError());
    }

    printf("Cliente desconectado.\n");
    closesocket(client_socket);
}

void iniciarServidor() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Inicializa a biblioteca Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printErrorAndExit("FALHA AO INICIALIZAR WINSOCK.");
    }

    // Cria e configura o socket do servidor
    server_socket = createServerSocket();

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bindSocket(server_socket, &server_addr);
    listenForConnections(server_socket);

    printf("SERVIDOR INICIADO. Aguardando conexões na porta %d...\n", PORT);

    // Aceita conexões de clientes
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Erro ao aceitar conexão. Código de erro: %d\n", WSAGetLastError());
            continue; // Tenta aceitar outro cliente, mesmo após erro
        }
        handleClient(client_socket);
    }

    // Fecha o socket do servidor e limpa a biblioteca Winsock
    closesocket(server_socket);
    WSACleanup();
}


// Função para carregar produtos do arquivo
void carregarProdutos() {
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "r");
    if (arquivo) {
        while (fscanf(arquivo, "%[^;];%[^;];%f;%[^;];\n", produtos[numProdutos].codigo, produtos[numProdutos].nome,
                      &produtos[numProdutos].peso, produtos[numProdutos].tipo) != EOF) {
            numProdutos++;
        }
        fclose(arquivo);
    }
}

// Função para listar produtos
void listarProdutos() {
    printf("LISTA DE PRODUTOS:\n");
    for (int i = 0; i < numProdutos; i++) {
        printf("%s;%s;%.2f;%s;\n", produtos[i].codigo, produtos[i].nome, produtos[i].peso, produtos[i].tipo);
    }
}

// Função para verificar se o produto já está cadastrado
int produtoJaCadastrado(const char *nome) {
    for (int i = 0; i < numProdutos; i++) {
        if (strcmp(produtos[i].nome, nome) == 0) {
            return 1; // Produto já cadastrado
        }
    }
    return 0; // Produto não cadastrado
}

// Função para cadastrar um produto
void cadastrarProduto() {
    if (numProdutos >= 100) {
        printf("LIMITE DE PRODUTOS ATINGIDO!\n");
        return;
    }

    printf("DIGITE O CÓDIGO DO PRODUTO: ");
    scanf("%s", produtos[numProdutos].codigo);
    printf("DIGITE O NOME DO PRODUTO: ");
    scanf(" %[^\n]", produtos[numProdutos].nome);
    printf("DIGITE O PESO DO PRODUTO (em KG): ");
    scanf("%f", &produtos[numProdutos].peso);
    printf("DIGITE 1 PARA GRANEL OU 2 PARA UNITARIO: ");
    int tipo;
    scanf("%d", &tipo);
    strcpy(produtos[numProdutos].tipo, (tipo == 1) ? "GRANEL" : "UNITARIO");

    // Atualiza o arquivo
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "a");
    if (arquivo) {
        fprintf(arquivo, "%s;%s;%.2f;%s;\n", produtos[numProdutos].codigo, produtos[numProdutos].nome,
                produtos[numProdutos].peso, produtos[numProdutos].tipo);
        fclose(arquivo);
        numProdutos++;
        printf("PRODUTO CADASTRADO COM SUCESSO!\n");
    } else {
        printf("ERRO AO ABRIR O ARQUIVO.\n");
    }
}

// Função para alterar um produto
void alterarProduto() {
    char codigo[3];
    printf("DIGITE O CODIGO OU NOME DO PRODUTO A SER ALTERADO: ");
    scanf(" %[^\n]", codigo);

    for (int i = 0; i < numProdutos; i++) {
        if (strcmp(produtos[i].codigo, codigo) == 0 || strcmp(produtos[i].nome, codigo) == 0) {
            printf("NOVO NOME DO PRODUTO: ");
            scanf(" %[^\n]", produtos[i].nome);
            printf("DIGITE 1 PARA GRANEL OU 2 PARA UNITARIO: ");
            int tipo;
            scanf("%d", &tipo);
            strcpy(produtos[i].tipo, (tipo == 1) ? "GRANEL" : "UNITARIO");

            // Atualiza o arquivo
            FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "w");
            for (int j = 0; j < numProdutos; j++) {
                fprintf(arquivo, "%s;%s;%.2f;%s;\n", produtos[j].codigo, produtos[j].nome,
                        produtos[j].peso, produtos[j].tipo);
            }
            fclose(arquivo);
            printf("PRODUTO ALTERADO COM SUCESSO!\n");
            return;
        }
    }
    printf("PRODUTO NÃO ENCONTRADO!\n");
}

// Função para remover um produto
void removerProduto() {
    char codigo[3];
    printf("DIGITE O CODIGO OU NOME DO PRODUTO A SER REMOVIDO: ");
    scanf(" %[^\n]", codigo);

    int i;
    for (i = 0; i < numProdutos; i++) {
        if (strcmp(produtos[i].codigo, codigo) == 0 || strcmp(produtos[i].nome, codigo) == 0) {
            break;
        }
    }

    if (i < numProdutos) {
        for (int j = i; j < numProdutos - 1; j++) {
            produtos[j] = produtos[j + 1]; // Move os produtos para frente
        }
        numProdutos--; // Reduz o número de produtos

        // Atualiza o arquivo
        FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "w");
        for (int j = 0; j < numProdutos; j++) {
            fprintf(arquivo, "%s;%s;%.2f;%s;\n", produtos[j].codigo, produtos[j].nome,
                    produtos[j].peso, produtos[j].tipo);
        }
        fclose(arquivo);
        printf("PRODUTO REMOVIDO COM SUCESSO!\n");
    } else {
        printf("PRODUTO NÃO ENCONTRADO!\n");
    }
}

// Função para pesar um produto
void pesarProduto() {
    char codigo[3];
    printf("DIGITE O CODIGO DO PRODUTO A SER PESADO: ");
    scanf("%s", codigo);

    float peso;
    printf("DIGITE O PESO DO PRODUTO: ");
    scanf("%f", &peso);

    int encontrado = 0; // Flag para verificar se o produto foi encontrado
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "r");
    FILE *temp = fopen("temp.txt", "w"); // Arquivo temporário para armazenar as atualizações

    if (arquivo == NULL || temp == NULL) {
        printf("ERRO AO ABRIR O ARQUIVO.\n");
        return;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), arquivo)) {
        Produto p;
        sscanf(linha, "%[^;];%[^;];%f;%[^;];", p.codigo, p.nome, &p.peso, p.tipo);

        if (strcmp(p.codigo, codigo) == 0) {
            // Atualiza o peso do produto
            p.peso = peso;
            encontrado = 1; // Marca que o produto foi encontrado
        }

        // Escreve os produtos (atualizados ou não) no arquivo temporário
        fprintf(temp, "%s;%s;%.2f;%s;\n", p.codigo, p.nome, p.peso, p.tipo);
    }

    fclose(arquivo);
    fclose(temp);

    // Substitui o arquivo original pelo temporário
    remove(ARQUIVO_PRODUTOS);
    rename("temp.txt", ARQUIVO_PRODUTOS);

    if (encontrado) {
        printf("PESO DO PRODUTO ATUALIZADO COM SUCESSO!\n");
    } else {
        printf("PRODUTO NÃO ENCONTRADO!\n");
    }
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    iniciarServidor(); // Inicia o servidor
    carregarProdutos(); // Carrega produtos do arquivo ao iniciar


    // Exemplo de uso
    int opcao;
    do {
        printf("\nMENU:\n");
        printf("1 - LISTAR PRODUTOS\n");
        printf("2 - CADASTRAR PRODUTO\n");
        printf("3 - ALTERAR PRODUTO\n");
        printf("4 - REMOVER PRODUTO\n");
        printf("5 - PESAR PRODUTO\n");
        printf("6 - SAIR\n");
        printf("DIGITE A OPCAO: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                listarProdutos();
                break;
            case 2:
                cadastrarProduto();
                break;
            case 3:
                alterarProduto();
                break;
            case 4:
                removerProduto();
                break;
            case 5:
                pesarProduto();
                break;
            case 6:
                printf("SAINDO...\n");
                break;
            default:
                printf("OPÇÃO INVÁLIDA!\n");
        }
    } while (opcao != 6);

    return 0;
}
