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


// No início da thread, ela deve receber como parâmetros seu tid, os tempos livre e crítico
// e os recursos que ela necessita para completar seu trabalho
// Em segunda, ela deve executar as operações:
*/

void tarefa(void* params){
  //spend_time(int tid, char* info, int time_ds)
  /*
  int* thread_params = *((int*) params);
  int tid = thread_params[0]; //*(thread_params[0])
  int tlivre = thread_params[1]; 
  int tcritico = thread_params[2];
  int* thread_recursos = thread_params[3];
  */
  int* numeros = (int*)params;
  /*
  spend_time(tid, NULL, tlivre); // info: E
  trava_recursos(thread_recursos);     // a forma de representar os recursos é uma decisão do desenvolvedor
  spend_time(tid, "C", tcritico); //info: C
  libera_recursos();            // note que cada thread deve ser ter sua lista de recursos registrada em algum lugar
  */
  pthread_exit(NULL); 
}

struct params_thread{
  int tid, tlivre, tcritico, qtdRecursos;
  int* recursos;
};

int main(){
  pthread_t threads[NTHREADS_MAX];
  int rc, t;
  int* params; // [tid, tlivre, tcritico, *recursos_thread]
  int tid, tlivre, tcritico, *recursos_thread;
  struct params_thread parametros_lidos;

  int numeros[11]; // Tamanho máximo de números inteiros em uma linha, considerando os 8 recursos mais os 3 inteiros (tid, tlivre e tcritico)
  char linha[32]; // Tamanho máximo da linha em caracteres, considerando que:
                  //  os 3 primeiros inteiros (tid, tlivre e tcritico) podem ter 4 digitos;
                  //  os 8 ultimos inteiros (os recursos) podem ter apenas um digito;
                  //  e os 11 espaços separando cada um dos inteiros e o caracter de fim de linha, totalizando assim 32 caracteres.
  int qtdNumeros = 0;

  init_recursos();
  printf("Digite linhas com números inteiros separados por espaço (Ctrl+D para encerrar no Linux):\n");

  while ( fgets(linha, sizeof(linha), stdin) != NULL ) {
      char *token = strtok(linha, " "); // A função strtok() divide a linha lida em fgets() em tokens usando o caracter de espaço
                                        //  como delimitador, retornando um ponteiro para o próximo token encontrado 
                                        //  até retornar NULL quando a linha terminar
      qtdNumeros = 0;

      while (token != NULL) { //  O retorno de strtok é NULL quando a linha termina
          if ( sscanf(token, "%d", &numeros[qtdNumeros]) == 1) {
              qtdNumeros++;
          }
          token = strtok(NULL, " "); // Como deseja-se continuar dividindo a mesma linha lida, 
                                     //   passa-se NULL como o primeiro parâmetro de strtok(),
                                     //   obtendo assim o próximo token da mesma linha até terminar
      }

      // Agora, 'numeros' contém os números inteiros da linha e 'qtdNumeros' indica quantos foram lidos
      //printf("Você digitou %d números inteiros: \n", qtdNumeros);
      //printf("tid: %d \n", numeros[0]);
      //printf("tlivre: %d \n", numeros[1]);
      //printf("tcritico: %d \n", numeros[2]);
      //printf("recursos: ");
      /*
      for (int i = 3; i < qtdNumeros; i++) {
          printf(" %d ", numeros[i]);
      }
      printf("\n");
      */

      parametros_lidos.tid = numeros[0];
      parametros_lidos.tlivre = numeros[1];
      parametros_lidos.tcritico = numeros[2];
      parametros_lidos.qtdRecursos = qtdNumeros - 3;
      parametros_lidos.recursos = &numeros[3];
      rc = pthread_create(&threads[t], NULL, tarefa, (void *)&parametros_lidos);
        if (rc){
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            printf("Code %d= %s\n",rc,strerror(rc));
            exit(-1);
        }

        for(t=0; t<NTHREADS_MAX; t++){
          pthread_join(threads[t],NULL);
        }
      printf("main(): all threads exited.\n");
      exit(0);
  }

//int tid, tlivre, tcritico, *recursos_thread;
//int** params; // [&tid, &tlivre, &tcritico, &recursos_thread]

//scanf(tid, tlivre, tcritico, recursos_thread);

  /* for(t=0; t<NTHREADS; t++){
      //rc = pthread_create(&threads[t], NULL, tarefa, (void *)&params[t])
        rc = pthread_create(&threads[t], NULL, tarefa, NULL);
        if (rc){
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            printf("Code %d= %s\n",rc,strerror(rc));
            exit(-1);
        }
    }
  */
}
