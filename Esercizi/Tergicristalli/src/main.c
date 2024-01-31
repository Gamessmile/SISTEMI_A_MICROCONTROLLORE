/*
 * main.c
 *
 *  Created on: 31 gen 2024
 *      Author: Games
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stm32_unict_lib.h"

enum{
    disattivato, // 0
    s_p, //Sensore di Pioggia, 1
    temporizzata, // 2
	continua, // 3
	confing, // 4
};

// IL CODICE CHE SEGUE VA COMPILATO CON OPENSTM O DARA' PROBLEMI

//Leggenda variabili globali
int stato = disattivato;
int tempo = 0;
int variabile = 0; // Variabile utilizzata per l'input della rotella nelle fasi 1-2-3
int gocce_cadute = 0;
double velocita_ts1 = 2.5; // Variabile modificabile dal confing (T)
double velocita_ts2 = 1; // Variabile modificabile dal confing (T)
int animazione = 0; // Variabile "BOOLEANA" che indica se l'animazione é in corso o meno
int stato_animazione = 0; // Variabile utilizzata per indicare a quale step si trovi l'animazione

void getstring(char * s, int maxlen){
	int i = 0;
	for (;;) {
		char c = readchar();
		if (c == 13) {
			printf("\n");
			s[i] = 0;
			return;
		}
		else if (c == 8) {
			if (i > 0) {
				--i;
				__io_putchar(8); // BS
				__io_putchar(' '); // SPAZIO
				__io_putchar(8); // BS
			}
		}
		else if (c >= 32) { // il carattere appartiene al set stampabile
			if (i < maxlen) {
				__io_putchar(c); // echo del carattere appena inserito
				// inserisci il carattere nella stringa
				s[i] = c;
				i++;
			}
		}
	}
}

void setup(){

    DISPLAY_init();
    GPIO_init(GPIOB);
    GPIO_init(GPIOC);

    ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
    ADC_channel_config(ADC1,GPIOC,0,10);
    ADC_on(ADC1);
    ADC_sample_channel(ADC1,10);


    GPIO_config_output(GPIOB, 0);
    GPIO_config_output(GPIOC, 2);
    GPIO_config_output(GPIOC, 3);

    GPIO_config_input(GPIOB,10); //X
    GPIO_config_EXTI(GPIOB,EXTI10);
    EXTI_enable(EXTI10, FALLING_EDGE);

    GPIO_config_input(GPIOB,4); //Y
    GPIO_config_EXTI(GPIOB,EXTI4);
    EXTI_enable(EXTI4,FALLING_EDGE);

    GPIO_config_input(GPIOB,5); //Z
    GPIO_config_EXTI(GPIOB,EXTI5);
    EXTI_enable(EXTI5,FALLING_EDGE);

    GPIO_config_input(GPIOB,6); //T
    GPIO_config_EXTI(GPIOB,EXTI6);
    EXTI_enable(EXTI6,FALLING_EDGE);

    TIM_init(TIM2);
    TIM_config_timebase(TIM2,8400,5000); // 0,5 s
    TIM_enable_irq(TIM2,IRQ_UPDATE);
    TIM_set(TIM2,0);
    TIM_on(TIM2);

    CONSOLE_init();
}

void EXTI15_10_IRQHandler(void){
    //X
    if(EXTI_isset(EXTI10)){
        if(stato!=s_p && stato!=confing){
            tempo = 0;
            stato=s_p;
            GPIO_write(GPIOB,0,1); // RED ON
            GPIO_write(GPIOC,2,0); // YELLOW OFF
            GPIO_write(GPIOC,3,0); //GREEN OFF

        }
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void){
    //Y
    if(EXTI_isset(EXTI4)){
        if(stato!=temporizzata && stato!=confing){
        	tempo = 0;
            stato=temporizzata;
            GPIO_write(GPIOB, 0, 0); // RED OFF
            GPIO_write(GPIOC, 2, 1); // YELLOW ON
            GPIO_write(GPIOC, 3, 0); //GREEN OFF

        }
        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void){
    //Z
    if(EXTI_isset(EXTI5)){
        if(stato!=continua && stato!=confing){
        	tempo = 0;
            stato=continua;
            GPIO_write(GPIOB, 0, 0); // RED OFF
            GPIO_write(GPIOC, 2, 0); // YELLOW OFF
            GPIO_write(GPIOC, 3, 1); //GREEN ON

        }
        EXTI_clear(EXTI5);
    }

    //T
    if(EXTI_isset(EXTI6)){
    	if(stato!=confing){
    		tempo = 0;
    		stato = confing;
            GPIO_write(GPIOB, 0, 1); // RED ON
            GPIO_write(GPIOC, 2, 1); // YELLOW ON
            GPIO_write(GPIOC, 3, 1); //GREEN ON
    	}
        EXTI_clear(EXTI6);
    }
}

void TIM2_IRQHandler(void){
    if(TIM_update_check(TIM2)){

        switch (stato)
        {
        case disattivato:
            break;
        case s_p:
            DISPLAY_puts(0, "    ");
            tempo++;
            if(tempo!= 0 && (tempo%2)==0){ // Goccia aggiuta ogni secondo
                gocce_cadute += variabile;
            }
            if(animazione==1){
                stato_animazione++;
            }
            break;
        case temporizzata:

            tempo++;
            if(animazione==1){
                stato_animazione++;
            }

            break;
        case continua:
        	tempo++;
        	if(animazione==1){
        		stato_animazione++;
        	}
        	break;
        }
        TIM_update_clear(TIM2);
    }
}

void Animazione1(){ //Animazione della spazzolata dipendente da ts1


	if(velocita_ts1==2){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "-   ");

            break;
        case 2:
            DISPLAY_puts(0, " -- ");

            break;
        case 3:
            DISPLAY_puts(0, "  --");

            break;
        case 4:
        	DISPLAY_puts(0, "   -");
        	break;
        case 5:
        	DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
            break;
        }
    }
    if(velocita_ts1==2.5){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "-   ");

            break;
        case 2:
            DISPLAY_puts(0, " -  ");

            break;
        case 3:
            DISPLAY_puts(0, " -- ");

            break;
        case 4:
            DISPLAY_puts(0, "  - ");
            break;
        case 5:
        	DISPLAY_puts(0, "   -");
            break;
        case 6:
            DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
        	break;

        }
    }
    if(velocita_ts1==3){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "-   ");

            break;
        case 2:
            DISPLAY_puts(0, "--  ");

            break;
        case 3:
            DISPLAY_puts(0, " -- ");

            break;
        case 4:
            DISPLAY_puts(0, " ---");
            break;
        case 5:
            DISPLAY_puts(0, "  --");
            break;
        case 6:
        	DISPLAY_puts(0, "   -");
            break;
        case 7:
            DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
        	break;
        }
    }
}

void Animazione2(){ //Animazione della spazzolata dipendente da ts2


	if(velocita_ts2==0.5){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "----");
            break;
        case 2:
            DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
            break;
        }
    }
    if(velocita_ts2==1){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "--  ");

            break;
        case 2:
            DISPLAY_puts(0, "  --");

            break;
        case 3:
            DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
            break;
        }
    }
    if(velocita_ts2==1.5){
        switch (stato_animazione){
        case 1:
            DISPLAY_puts(0, "-   ");

            break;
        case 2:
            DISPLAY_puts(0, "--  ");

            break;
        case 3:
            DISPLAY_puts(0, "  --");

            break;
        case 4:
            DISPLAY_puts(0, "    ");
            stato_animazione=0;
            animazione = 0;
            break;
        }
    }
}

void loop(){
    switch (stato){
		case disattivato:
			DISPLAY_puts(0," DIS");
			break;

		case s_p:
			if(animazione!=1){
				ADC_channel_config(ADC1,GPIOC,1,11);
				ADC_on(ADC1);
				ADC_sample_channel(ADC1,11);
				ADC_start(ADC1);

				int valore = ADC_read(ADC1);
				variabile = valore*5.0/255+0; // Range 0-5

				if(gocce_cadute>=10){
					animazione = 1;
					gocce_cadute=0;
				}
			}else{
				Animazione1();
			}
			break;

		case temporizzata:
			if(animazione!=1){
				ADC_channel_config(ADC1,GPIOC,0,10);
				ADC_on(ADC1);
				ADC_sample_channel(ADC1,10);
				ADC_start(ADC1);

				int valore = ADC_read(ADC1);
				variabile = valore*5.0/255+0; // Range 0-5

				if(tempo >= variabile*2){ // *2 poiché il timer é a 0,5
					animazione = 1;
					tempo=0;
				}

			}
			else{
				Animazione1();
			}
			break;

		case continua:
			if(tempo >= velocita_ts2 && animazione != 1){ // Modo casuale per mettere una pausa fra una spazzata e un'altra
				animazione = 1;
				tempo = 0;
			}
			if(animazione == 1){
				Animazione2();
			}
			break;

		case confing:
			DISPLAY_puts(0,"CONF");
			printf("Benvenut* nella modalitá CONFING, una modalitá che ti permette di modificare i seguenti valori:\n"
					"^TS1 -= Durata di una spazzolata in 1 e 2 , il valore deve essere compreso fra {2, 2.5, 3}\n"
					"^TS2 -= Durata spazzolata in 3, il valore deve essere compreso fra {0.5, 1, 1.5}\n\n"
					"Per poter modificare i valori indica TS1 o TS2 per modificare il valore rispettivo, o\n"
					"Esempio: TS1 2; verrá impostato TS2  a 2 secondi.\n"
					"Invia QUIT per uscire dalla modalitá CONFING e riprendere normalmente. {OBBLIGATORIO PER USCIRE}.\n\n");
			fflush(stdout);
			char s[20];
			getstring(s,19);
			char *token = strtok(s, " "); //Prende e mette in token tutto quello prima dello spazio in s
			if(strcmp(token, "TS1")==0){
				token = strtok(NULL, " "); //Prende il restante
				if(strcmp(token, "2") == 0 || strcmp(token, "2.5") == 0 || strcmp(token, "3") == 0 ){
					velocita_ts1 = strtod(token,&token); //String to double
				}
				else{
					printf("Cio' che ha inviato non rispetta le condizioni date, il valore DEVE essere compreso tra 2, 2.5, 3\n");
				}
			}
			else if(strcmp(token, "TS2")==0){
				token = strtok(NULL, " "); //Prende il restante
				if(strcmp(token, "0.5") == 0 || strcmp(token, "1") == 0 || strcmp(token, "1.5") == 0 ){
					velocita_ts2 = strtod(token,&token); //String to Double
				}
				else{
					printf("Cio' che ha inviato non rispetta le condizioni date,  il valore DEVE essere compreso tra 0.5, 1, 1.5\n");
				}
			}
			else if(strcmp(token, "QUIT")==0){
				stato = disattivato;
				DISPLAY_puts(0, "    ");
	            GPIO_write(GPIOB, 0, 0); // RED OFF
	            GPIO_write(GPIOC, 2, 0); // YELLOW OFF
	            GPIO_write(GPIOC, 3, 0); //GREEN OFF
			}
			else{
				printf("Cio' che ha inviato non rispetta le condizioni date, va scrito TS1 #valore OPPURE TS2 #valore OPPURE QUIT\n");
			}
			break;
    }
}

int main(){
    //ClockConfig(); //Il codice non sembra funzionare correttamente se compilato su visual
    setup();
    for(;;){
        loop();
    }
}
