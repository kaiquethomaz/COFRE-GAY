//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  || |\| || |\ ||   |\ ||   || || | |  |   || || |\/| || |  || | |   /|    //    
//         ||  ||  |\  | | | | |/ | |   |/ | |   |   |\  ||  || |\  | | |  | | | |_ | | ||   _|   //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h> 
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h" 
#include "esp_log.h" 
#include "ioplaca.h"  
#include "lcdvia595.h" 
#include "driver/adc.h" 
#include "hcf_adc.h" 
#include "MP_hcf.h" 
#include "esp_err.h" 
// Área das macros
//-----------------------------------------------------------------------------------------------------------------------

#define DESLIGAR_TUDO       saidas&=0b00000000 // desligar todas as saídas
#define LIGAR_RELE_1        saidas|=0b00000001 // ligar o relé 1
#define DESLIGAR_RELE_1     saidas&=0b11111110 // desligar o relé 1
#define LIGAR_RELE_2        saidas|=0b00000010 // ligar o relé 2
#define DESLIGAR_RELE_2     saidas&=0b11111101 // desligar o relé 2
#define LIGAR_TRIAC_1       saidas|=0b00000100 // ligar o triac 1
#define DESLIGAR_TRIAC_1    saidas&=0b11111011 // desligar o triac 1
#define LIGAR_TRIAC_2       saidas|=0b00001000 // ligar o triac 2
#define DESLIGAR_TRIAC_2    saidas&=0b11110111 // desligar o triac 2
#define LIGAR_TBJ_1         saidas|=0b00010000 // ligar o transistor bipolar de junção (TBJ) 1
#define DESLIGAR_TBJ_1      saidas&=0b11101111 // desligar o TBJ 1
#define LIGAR_TBJ_2         saidas|=0b00100000 // ligar o TBJ 2
#define DESLIGAR_TBJ_2      saidas&=0b11011111 // desligar o TBJ 2
#define LIGAR_TBJ_3         saidas|=0b01000000 // ligar o TBJ 3
#define DESLIGAR_TBJ_3      saidas&=0b10111111 // desligar o TBJ 3
#define LIGAR_TBJ_4         saidas|=0b10000000 // ligar o TBJ 4
#define DESLIGAR_TBJ_4      saidas&=0b01111111 // desligar o TBJ 4


#define TECLA_1 le_teclado() == '1' // tecla 1 ser pressionada
#define TECLA_2 le_teclado() == '2' // tecla 2 ser pressionada
#define TECLA_3 le_teclado() == '3' // tecla 3 ser pressionada
#define TECLA_4 le_teclado() == '4' // tecla 4 ser pressionada
#define TECLA_5 le_teclado() == '5' // tecla 5 ser pressionada
#define TECLA_6 le_teclado() == '6' // tecla 6 ser pressionada
#define TECLA_7 le_teclado() == '7' // tecla 7 ser pressionada
#define TECLA_8 le_teclado() == '8' // tecla 8 ser pressionada
#define TECLA_0 le_teclado() == '0' // tecla 0 ser pressionada

// Definir macros para os sensores de fim de curso
#define FIM_DE_CURSO_ABERTO  (entradas & 0b00000001) // Fim de curso de aberto na entrada 1
#define FIM_DE_CURSO_FECHADO (entradas & 0b00000010) // Fim de curso de fechado na entrada 2

// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

static const char *TAG = "Placa";
static uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas

int controle = 0; // Controle para entrada de dígitos
int numero1 = 0; // Armazenar número digitado
int qdig = 0; // Quantidade de dígitos inseridos
int coluna = 0; // Controle de coluna no display LCD
int resultado = 0; // Resultado de alguma operação (não usado no código atual)
char operador; // Operador (não usado no código atual)
char tecla; // Tecla pressionada
char mostra[40]; // String para mostrar no display LCD
uint32_t adcvalor = 0; // Valor lido do ADC
int auxilia = 0;  // Auxiliar (não usado no código atual)
int erros = 0; // Contador de erros de senha

// Função sempre ativa(?)
//-----------------------------------------------------------------------------------------------------------------------


// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------





// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    MP_init(); // configura pinos do motor
    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)
    
    // inicializar os IOs e teclado da placa
    ioinit();      
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas

    // inicializar o display LCD 
    lcd595_init();
    lcd595_write(1,1,"    Jornada 1   ");
    lcd595_write(2,1," Programa Basico");
    
    // Inicializar o componente de leitura de entrada analógica
    esp_err_t init_result = hcf_adc_iniciar();
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o componente ADC personalizado");
    }

    // inica motor
    DRV_init(6, 7);

    //delay inicial
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    lcd595_clear();

    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados
    hcf_adc_ler(&adcvalor);

    if(adcvalor > 350)
       {
           hcf_adc_ler(&adcvalor);
           while(adcvalor > 350)
           {
                hcf_adc_ler(&adcvalor);
                rotacionar_DRV(0, 11, saidas);
           }
      }

    /////////////////////////////////////////////////////////////////////////////////////   Início do ramo principal                    
   while(1)
    {
       hcf_adc_ler(&adcvalor); // Ler valor do ADC
    

       tecla = le_teclado(); // Ler tecla pressionada

        if(tecla>='0' && tecla <='9')
        {
          if(controle == 0)
            {
                numero1 = numero1 * 10 + tecla - '0'; // Constrói o número digitado
                qdig = qdig + 1; // Incrementa a quantidade de dígitos
            }
        }

        if(tecla == 'C')
        {
            numero1 = 0; // Reseta número digitado
            qdig = 0; // Reseta quantidade de dígitos
        }

        lcd595_write(1,0, "Digite a senha!"); // Mensagem no display


        if(tecla!= '_')
        {
            sprintf(&mostra[0], "%c", tecla); // Mostrar tecla pressionada
            lcd595_write(2, coluna, &mostra[0]);
            coluna++;
        }

        if(qdig == 0) // Display vazio
        {
            lcd595_write(2,0, "[ ] [ ] [ ] [ ]");
        }
        if(qdig == 1) // Mostrar * para o primeiro dígito
        {
            sprintf(&mostra[0], "[ ] [ ] [ ] [*]");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 2) // Mostrar * para o segundo dígito
        {
            sprintf(&mostra[0], "[ ] [ ] [] []");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 3)  // Mostrar * para o terceiro dígito
        {
            sprintf(&mostra[0], "[ ] [] [] [*]");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 4) // Mostrar * para o quarto dígito
        {
            sprintf(&mostra[0], "[] [] [] []");
            lcd595_write(2,0, &mostra[0]);

            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if(qdig == 4)
        {
            if(numero1 == 1408) // Se a senha digitada for correta
            {
                int interrompe = 10;

                hcf_adc_ler(&adcvalor);
            
                while(adcvalor <= 2800 && interrompe > 0)
                {
                hcf_adc_ler(&adcvalor);

                lcd595_clear();
                lcd595_write(1,0, "COFRE ABERTO!");

                rotacionar_DRV(1, 11, saidas); // Rotacionar motor para abrir o cofre
                
                interrompe--;
                }
                
                if(adcvalor == 2800 || interrompe == 0)
                {
                    vTaskDelay(5000 / portTICK_PERIOD_MS);

                     hcf_adc_ler(&adcvalor);

                    while(adcvalor >= 350 && interrompe < 11)
                    {  
                        hcf_adc_ler(&adcvalor);

                        

                        lcd595_clear();
                        lcd595_write(1,0, "COFRE FECHANDO!");

                        rotacionar_DRV(0, 11, saidas); // Rotacionar motor para fechar o cofre

                        interrompe++;
                    }
                qdig = 0; // Reseta quantidade de dígitos
                numero1 = 0; // Reseta número digitado
                }
            }

        }

            else // Se a senha digitada for incorreta
            {
                lcd595_clear();
                lcd595_write(1,0, "SENHA ERRADA!");
              
                erros = erros + 1;  // Incrementa contador de erros
                qdig = 0; // Reseta quantidade de dígitos
                numero1 = 0; // Reseta número digitado
            }


            if(erros ==3)
            {
                lcd595_clear();
                lcd595_write(2,1, "TENTE DNV");
                vTaskDelay(500 / portTICK_PERIOD_MS); // Delay para exibir mensagem
            }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS); 
        }
           

       
     
        
        vTaskDelay(100 / portTICK_PERIOD_MS);

    


    // caso erro no programa, desliga o módulo ADC
    hcf_adc_limpar(); 
}
        
