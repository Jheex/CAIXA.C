#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <locale.h>
#include <windows.h>
#include <wchar.h>
#include <ctype.h>

#define SERVER_IP "127.0.0.1"                // IP do servidor
#define PORTA 9090                           // Porta do servidor
#define MAX_PRODUTOS 300                     // Máximo de produtos cadastrados
#define ARQUIVO_PRODUTOS "Data/produtos.txt" // Nome do arquivo de produtos
#define ARQUIVO_VENDAS "Data/vendas.txt"     // Nome do arquivo de vendas

typedef struct
{
    char codigo[3];   // Código do produto
    char nome[50];    // Nome do produto
    float preco;      // Preço do produto
    char unidade[10]; // Unidade do produto
} Produto;

Produto produtos[MAX_PRODUTOS]; // Array para armazenar produtos
int total_produtos = 0;         // Contador de produtos cadastrados

// Função para exibir uma barra de progresso no terminal
void barra_progresso(int duracao)
{
    int largura = 50; // Largura da barra de progresso
    printf("Conectando ao servidor: [");

    for (int i = 0; i < largura; i++)
    {
        printf(" "); // Preenche a barra com espaços vazios inicialmente
    }
    printf("]\rConectando ao servidor: [");
    fflush(stdout);

    for (int i = 0; i < largura; i++)
    {
        printf("#"); // Preenche a barra de progresso com '#'
        fflush(stdout);
        Sleep(duracao); // Use Sleep para pausar (tempo em milissegundos)
    }
    printf("] \nCONEXAO COMPLETA!\n");
}

void pausar_e_limpar(int segundos)
{
    // Pausa a execução
#ifdef _WIN32
    Sleep(segundos * 1000); // Sleep em milissegundos
#else
    sleep(segundos); // sleep em segundos
#endif

    // Limpa a tela
#ifdef _WIN32
    system("cls"); // Limpa a tela no Windows
#else
    system("clear"); // Limpa a tela em sistemas Unix/Linux
#endif
}

void conectar_ao_servidor(SOCKET *s)
{
    struct sockaddr_in servidor;
    char linha[100] = ""; // Para armazenar a repetição de "=-=" 30 vezes
    char mensagem[300];   // Ajustar o tamanho do buffer para a mensagem completa

    // Constrói a linha "=-=" repetida 30 vezes
    for (int i = 0; i < 30; i++)
    {
        strcat(linha, "=-="); // Concatenar "=-="
    }

    // Constrói a mensagem final
    sprintf(mensagem, "%s\nCAIXA CONECTADO!\n%s\n", linha, linha);

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
        printf("FALHA AO CONECTAR AO SERVIDOR. O CAIXA VAI CONTINUAR FUNCIONANDO SEM O SERVIDOR..\n");
        return; // Permitir que o caixa continue
    }

    // Simula o envio de mensagem com uma barra de progresso
    barra_progresso(50); // Duração de cada passo da barra (em ms)

    // Envia a mensagem ao servidor
    send(*s, mensagem, strlen(mensagem), 0);
    printf("CONEXAO ESTABELECIDA COM O SERVIDOR!\n");

    pausar_e_limpar(2);
}

void carregar_produtos()
{
    FILE *file = fopen(ARQUIVO_PRODUTOS, "r");
    if (!file)
    {
        printf("FALHA AO ABRIR O ARQUIVO DE PRODUTOS.\n");
        return;
    }

    total_produtos = 0; // Zera o contador
    while (fscanf(file, " %2[^;];%49[^;];%f;%9[^;];", produtos[total_produtos].codigo, produtos[total_produtos].nome, &produtos[total_produtos].preco, produtos[total_produtos].unidade) == 4)
    {
        total_produtos++;
        if (total_produtos >= MAX_PRODUTOS)
            break; // Limita o número de produtos
    }

    fclose(file);
}

void salvar_produtos()
{
    FILE *file = fopen(ARQUIVO_PRODUTOS, "w");
    if (!file)
    {
        printf("FALHA AO ABRIR O ARQUIVO PARA SALVAR PRODUTOS.\n");
        return;
    }

    for (int i = 0; i < total_produtos; i++)
    {
        fprintf(file, "%s;%s;%.2f;%s;\n", produtos[i].codigo, produtos[i].nome, produtos[i].preco, produtos[i].unidade);
    }

    fclose(file);
    printf("PRODUTOS SALVOS COM SUCESSO!\n");
}

void listar_produtos()
{
    pausar_e_limpar(1); // 2 SEGUNDO

    if (total_produtos == 0)
    {
        printf("NAO HA PRODUTOS CADASTRADOS.\n");
        return;
    }
    printf("LISTANDO PRODUTOS:\n");
    for (int i = 0; i < total_produtos; i++)
    {
        printf("%s - %s - R$ %.2f - %s\n", produtos[i].codigo, produtos[i].nome, produtos[i].preco, produtos[i].unidade);
    }

    // Espera o usuário pressionar uma tecla para continuar
    printf("\nPRESSIONE ENTER PARA CONTINUAR...\n");
    getchar(); // Captura ENTER
    getchar(); // Adiciona uma segunda chamada para capturar qualquer entrada anterior do buffer

    pausar_e_limpar(1); // 2 SEGUNDO
}

int gerar_codigo_produto()
{
    if (total_produtos == 0)
    {
        return 1; // Se não houver produtos, o código começa em 1
    }
    return atoi(produtos[total_produtos - 1].codigo) + 1; // Gera o próximo código
}

// Função para escolher a unidade do produto
void escolherUnidade(char *unidade)
{
    int opcao;

    printf("\nESCOLHA A UNIDADE DO PRODUTO:\n");
    printf("1 - GRANEL\n");
    printf("2 - UNITARIO\n");
    printf("\nDIGITE A OPCÃO: ");
    scanf("%d", &opcao);

    switch (opcao)
    {
    case 1:
        strcpy(unidade, "GRANEL");
        break;
    case 2:
        strcpy(unidade, "UNITARIO");
        break;
    default:
        printf("\nOPCÃO INVÁLIDA. SELECIONE 1 OU 2.\n");
        escolherUnidade(unidade); // Tenta novamente em caso de erro
        return;
    }
}

void adicionar_produto()
{
    if (total_produtos >= MAX_PRODUTOS)
    {
        printf("CAPACIDADE MAXIMA DE PRODUTOS ALCANCADA.\n");
        return;
    }

    pausar_e_limpar(1);

    Produto novo_produto;
    sprintf(novo_produto.codigo, "%02d", gerar_codigo_produto()); // Gera o código automaticamente
    printf("\nCODIGO GERADO: %s\n", novo_produto.codigo);         // Exibe o código gerado

    printf("\nDIGITE O NOME DO PRODUTO: ");
    scanf(" %[^\n]", novo_produto.nome); // Lê o nome do produto

    printf("\nDIGITE O PRECO DO PRODUTO: ");
    scanf("%f", &novo_produto.preco); // Lê o preço do produto

    // printf("DIGITE A UNIDADE DO PRODUTO: ");
    // scanf(" %[^\n]", novo_produto.unidade); // Lê a unidade do produto
    escolherUnidade(novo_produto.unidade); // Chama a função para escolher a unidade

    produtos[total_produtos++] = novo_produto; // Adiciona o produto ao array
    printf("\nPRODUTO ADICIONADO COM SUCESSO!\n");

    salvar_produtos(); // Salva os produtos após adicionar

    pausar_e_limpar(2); // 2 SEGUNDO
}

void alterar_produto()
{
    pausar_e_limpar(1); // 2 SEGUNDO

    int index;
    listar_produtos();
    if (total_produtos == 0)
        return; // Retorna se não houver produtos

    printf("DIGITE O NUMERO DO PRODUTO A SER ALTERADO: ");
    scanf("%d", &index);
    index--; // Ajusta para índice do array

    if (index < 0 || index >= total_produtos)
    {
        printf("PRODUTO INEXISTENTE.\n");
        return;
    }

    // printf("DIGITE O NOVO CODIGO DO PRODUTO: ");
    // scanf(" %[^\n]", produtos[index].codigo); // Lê o novo código

    printf("DIGITE O NOVO NOME DO PRODUTO: ");
    scanf(" %[^\n]", produtos[index].nome); // Lê o novo nome
    printf("DIGITE O NOVO PRECO DO PRODUTO: ");
    scanf("%f", &produtos[index].preco); // Lê o novo preço

    // printf("DIGITE SE O PRODUTO É UNITARIO OU GRANEL: ");
    // scanf(" %[^\n]", produtos[index].unidade); // Lê a nova unidade
    escolherUnidade(produtos[index].unidade); // Chama a função para escolher a unidade

    printf("PRODUTO ALTERADO COM SUCESSO!\n");
    salvar_produtos(); // Salva os produtos após alterar

    pausar_e_limpar(2); // 2 SEGUNDO
}

void remover_produto()
{

    pausar_e_limpar(1); // 2 SEGUNDO
    int index;
    listar_produtos();
    if (total_produtos == 0)
        return; // Retorna se não houver produtos

    printf("DIGITE O NUMERO DO PRODUTO A SER REMOVIDO: ");
    scanf("%d", &index);
    index--; // Ajusta para índice do array

    if (index < 0 || index >= total_produtos)
    {
        printf("PRODUTO INEXISTENTE.\n");
        return;
    }

    // Move os produtos para preencher a lacuna
    for (int i = index; i < total_produtos - 1; i++)
    {
        produtos[i] = produtos[i + 1];
    }
    total_produtos--; // Reduz o contador de produtos

    printf("PRODUTO REMOVIDO COM SUCESSO!\n");
    salvar_produtos();  // Salva os produtos após remover
    pausar_e_limpar(2); // 2 SEGUNDO
}

#include <stdio.h>

void realizar_pagamento(float total, FILE *file)
{
    float pagamento;
    int forma_pagamento;

    fprintf(file, "VALOR TOTAL: R$ %.2f\n", total); // Salva o total da venda

    // Escolha da forma de pagamento
    printf("ESCOLHA A FORMA DE PAGAMENTO:\n");
    printf("1 - Dinheiro\n");
    printf("2 - Cartão de Crédito\n");
    printf("3 - Cartão de Débito\n");
    printf("\nDIGITE O NÚMERO DA OPCÃO DESEJADA: ");
    scanf("%d", &forma_pagamento);

    // Validação da forma de pagamento
    while (forma_pagamento < 1 || forma_pagamento > 3)
    {
        printf("OPCÃO INVÁLIDA. TENTE NOVAMENTE.\n");
        printf("DIGITE O NÚMERO DA OPCÃO DESEJADA: ");
        scanf("%d", &forma_pagamento);
    }

    // Loop para garantir que o pagamento seja suficiente
    do
    {
        printf("\nDIGITE O VALOR DO PAGAMENTO: R$ ");
        scanf("%f", &pagamento);

        if (pagamento < total)
        {
            printf("\nVALOR INSERIDO INSUFICIENTE! TENTE NOVAMENTE.\n");
        }
    } while (pagamento < total); // Repete até que o pagamento seja maior ou igual ao total

    // Calcula e exibe o troco (apenas se for pagamento em dinheiro)
    if (forma_pagamento == 1)
    {
        float troco = pagamento - total;
        printf("TROCO: R$ %.2f\n", troco);
        fprintf(file, "TROCO: R$ %.2f\n", troco);
    }

    // Salva as informações de pagamento no arquivo
    const char *forma;
    switch (forma_pagamento)
    {
    case 1:
        forma = "Dinheiro";
        break;
    case 2:
        forma = "Cartão de Crédito";
        break;
    case 3:
        forma = "Cartão de Débito";
        break;
    }

    fprintf(file, "FORMA DE PAGAMENTO: %s\n", forma);
    fprintf(file, "VALOR DO PAGAMENTO: R$ %.2f\n", pagamento);

    // Fecha o arquivo
    fclose(file);
    printf("\nVENDA REALIZADA COM SUCESSO!\n");
}

// Função para converter uma string para maiúsculas
void str_toupper(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void realizar_venda()
{
    if (total_produtos == 0)
    {
        printf("NAO HA PRODUTOS CADASTRADOS.\n");
        return; // Verifica se há produtos antes de realizar a venda
    }

    pausar_e_limpar(1);

    char codigo[3];
    float total = 0;
    int quantidade[MAX_PRODUTOS] = {0}; // Para armazenar a quantidade de cada produto vendido
    int vendido[MAX_PRODUTOS] = {0};    // Para marcar se o produto foi vendido

    printf("\n=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
    printf("--> REALIZANDO VENDA...");
    printf("\n=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");

    while (1)
    {
        printf("\nDIGITE O CODIGO DO PRODUTO (OU 'FINALIZAR' OU '0' PARA ENCERRAR A VENDA): ");
        scanf(" %[^\n]", codigo); // Lê o código do produto, incluindo espaços

        // Converte o código para maiúsculas
        str_toupper(codigo);

        // Verifica se o código é "FINALIZAR" ou "0"
        if (strcmp(codigo, "FINALIZAR") == 0 || strcmp(codigo, "0") == 0)
        {
            printf("VENDA ENCERRADA!\n");
            break; // Encerra a venda
        }

        // Busca o produto pelo código
        for (int i = 0; i < total_produtos; i++)
        {
            if (strcmp(produtos[i].codigo, codigo) == 0)
            {
                printf("PRODUTO ADICIONADO: %s - PRECO: R$ %.2f\n", produtos[i].nome, produtos[i].preco);
                total += produtos[i].preco; // Adiciona o preço ao total
                vendido[i]++;               // Marca que o produto foi vendido
                quantidade[i]++;            // Aumenta a quantidade vendida do produto
                break;                      // Sai do loop após encontrar o produto
            }
        }
    }

    // Exibe o histórico de vendas
    FILE *file = fopen(ARQUIVO_VENDAS, "a");
    if (!file)
    {
        printf("FALHA AO ABRIR O ARQUIVO DE VENDAS.\n");
        return;
    }

    time_t agora = time(NULL);
    struct tm *tm_info = localtime(&agora);

    pausar_e_limpar(1);

    // Exibe e salva o histórico da venda
    printf("\n=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
    printf("HISTORICO DA VENDA:");
    printf("\n=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
    fprintf(file, "VENDA REALIZADA EM: %02d/%02d/%04d - %02d:%02d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900, tm_info->tm_hour, tm_info->tm_min);

    for (int i = 0; i < total_produtos; i++)
    {
        if (vendido[i] > 0)
        {
            printf("CDP: %s - PRODUTO: %s - QTD: %d - PRECO: R$ %.2f\n", produtos[i].codigo, produtos[i].nome, vendido[i], produtos[i].preco);
            fprintf(file, "CDP: %s - PRODUTO: %s - QTD: %d - PRECO: R$ %.2f\n", produtos[i].codigo, produtos[i].nome, vendido[i], produtos[i].preco);
        }
    }

    printf("\nVALOR TOTAL: R$ %.2f\n", total);
    fprintf(file, "VALOR TOTAL: R$ %.2f\n", total); // Salva o total da venda

    realizar_pagamento(total, file);
    pausar_e_limpar(2); // 2 SEGUNDO
}

void exibir_historico_vendas()
{
    FILE *file = fopen(ARQUIVO_VENDAS, "r");
    if (!file)
    {
        printf("FALHA AO ABRIR O ARQUIVO DE VENDAS.\n");
        return;
    }

    char linha[256];
    printf("HISTORICO DE VENDAS:\n");
    while (fgets(linha, sizeof(linha), file))
    {
        printf("%s", linha);
    }

    fclose(file);
}

int main()
{
    setlocale(LC_ALL, "");
    SetConsoleOutputCP(CP_UTF8);
    SOCKET s;
    conectar_ao_servidor(&s); // Conecta ao servidor

    carregar_produtos(); // Carrega produtos do arquivo

    int opcao;
    do
    {
        printf("=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
        printf("--> MENU DO CAIXA <--");
        printf("\n=-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
        printf("1 - LISTAR PRODUTOS\n");
        printf("2 - ADICIONAR PRODUTO\n");
        printf("3 - ALTERAR PRODUTO\n");
        printf("4 - REMOVER PRODUTO\n");
        printf("5 - REALIZAR VENDA\n");
        printf("6 - EXIBIR HISTORICO DE VENDAS\n");
        printf("0 - SAIR\n\n");
        printf("-> DIGITE A OPCAO: ");
        scanf("%d", &opcao);

        switch (opcao)
        {
        case 1:
            listar_produtos();
            break;
        case 2:
            adicionar_produto();
            break;
        case 3:
            alterar_produto();
            break;
        case 4:
            remover_produto();
            break;
        case 5:
            realizar_venda();
            break;
        case 6:
            exibir_historico_vendas();
            break;
        case 0:
            printf("SAINDO...\n");
            break;
        default:
            printf("OPCAO INVALIDA. TENTE NOVAMENTE.\n");
            break;
        }
    } while (opcao != 0);

    salvar_produtos(); // Salva produtos ao sair
    closesocket(s);    // Fecha o socket
    WSACleanup();      // Limpa a biblioteca de sockets
    return 0;
}
