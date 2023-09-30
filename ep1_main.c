#include "spend_time.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define NTHREADS_MAX	1000

/*  Cada thread tem: 
      -o seu identificador quando é criada (pthread_t);
      -os parâmetros (tid, tlivre, tcritico) lidos da entrada padrao;
      -um vetor de inteiros representando os recursos pedidos (0...7) do tamanho da qtd de recursos pedidos.
*/
typedef struct dados_thread{ 
  int tid, tlivre, tcritico, qtd_recursos_pedidos;
  int* recursos_pedidos;
  pthread_t thread_atual;
} dados_thread;
//-------------------------------------

/*  Os recursos foram representados como uma struct contendo:
      -uma variavel 'ocupado' q indica se o recurso já está sendo utilizado (1 - ocupado; 0 - livre);
      -um mutex que controla o acesso a variavel 'ocupado';
      -uma variavel de condicao 'recurso_livre' utilizada para sinalizar a liberação.
*/
typedef struct recurso{
  pthread_mutex_t mutex_recurso;
  pthread_cond_t recurso_livre;
  int ocupado;
} recurso;
//-------------------------------------

recurso recursos_globais[8]; // Como os 8 recursos são globais, é definido um vetor do tipo recurso.

void init_recursos(){ // Inicializa cada um dos 8 recursos globais
  int i;
  for (i=0; i < 8; i++){
    recursos_globais[i].ocupado = 0;
    pthread_mutex_init(&(recursos_globais[i].mutex_recurso), NULL);
    pthread_cond_init(&(recursos_globais[i].recurso_livre), NULL);
  }
}

void trava_recursos(int* recursos_pedidos, int qtd_recursos_pedidos){
  int k, recurso_pedido, recurso_conseguido, qtd_recursos_conseguidos = 0;

  recurso_pedido = recursos_pedidos[0]; // Inicialmente, para toda thread conseguir aguardar as variaveis de condicao dos recursos de que precisa,
                                        //  deve conseguir pelo menos o primeiro recurso, aguardando ate conseguir 'lockar' o mutex.
  pthread_mutex_lock(&(recursos_globais[recurso_pedido].mutex_recurso));
  recursos_globais[recurso_pedido].ocupado = 1;
  recurso_conseguido = recursos_pedidos[0];
  qtd_recursos_conseguidos = 1;

  if(qtd_recursos_pedidos == 1){  // Se não precisa de mais nenhum outro recurso alem daquele único que já conseguiu, volta pra execução da thread.
    return;
  }

  // Mas se precisar de mais de um recurso:
  while(qtd_recursos_conseguidos < qtd_recursos_pedidos){

    recurso_pedido = recursos_pedidos[qtd_recursos_conseguidos]; // O recurso pedido representa o próximo recurso

    while(recursos_globais[recurso_pedido].ocupado){
      
      if (qtd_recursos_conseguidos > 1){  //Se a thread já tem mais de um recurso e não conseguiu o atual por estar ocupado:

        for(k = qtd_recursos_conseguidos - 1; k == 1; k--){ //  libera todos os recursos, exceto o primeiro.
          recurso_conseguido = recursos_pedidos[k];
          recursos_globais[recurso_conseguido].ocupado = 0;
          pthread_cond_broadcast(&(recursos_globais[recurso_conseguido].recurso_livre));  // sinaliza p/ todas as q estavam esperando um dado recurso
          pthread_mutex_unlock(&(recursos_globais[recurso_conseguido].mutex_recurso));
        }

        qtd_recursos_conseguidos = 1; // mantém apenas o primeiro recurso para conseguir dar wait() na variavel de condicao e libera-lo em seguida
        recurso_conseguido = recursos_pedidos[0];
        recurso_pedido = recursos_pedidos[1];
      } 

      recursos_globais[recurso_conseguido].ocupado = 0; //  prepara para liberar o primeiro recurso
      pthread_cond_broadcast(&(recursos_globais[recurso_conseguido].recurso_livre));
      // aguarda o recurso q n conseguiu sinalizando e liberando o primeiro recurso
      pthread_cond_wait(&(recursos_globais[recurso_pedido].recurso_livre), &(recursos_globais[recurso_conseguido].mutex_recurso));
      // assim q retorna do wait, consegue a posse do primeiro recurso novamente atraves do mutex e muda a variavel ocupado
      recursos_globais[recurso_conseguido].ocupado = 1;
    }
    
    // Se o proximo recurso nao está bloqueado, se torna mais um recurso conseguido ao travar com o mutex e mudar a variavel 'ocupado'
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
  int rc, t, qtd_recursos_pedidos, qtd_parametros=0, n_threads_criadas=0;
  int parametros[11]; // Tamanho máximo de números inteiros em uma linha, considerando os 8 recursos mais os 3 inteiros (tid, tlivre e tcritico)
  char linha[32]; // Tamanho máximo da linha em caracteres, considerando que:
                  //  os 3 primeiros inteiros (tid, tlivre e tcritico) podem ter 4 digitos;
                  //  os 8 ultimos inteiros (os recursos) podem ter apenas um digito;
                  //  e os 11 espaços separando cada um dos inteiros e o caracter de fim de linha, totalizando assim 32 caracteres.
  dados_thread threads[NTHREADS_MAX];

  init_recursos();

  while ( fgets(linha, sizeof(linha), stdin) != NULL ) {
    char *token = strtok(linha, " "); // A função strtok() divide a linha lida em fgets() em tokens usando o caracter de espaço
                                      //  como delimitador, retornando um ponteiro para o próximo token encontrado 
                                      //  até retornar NULL quando a linha terminar
    qtd_parametros = 0;

    while (token != NULL) { //  O retorno de strtok é NULL quando a linha termina
      if ( sscanf(token, "%d", &parametros[qtd_parametros]) == 1) {
        qtd_parametros++;
      }
      token = strtok(NULL, " ");  // Como deseja-se continuar dividindo a mesma linha lida, 
                                  //   passa-se NULL como o primeiro parâmetro de strtok(),
                                  //   obtendo assim o próximo token da mesma linha até terminar
    }

    threads[n_threads_criadas].tid = parametros[0];
    threads[n_threads_criadas].tlivre = parametros[1];
    threads[n_threads_criadas].tcritico = parametros[2];
    qtd_recursos_pedidos =  qtd_parametros - 3;
    threads[n_threads_criadas].qtd_recursos_pedidos = qtd_recursos_pedidos;
    // Aloca um vetor de inteiros do tamanho da qtd de recursos pedidos e armazena nos dados da thread.
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
      free(threads[t].recursos_pedidos); // Libera todos os vetores alocados após o termino de todas as threads.
  }

  exit(0);
}
