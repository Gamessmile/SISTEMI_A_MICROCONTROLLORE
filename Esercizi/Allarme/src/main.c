#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stm32_unict_lib.h"

enum{
    DISATTIVATO,
    ATTIVAZIONE,
    PREATTIVAZIONE,
    ATTIVATO,
    DISATTIVAZIONE,
    PRE_ALLARME,
    ALLARME,
    CONFIGURAZIONE,
};

/* VARIABILI GLOBALI */
int stato = DISATTIVATO;
char password[999] = "1234";
int tempo_uscita = 50; // Variabile inerente al tempo di uscita nella fase di pre allarme
int time = 0; // Valore utilizzato per la gestione dei tempi
int try = 0; //tentativi della password
int i=0;//Ultimo accesso alla stringa usata per prendere la password (preallarme, allarme), messo fuori per evitare venga resettata a 0 ove non serve
int invio = 0; // Utilizzata come variabile booleana per verificare o meno che la password scritta sia inviata (preallarme, allarme), e quindi confrontarla
char c[999];


void setup(){

    DISPLAY_init();

    GPIO_init(GPIOB);
    GPIO_init(GPIOC);


    GPIO_config_input(GPIOB,10);
    GPIO_config_EXTI(GPIOB, EXTI10);
    EXTI_enable(EXTI10, FALLING_EDGE);

    GPIO_config_input(GPIOB,4);
    GPIO_config_EXTI(GPIOB, EXTI4);
    EXTI_enable(EXTI4, FALLING_EDGE);

    GPIO_config_input(GPIOB,5);
    GPIO_config_EXTI(GPIOB, EXTI5);
    EXTI_enable(EXTI5, FALLING_EDGE);

    GPIO_config_output(GPIOB,0);
    GPIO_config_output(GPIOC,2);
    GPIO_config_output(GPIOC,3);
    

    TIM_init(TIM2);
    TIM_config_timebase(TIM2, 8400, 1000); //100 MS
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_set(TIM2, 0); // resets the counter
    TIM_on(TIM2); // starts the timer
    
    CONSOLE_init();
}

int getstring(char * s, int maxlen)
{
	int i = 0;
	for (;;) {
		char c = readchar();
		if (c == 13) {
			printf("\n");
			s[i] = 0;
			return 1;
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
    return 0;
}




int password_verify(){
    char s[999];
    printf("Inserisci la password: ");  
    fflush(stdout);
    getstring(s, 998);

    if (strcmp(s, password) == 0){
        printf("Password Corretta\n");
        fflush(stdout);
        return 1;
    }
    return 0;
}

void EXTI15_10_IRQHandler(void){
    // X
    if(EXTI_isset(EXTI10)){
        stato = ATTIVAZIONE;
    
        EXTI_clear(EXTI10);
    }
}

void EXTI4_IRQHandler(void){
    // Y
    if(EXTI_isset(EXTI4)){
        GPIO_write(GPIOB,0,1);
        stato = DISATTIVAZIONE;

        EXTI_clear(EXTI4);
    }
}

void EXTI9_5_IRQHandler(void){
    // Z
    if(EXTI_isset(EXTI5)){
        if(stato == ATTIVATO){
            GPIO_write(GPIOB,0,1);
            printf("⚠️  INTRUSO RILEVATO ⚠️\nHai 20 secondi e 3 tentativi per poter inserire la password ed evitare che l'allarme suoni\n\n");
            fflush(stdout);
            stato = PRE_ALLARME;
            
        }
        EXTI_clear(EXTI5);
    }
}

void TIM2_IRQHandler(void){
    if(TIM_update_check(TIM2)){
        
        switch (stato)
        {
        case DISATTIVATO:
            DISPLAY_puts(0,"DIS");
            GPIO_write(GPIOC,3,1);
            if (kbhit()){
                char X = readchar();
                //getstring(k, 1);
                if(X == '!'){
                    printf("Setup Avviato\n");
                    fflush(stdout);
                    stato = CONFIGURAZIONE;
                }
            }           
            break;
  
        case CONFIGURAZIONE:
            DISPLAY_puts(0,"    ");
            char p[999];
            printf( "• cambio della password, comando PASSWORD <xyz>\n"
                    "• impostazione del “tempo di uscita”, comando OUT-TIME <valore_in_ms>\n"
                    "• impostazione del “tempo di ingresso”, comando IN-TIME <valore_in_ms>\n"
                    "• uscita dal “setup”, comando EXIT\n\n");

            fflush(stdout);
            getstring(p,998);
            char * token = strtok(p, " ");
            if(strcmp(token,"PASSWORD")==0){
                token = strtok(NULL, " ");
                strcpy(password,token);
                printf("Password cambiata\n\n");
            }
            if(strcmp(token,"OUT-TIME")==0){
                token = strtok(NULL, " ");
                tempo_uscita = atoi(token);
                printf("Tempo di uscita modificato\n\n");
            }
            if(strcmp(token,"IN-TIME")==0){
                printf("Non é stato indicato a quale tempo si riferisca quello di entrata, si suppone quello per inserire la password in preallarme?\n");
            }
            if(strcmp(token,"EXIT")==0){
                printf("Uscita ...\n\n");
                stato = DISATTIVATO;
            }
            memset(&p,0,strlen(p));
            break;
  
        case ATTIVAZIONE:
            DISPLAY_puts(0,"    ");
            GPIO_write(GPIOC,3,0);
            
            if(password_verify()==1){
                stato = PREATTIVAZIONE;
            }

            break;
        case PREATTIVAZIONE:
            time++;
            if((time % 2)==0){ //Si attiva ogni 200 ms
                GPIO_toggle(GPIOC,3);
            }
            if(time == tempo_uscita*10){ //Si attiva una volta arrivati a  default: 5000ms = 5 secondi
                GPIO_write(GPIOC,3,0);
                time = 0;
                stato = ATTIVATO;
            }
            break;

        case ATTIVATO:
            GPIO_write(GPIOB,0,1);
            break;

        case DISATTIVAZIONE:
            GPIO_write(GPIOB,0,0);
            

            if(password_verify()==1){
                stato = DISATTIVATO;
            }else{
                try++;
            }
            if(try==3){
                stato = ALLARME;
            }
            break;
        
        case PRE_ALLARME:
            time++;
            GPIO_write(GPIOB,0,0);
            GPIO_write(GPIOC,2,1);
            char remaining_time[5];
            sprintf(remaining_time, "%4d", (20-(time/10)));
            DISPLAY_puts(0, remaining_time);
                
            if(kbhit()){ // Gestione della getstring non bloccante
                char h = readchar();
                if(h==13){
                    printf("\n");
			        c[i] = 0;
                    invio = 1;
                }
                else if (h == 8) {
			        if (i > 0) {
                        --i;
                        __io_putchar(8); // BS
                        __io_putchar(' '); // SPAZIO
                        __io_putchar(8); // BS
			        }
                }
                else if (h >= 32 && h <= 126) { // il carattere appartiene al set stampabile
                    if (i < 998) {
                        __io_putchar(h); // echo del carattere appena inserito
                        // inserisci il carattere nella stringa
                        c[i] = h;
                        i++;
                    }
                }
            }

            if(invio==1){
                invio = 0;
                if(strcmp(c,password)==0){
                    GPIO_write(GPIOC,2,0);
                    printf("Passowrd Corretta\n");
                    fflush(stdout);
                    stato = DISATTIVATO;
                    time = 0;
                    memset(&c,0,strlen(c));
                    i = 0;c[i] = 0;
                    
                }
                else{
                    try++;
                    printf("%s Passowrd Errata, hai altri %d tentativi\n", c,(3-try));
                    fflush(stdout);
                    memset(&c,0,strlen(c));
                    i = 0;
                    c[i] = 0;
                }
            }
            if(try==3){
                memset(&c,0,strlen(c));
                i = 0;c[i] = 0;
                GPIO_write(GPIOC,2,0);
                time = 0;
                
                try = 0;
                printf("ALLARME\nper uscire da questo stato devi inserire la password corretta.\n\n");
                fflush(stdout);
                stato = ALLARME;
            }
            if(time == 200){ // 20.000 ms ovvero 20 secondi
                memset(&c,0,strlen(c));
                i = 0;c[i] = 0;
                GPIO_write(GPIOC,2,0);
                printf("Tempo Scaduto\nALLARME\nper uscire da questo stato devi inserire la password corretta.\n\n");
                fflush(stdout);
                time = 0;
                
                stato = ALLARME;
            }  
            break;
        
        case ALLARME:
            time++;    
            if((time % 2)==0){ //Si attiva ogni 200 ms
                GPIO_toggle(GPIOB,0);
            }
            if(kbhit() && invio == 0){ // Gestione della getstring non bloccante
                char h = readchar();
                if(h==13){
                    printf("\n");
			        c[i] = 0;
                    invio = 1;
                }
                else if (h == 8) {
			        if (i > 0) {
                        --i;
                        __io_putchar(8); // BS
                        __io_putchar(' '); // SPAZIO
                        __io_putchar(8); // BS
			        }
                }
                else if (h >= 32 && h <= 126) { // il carattere appartiene al set stampabile
                    if (i < 998) {
                        __io_putchar(h); // echo del carattere appena inserito
                        // inserisci il carattere nella stringa
                        c[i] = h;
                        i++;
                    }
                }
            }

           if(invio==1){
                invio = 0;
                if(strcmp(c,password)==0){
                    GPIO_write(GPIOC,2,0);
                    printf("Passowrd Corretta\n");
                    fflush(stdout);
                    stato = DISATTIVATO;
                    time = 0;
                    memset(&c,0,strlen(c));
                    i = 0;c[i] = 0;
                    
                }
                else{
                    printf("%s Passowrd Errata\n", c);
                    fflush(stdout);
                    memset(&c,0,strlen(c));
                    i = 0;
                    c[i] = 0;
                    
                }
            }
            break;
        }



        TIM_update_clear(TIM2);
    }
}

void loop(){

    switch (stato)
    {
    case CONFIGURAZIONE:
        DISPLAY_puts(0,"    ");
        break;
    case DISATTIVATO:
        DISPLAY_puts(0,"dis ");
        break;
    case DISATTIVAZIONE:
        DISPLAY_puts(0,"    ");
        break;
    case ATTIVAZIONE:
        DISPLAY_puts(0,"    ");
        break;
    case PRE_ALLARME:
        //Viene visualizzato il timer di 20 secondi
        break;
    case ALLARME:
        DISPLAY_puts(0,"ALL ");
        break;
    
    }

}

int main(){
    ClockConfig();
    setup();
    for(;;){
        loop();
    }    
}