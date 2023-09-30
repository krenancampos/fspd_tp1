#include "spend_time.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define NTHREADS_MAX	1000 /* 32 */

typedef struct dados_thread{
  int tid, tlivre, tcritico, qtd_recursos_pedidos;
  int* recursos_pedidos;
  pthread_t thread_atual;
} dados_thread;

typedef struct recurso{
  pthread_mutex_t mutex_recurso;
  pthread_cond_t recurso_livre;
  int ocupado;
} recurso;

recurso recursos_globais[8];

void init_recursos(){
  int i;
  for (i=0; i < 8; i++){
    recursos_globais[i].ocupado = 0;
    pthread_mutex_init(&(recursos_globais[i].mutex_recurso), NULL);
    pthread_cond_init(&(recursos_globais[i].recurso_livre), NULL);
  }
}


void trava_recursos(int* recursos_pedidos, int qtd_recursos_pedidos){
  int k, recurso_pedido, recurso_conseguido, qtd_recursos_conseguidos = 0;

  recurso_pedido = recursos_pedidos[0];
  pthread_mutex_lock(&(recursos_globais[recurso_pedido].mutex_recurso));
  recursos_globais[recurso_pedido].ocupado = 1;
  recurso_conseguido = recursos_pedidos[0];
  qtd_recursos_conseguidos = 1;

  if(qtd_recursos_pedidos == 1){
    return;
  }

  while(qtd_recursos_conseguidos < qtd_recursos_pedidos){

    recurso_pedido = recursos_pedidos[qtd_recursos_conseguidos];

    while(recursos_globais[recurso_pedido].ocupado){
      
      if (qtd_recursos_conseguidos > 1){

        for(k = qtd_recursos_conseguidos - 1; k == 1; k--){
          recurso_conseguido = recursos_pedidos[k];
          recursos_globais[recurso_conseguido].ocupado = 0;
          pthread_cond_broadcast(&(recursos_globais[recurso_conseguido].recurso_livre));
          pthread_mutex_unlock(&(recursos_globais[recurso_conseguido].mutex_recurso));
        }

        qtd_recursos_conseguidos = 1;
        recurso_conseguido = recursos_pedidos[0];
        recurso_pedido = recursos_pedidos[1];
      } 

      recursos_globais[recurso_conseguido].ocupado = 0;
      pthread_cond_broadcast(&(recursos_globais[recurso_conseguido].recurso_livre));
      pthread_cond_wait(&(recursos_globais[recurso_pedido].recurso_livre), &(recursos_globais[recurso_conseguido].mutex_recurso));
      recursos_globais[recurso_conseguido].ocupado = 1;
    }
    
    pthread_mutex_lock(&(recursos_globais[recurso_pedido].mutex_recurso));
    recursos_globais[recurso_pedido].ocupado = 1;
    recurso_conseguido = recurso_pedido;
    qtd_recursos_conseguidos++;
  }
}

void libera_recursos(int* recursos_pedidos, int qtd_recursos_pedidos){
  int i, recurso_conseguido;
  for(i =  0; i < qtd_recursos_pedidos; i++){
    recurso_conseguido = recursos_pedidos[i];
    recursos_globais[recurso_conseguido].ocupado = 0;
    pthread_cond_broadcast(&(recursos_globais[recurso_conseguido].recurso_livre));
    pthread_mutex_unlock(&(recursos_globais[recurso_conseguido].mutex_recurso));
  }
}

void* tarefa(void* params){
    dados_thread* parametros_lidos = (dados_thread*) params;
    int tid = parametros_lidos->tid;
    int tlivre = parametros_lidos->tlivre;
    int tcritico = parametros_lidos->tcritico;
    int qtd_recursos_pedidos = parametros_lidos->qtd_recursos_pedidos;
    int* recursos_pedidos = parametros_lidos->recursos_pedidos;
    
    spend_time(tid, NULL, tlivre);
    trava_recursos(recursos_pedidos, qtd_recursos_pedidos); 
    spend_time(tid, "C", tcritico);
    libera_recursos(recursos_pedidos, qtd_recursos_pedidos);  
    pthread_exit(NULL); 
}


int main(){
  int rc, t, qtd_parametros=0, n_threads_criadas=0, parametros[11], qtd_recursos_pedidos;
  char linha[32];
  dados_thread threads[NTHREADS_MAX];

  init_recursos();
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
        qtd_recursos_pedidos =  qtd_parametros - 3;
        threads[n_threads_criadas].qtd_recursos_pedidos = qtd_recursos_pedidos;
        threads[n_threads_criadas].recursos_pedidos = (int*) malloc(sizeof(int)*(qtd_recursos_pedidos));

        for(t = 0; t < qtd_recursos_pedidos; t++){
          threads[n_threads_criadas].recursos_pedidos[t] = parametros[3+t]; 
        }
        
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

    //printf("main(): all threads exited.\n");

    for(t = 0; t < n_threads_criadas; t++){
        free(threads[t].recursos_pedidos);
    }

    exit(0);
}
