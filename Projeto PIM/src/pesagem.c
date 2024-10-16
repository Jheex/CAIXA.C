#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <winsock2.h>
#include <locale.h>
#include <windows.h>  // Inclui a função Sleep()

#pragma comment(lib, "ws2_32.lib")

// #define SERVER_IP "127.0.0.1"  // IP do servidor
// #define MAX_PRODUTOS 300
// #define ARQUIVO_PRODUTOS "Data/produtos.txt"
// #define HISTORICO_PESAGEM "Data/historico.txt"
// #define PORT 9090
// #define BUFFER_SIZE 1024

#define SERVER_IP "127.0.0.1"                // IP do servidor
#define PORTA 9090                           // Porta do servidor
#define MAX_PRODUTOS 300                     // Máximo de produtos cadastrados
#define ARQUIVO_PRODUTOS "Data/produtos.txt" // Nome do arquivo de produtos
#define ARQUIVO_VENDAS "Data/vendas.txt"     // Nome do arquivo de vendas
#define HISTORICO_PESAGEM "Data/historico.txt"
#define BUFFER_SIZE 1024

typedef struct
{
    char codigo[3]; // Código do produto
    char nome[50];  // Nome do produto
    float peso;     // Peso total do produto
    char tipo[10];  // Tipo do produto (granel ou unitário)
} Produto;

Produto produtos[MAX_PRODUTOS];
int numProdutos = 0; // Total de produtos cadastrados

void conectar_ao_servidor(SOCKET *s)
{
    struct sockaddr_in servidor;
    char mensagem[200] = "PESAGEM CONECTADO!";

    // Inicializa a biblioteca de sockets
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("FALHA AO INICIALIZAR A BIBLIOTECA DE SOCKETS.\n");
        return;
    }

    // Cria o socket
    *s = socket(AF_INET, SOCK_STREAM, 0);
    if (*s == INVALID_SOCKET)
    {
        printf("FALHA AO CRIAR SOCKET.\n");
        WSACleanup();
        return;
    }

    // Configura o endereço do servidor
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PORTA);
    servidor.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Tenta conectar ao servidor
    if (connect(*s, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
    {
        printf("FALHA AO CONECTAR AO SERVIDOR. PESAGEM VAI CONTINUAR FUNCIONANDO SEM O SERVIDOR..\n");
        return; // Permitir que o caixa continue
    }

    // Envia uma mensagem ao servidor
    send(*s, mensagem, strlen(mensagem), 0);
    printf("CONEXAO ESTABELECIDA COM O SERVIDOR!\n");
}

// Função para carregar produtos do arquivo
void carregarProdutosDoArquivo()
{
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "r");
    if (arquivo == NULL)
    {
        printf("ERRO AO ABRIR O ARQUIVO. PODE SER QUE NAO HAJA PRODUTOS CADASTRADOS.\n");
        return;
    }

    while (fscanf(arquivo, "%[^;];%[^;];%f;%[^;];\n", produtos[numProdutos].codigo, produtos[numProdutos].nome,
                  &produtos[numProdutos].peso, produtos[numProdutos].tipo) != EOF)
    {
        numProdutos++;
        if (numProdutos >= MAX_PRODUTOS)
        {
            break; // Para evitar ultrapassar o limite do vetor
        }
    }
    fclose(arquivo);
}

// Função para cadastrar um novo produto
void cadastrarProduto()
{
    if (numProdutos >= MAX_PRODUTOS)
    {
        printf("LIMITE DE PRODUTOS ALCANÇADO!\n");
        return;
    }

    Produto novoProduto;
    printf("DIGITE O NOME DO PRODUTO: ");
    scanf(" %[^\n]%*c", novoProduto.nome);

    // Pergunta se é granel ou unitário
    int tipo;
    printf("DIGITE 1 PARA GRANEL OU 2 PARA UNITARIO: ");
    scanf("%d", &tipo);
    strcpy(novoProduto.tipo, (tipo == 1) ? "GRANEL" : "UNITARIO");
    printf("PRODUTO CADASTRADO! ");
    printf("PRESSIONE ENTER PARA CONTINUAR... ");
    getchar();

    // Gera um código automático
    snprintf(novoProduto.codigo, sizeof(novoProduto.codigo), "%02d", numProdutos + 1);
    novoProduto.peso = 0.0; // O peso será definido posteriormente

    // Adiciona o novo produto ao array
    produtos[numProdutos++] = novoProduto;

    // Grava o produto no arquivo
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "a");
    fprintf(arquivo, "%s;%s;%.2f;%s;\n", novoProduto.codigo, novoProduto.nome, novoProduto.peso, novoProduto.tipo);
    fclose(arquivo);

    printf("PRODUTO CADASTRADO COM SUCESSO!\n");
    system("cls");
}

// Função para listar todos os produtos cadastrados
void listarProdutos()
{
    printf("PRODUTOS CADASTRADOS:\n");
    for (int i = 0; i < numProdutos; i++)
    {
        printf("CÓDIGO: %s - NOME: %s - PESO: %.2f KG - TIPO: %s\n",
               produtos[i].codigo, produtos[i].nome, produtos[i].peso, produtos[i].tipo);
    }
}

// Função para alterar um produto existente
void alterarProduto()
{
    char entrada[100]; // Buffer para entrada do usuário
    printf("DIGITE O CODIGO OU NOME DO PRODUTO A SER ALTERADO: ");
    scanf(" %[^\n]%*c", entrada); // Lê a entrada

    for (int i = 0; i < numProdutos; i++)
    {
        // Verifica se a entrada corresponde ao código ou ao nome do produto
        if (strcmp(produtos[i].codigo, entrada) == 0 || strcmp(produtos[i].nome, entrada) == 0)
        {
            printf("NOVO NOME DO PRODUTO: ");
            scanf(" %[^\n]%*c", produtos[i].nome);
            printf("DIGITE 1 PARA GRANEL OU 2 PARA UNITARIO: ");
            int tipo;
            scanf("%d", &tipo);
            strcpy(produtos[i].tipo, (tipo == 1) ? "GRANEL" : "UNITARIO");

            // Atualiza o arquivo
            FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "w");
            for (int j = 0; j < numProdutos; j++)
            {
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
void removerProduto()
{
    char entrada[100]; // Buffer para entrada do usuário
    printf("DIGITE O CODIGO OU NOME DO PRODUTO A SER REMOVIDO: ");
    scanf(" %[^\n]%*c", entrada); // Lê a entrada

    int i;
    for (i = 0; i < numProdutos; i++)
    {
        // Verifica se a entrada corresponde ao código ou ao nome do produto
        if (strcmp(produtos[i].codigo, entrada) == 0 || strcmp(produtos[i].nome, entrada) == 0)
        {
            break;
        }
    }

    if (i < numProdutos)
    {
        for (int j = i; j < numProdutos - 1; j++)
        {
            produtos[j] = produtos[j + 1]; // Move os produtos para frente
        }
        numProdutos--; // Reduz o número de produtos

        // Atualiza o arquivo
        FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "w");
        for (int j = 0; j < numProdutos; j++)
        {
            fprintf(arquivo, "%s;%s;%.2f;%s;\n", produtos[j].codigo, produtos[j].nome,
                    produtos[j].peso, produtos[j].tipo);
        }
        fclose(arquivo);
        printf("PRODUTO REMOVIDO COM SUCESSO!\n");
    }
    else
    {
        printf("PRODUTO NÃO ENCONTRADO!\n");
    }
}

// Função para pesar um produto
void pesarProduto(SOCKET cliente_socket)
{
    char nome[50];
    float peso;
    char buffer[BUFFER_SIZE];

    printf("PESAGEM DE PRODUTOS:\n");
    while (1)
    {
        printf("DIGITE 'LISTA' PARA VER OS PRODUTOS CADASTRADOS\n");
        printf("DIGITE O NOME DO PRODUTO (OU 'SAIR' PARA FINALIZAR): ");
        scanf(" %[^\n]%*c", nome);
        if (strcmp(nome, "SAIR") == 0)
        {
            break;
        }
        if (strcmp(nome, "LISTA") == 0)
        {
            listarProdutos();
            continue;
        }

        printf("DIGITE O PESO (KG) DO PRODUTO: ");
        scanf("%f", &peso);
        getchar(); // Limpa o buffer do teclado
        printf("O PESO DO PRODUTO FOI CADASTRADO COM SUCESSO!\n");

        // Abre o arquivo de produtos para leitura
        FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "r");
        if (arquivo == NULL)
        {
            printf("Erro ao abrir o arquivo de produtos!\n");
            return;
        }

        // Cria uma lista temporária para armazenar produtos atualizados
        Produto produtosTemp[MAX_PRODUTOS];
        int numProdutosTemp = 0;
        char linha[100];

        // Lê os produtos do arquivo e atualiza o peso do produto correspondente
        while (fgets(linha, sizeof(linha), arquivo))
        {
            Produto p;
            sscanf(linha, "%[^;];%[^;];%f;%[^;];", p.codigo, p.nome, &p.peso, p.tipo);
            if (strcmp(p.nome, nome) == 0)
            {
                // Atualiza o peso
                p.peso = peso;
            }
            produtosTemp[numProdutosTemp++] = p; // Adiciona o produto à lista temporária
        }
        fclose(arquivo); // Fecha o arquivo de produtos

        // Abre o arquivo de produtos para gravação
        arquivo = fopen(ARQUIVO_PRODUTOS, "w");
        if (arquivo == NULL)
        {
            printf("Erro ao abrir o arquivo de produtos para gravação!\n");
            return;
        }

        // Grava todos os produtos, incluindo o atualizado
        for (int i = 0; i < numProdutosTemp; i++)
        {
            fprintf(arquivo, "%s;%s;%.2f;%s;\n", produtosTemp[i].codigo, produtosTemp[i].nome, produtosTemp[i].peso, produtosTemp[i].tipo);
        }
        fclose(arquivo); // Fecha o arquivo de produtos

        // Formata a string para enviar ao servidor
        snprintf(buffer, sizeof(buffer), "PESAR %s;%.2f", nome, peso);

        // Envia o comando para o servidor
        send(cliente_socket, buffer, strlen(buffer), 0);

        // Recebe a resposta do servidor
        int bytes_recebidos = recv(cliente_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recebidos > 0)
        {
            buffer[bytes_recebidos] = '\0'; // Adiciona terminador nulo
            printf("Resposta do servidor: %s\n", buffer);
        }
        else
        {
            printf("Erro ao receber resposta do servidor: %d\n", WSAGetLastError());
            break; // Sai do loop se houver erro
        }

        // Registra a pesagem no histórico
        FILE *historico = fopen(HISTORICO_PESAGEM, "a");
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(historico, "PESAGEM - NOME: %s - PESO: %.2f KG - DATA: %02d/%02d/%04d - %02d:%02d\n",
                nome, peso, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
        fclose(historico);
        return;
    }
}

// Função para listar o histórico de pesagem
void listarHistorico()
{
    char linha[BUFFER_SIZE];
    FILE *historico = fopen(HISTORICO_PESAGEM, "r");
    if (historico == NULL)
    {
        printf("NENHUM HISTÓRICO DE PESAGENS ENCONTRADO.\n");
        return;
    }

    printf("HISTÓRICO DE PESAGENS:\n");
    while (fgets(linha, sizeof(linha), historico))
    {
        printf("%s", linha);
    }
    fclose(historico);
}

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    WSADATA wsaData;
    SOCKET cliente_socket;
    // SOCKET s;
    struct sockaddr_in server_addr;

    conectar_ao_servidor(&cliente_socket); // Conecta ao servidor

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Falha ao inicializar o Winsock. Código de erro: %d\n", WSAGetLastError());
        return 1;
    }

    // Cria um socket para o cliente
    cliente_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cliente_socket == INVALID_SOCKET)
    {
        printf("Erro ao criar o socket. Código de erro: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Define o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTA);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Endereço IP do servidor

    // Conecta ao servidor
    if (connect(cliente_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao conectar ao servidor. Código de erro: %d\n", WSAGetLastError());
        closesocket(cliente_socket);
        WSACleanup();
        return 1;
    }

    carregarProdutosDoArquivo(); // Carrega produtos ao iniciar

    int opcao;
    do
    {
        printf("\n==========================\n");
        printf("    MENU PESAGEM\n");
        printf("==========================\n");
        printf("1 - CADASTRAR PRODUTO\n");
        printf("2 - LISTAR PRODUTOS\n");
        printf("3 - ALTERAR PRODUTO\n");
        printf("4 - PESAR PRODUTO\n");
        printf("5 - LISTAR HISTORICO\n");
        printf("6 - REMOVER PRODUTO\n");
        printf("7 - SAIR\n");
        printf("=========================\n");
        printf("DIGITE A OPCAO: ");
        scanf("%d", &opcao);
        getchar(); // Limpa o buffer do teclado

        switch (opcao)
        {
        case 1:
            system("cls");
            cadastrarProduto();
            break;
        case 2:
            system("cls");
            listarProdutos();
            break;
        case 3:
            system("cls");
            alterarProduto();
            break;
        case 4:
            system("cls");
            pesarProduto(cliente_socket);
            break;
        case 5:
            system("cls");
            listarHistorico();
            break;
        case 6:
            system("cls");
            removerProduto();
            break;
        case 7:
            system("cls");
            printf("SAINDO...\n");
            break;
        default:
            system("cls");
            printf("OPÇÃO INVÁLIDA!\n");
            break;
        }
    } while (opcao != 7);

    // Fecha o socket e limpa o Winsock
    closesocket(cliente_socket);
    WSACleanup();
    return 0;
}
