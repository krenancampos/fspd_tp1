#include "spend_time.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define NTHREADS_MAX	1000 /* 32 */

int recursos[8];

void init_recursos(){
  int i;
  for (i=0; i < 8; i++){
    recursos[i] = i;
  }
}

/*
void trava_recursos(int* recursos_thread){

}

void libera_recursos(void){

}

*/

typedef struct dados_threads{
  int tid, tlivre, tcritico, qtd_recursos;
  int* recursos;
  pthread_t thread_atual;
} dados_threads;

void* tarefa(void* params){
    dados_threads* parametros_lidos = (dados_threads*) params;
    int i = 0;
    int tid = parametros_lidos->tid;
    int tlivre = parametros_lidos->tlivre;
    int tcritico = parametros_lidos->tcritico;
    int qtd_recursos = parametros_lidos->qtd_recursos;
    int recursos[qtd_recursos];
    //printf("Esta thread tem id = %d, tlivre = %d, tcritico = %d, qtd_recursos = %d \n", tid, tlivre, tcritico, qtd_recursos);
    for (i = 0; i < qtd_recursos; i++){
        recursos[i] = parametros_lidos->recursos[i];
        //printf("\nrecurso %d: %d", i, recursos[i]);
    }
    //printf("\nTermino da tread %d\n", tid);
    spend_time(tid, NULL, tlivre); // info: E
    //trava_recursos(thread_recursos);     // a forma de representar os recursos é uma decisão do desenvolvedor
    spend_time(tid, "C", tcritico); //info: C
    //libera_recursos();  
    pthread_exit(NULL); 
}


int main(){
  int rc, t, qtd_parametros=0, n_threads_criadas=0, parametros[11];
  char linha[32];
  dados_threads threads[NTHREADS_MAX];

  init_recursos();
  printf("Digite linhas com números inteiros separados por espaço (Ctrl+D para encerrar no Linux):\n");

    while ( fgets(linha, sizeof(linha), stdin) != NULL ) {
        char *token = strtok(linha, " "); 
        qtd_parametros = 0;

        while (token != NULL) {
            if ( sscanf(token, "%d", &parametros[qtd_parametros]) == 1) {
                qtd_parametros++;
            }
            token = strtok(NULL, " "); 
        }

        threads[n_threads_criadas].tid = parametros[0];
        threads[n_threads_criadas].tlivre = parametros[1];
        threads[n_threads_criadas].tcritico = parametros[2];
        threads[n_threads_criadas].qtd_recursos = qtd_parametros - 3;
        threads[n_threads_criadas].recursos = &parametros[3];

        rc = pthread_create(&threads[n_threads_criadas].thread_atual, NULL, tarefa, (void *)&threads[n_threads_criadas]);
        if (rc){
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            printf("Code %d= %s\n",rc,strerror(rc));
            exit(-1);
        }

        n_threads_criadas++;
    }

    for(t = 0; t < n_threads_criadas; t++){
        pthread_join(threads[t].thread_atual, NULL);
    }
    printf("main(): all threads exited.\n");
    exit(0);
}
