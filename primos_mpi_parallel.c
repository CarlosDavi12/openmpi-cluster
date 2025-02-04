#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declaração de funções
int contar_primos(int limite, int id_processo, int num_processos);
void imprimir_timestamp();

int main(int argc, char *argv[])
{
    int id_processo, num_processos, erro_mpi;
    int limite_inicial = 1, limite_maximo = 1048576, fator_crescimento = 2;
    int total_primos, primos_parciais, limite_atual;
    double tempo_inicio;

    // Inicializa o ambiente MPI
    erro_mpi = MPI_Init(&argc, &argv);

    // Obtém o número total de processos
    erro_mpi = MPI_Comm_size(MPI_COMM_WORLD, &num_processos);

    // Obtém o identificador único do processo atual
    erro_mpi = MPI_Comm_rank(MPI_COMM_WORLD, &id_processo);

    // Apenas o processo mestre (id 0) exibe informações iniciais
    if (id_processo == 0)
    {
        imprimir_timestamp();
        printf("\nPRIME_MPI\n");
        printf("  Versão C/MPI\n");
        printf("  Contagem paralela de números primos usando MPI.\n");
        printf("  Número de processos: %d\n\n", num_processos);
        printf("         N        Pi          Tempo\n");
        printf("-----------------------------------\n");
    }

    limite_atual = limite_inicial;

    while (limite_atual <= limite_maximo)
    {
        // O processo mestre inicia a contagem do tempo
        if (id_processo == 0)
        {
            tempo_inicio = MPI_Wtime();
        }

        // Envia o limite_atual para todos os processos
        MPI_Bcast(&limite_atual, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Cada processo calcula sua parte dos primos
        primos_parciais = contar_primos(limite_atual, id_processo, num_processos);

        // Reduz os resultados individuais para o processo mestre
        MPI_Reduce(&primos_parciais, &total_primos, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // O processo mestre imprime os resultados
        if (id_processo == 0)
        {
            double tempo_total = MPI_Wtime() - tempo_inicio;
            printf("  %8d  %8d  %14f\n", limite_atual, total_primos, tempo_total);
        }

        // Aumenta o limite de busca
        limite_atual *= fator_crescimento;
    }

    // Finaliza o ambiente MPI
    MPI_Finalize();

    // O processo mestre finaliza com uma mensagem
    if (id_processo == 0)
    {
        printf("\nPRIME_MPI - Processo mestre:\n");
        printf("  Execução concluída com sucesso.\n\n");
        imprimir_timestamp();
    }

    return 0;
}

// Função que conta números primos até um determinado limite, distribuindo o trabalho entre os processos MPI
int contar_primos(int limite, int id_processo, int num_processos)
{
    int i, j, eh_primo, total_primos = 0;

    // Cada processo começa em um número diferente e salta pelo número total de processos
    for (i = 2 + id_processo; i <= limite; i += num_processos)
    {
        eh_primo = 1;
        for (j = 2; j < i; j++)
        {
            if (i % j == 0)
            {
                eh_primo = 0;
                break;
            }
        }
        total_primos += eh_primo;
    }
    return total_primos;
}

// Função que imprime um timestamp no formato "DD Mês AAAA HH:MM:SS AM/PM"
void imprimir_timestamp(void)
{
#define TAMANHO_HORA 40
    static char buffer_hora[TAMANHO_HORA];
    const struct tm *tm;
    time_t agora = time(NULL);
    tm = localtime(&agora);
    strftime(buffer_hora, TAMANHO_HORA, "%d %B %Y %I:%M:%S %p", tm);
    printf("%s\n", buffer_hora);
#undef TAMANHO_HORA
}
