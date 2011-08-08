#include <16F628A.h>

#FUSES NOWDT				//No Watch Dog Timer
#FUSES XT          	//Crystal osc <= 4mhz for PCM/PCH , 3mhz to 10 mhz for PCD
#FUSES NOPUT        //No Power Up Timer
#FUSES NOPROTECT    //Code not protected from reading

#use delay(clock = 4MHz)
#use fast_io(a)
//#use fast_io(b)

#include "lcd.c"

#define tc 7	//Valor en el que empezará a contar el timer. Se hace así para que junto con el prescaler y el registro de tiempo
							//asociado al timer se pueda contar 1 segundo

//Definición e inicialización de variables globales a utilizar

int segundos = 0;
int minutos = 0;
int aux = 0;
short int estado = 0; //Indica si se debe decrementar el tiempo o no
short int zumbador = 0; //Indica si se debe encender o no el zumbador
int cont_seg = 0; //Para determinar cuando pasó un segundo
int cont_tecla = 0; //Para determinar cuando se debe chequear si se apretó una tecla
int cont_zumbador = 0; //Para probocar los pitidos a través del zumbador
int aux_zum = 0; //Indica cuantas veces ha emitido un sonido el zumbador

//Definición de funciones a utilizar

//Función para el manejo del incremento, decremento del tiempo y puesta en marcha o parada del sistema

void cheq_tecla(void)
	{
	if(input(PIN_A1) == 1 && estado == 0) //Algoritmo para incrementar de a 5 minutos (0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60)
		{
		//estado = 0; //Detengo el reloj
		segundos = 0;
		aux = minutos;
		aux = aux/5;
		minutos = (int)aux * 5 + 5;
		if(minutos > 60)
			minutos = 0;
		//estado = 1; //Vuelvo a poner en marcha el reloj
		}
	
	if(input(PIN_A0) == 1 && estado == 0) //Algoritmo para decrementar de a 5 minutos (0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60)
		{
		segundos = 0;
		if(minutos > 0)
			{
			aux = minutos - 1;
			aux = aux/5;
			minutos = (int)aux * 5;
			}
				
		//Solo para realizar pruebas del zumbador
		
		//minutos = 0;
		//segundos = 3;
		}	
	
	if(input(PIN_A2) == 1) //Para arrancar y parar el sistema. Tiene efecto solo cuando se haya seleccionado la temporización
		{
		if(minutos > 0 || segundos > 0)
			estado = !estado; //Arranque o parada del timer
		}
	}

//Rutina de manejo de tiempos

void tiempo(void)
	{
	if(estado == 1) //Indica que se debe seguir decrementando
		{
		if(segundos == 0)
			{
			if(minutos > 0)
				{
				minutos--;
				segundos = 59;
				}
			}
			else
				segundos--;
			if(minutos == 0 && segundos ==0)
				{
				estado = 0; //Terminó la cuenta, por ende no se entrará más a este lazo
				cont_zumbador = 0; //Reinicio el contador para la temporación del zumbador
				output_bit(pin_a3, 0); //Enciendo el zumbador
				zumbador = 1; //Indico que se debe encender el zumbador
				}
		}	
	}	
		
//Rutina de refresco de display (cada 1 segundo se lo refresca)	

void ref_display(void)
	{
	lcd_gotoxy(1,1);
	printf(LCD_PUTC, "MT GIOSER\n");
	printf(LCD_PUTC, "Tiempo: %02u:%02u ", minutos, segundos);
	}

//Interrupción del Timer0. Interrumpe cada 8[mseg], por lo que se deben contar 125 interrupciones para
//contar 1[seg] aprox.

#int_TIMER0
void  TIMER0_isr(void)
	{
	set_timer0(tc); //Reinicio el timer
	cont_tecla++; //Incremento el registro asociado a la captura de teclas
	cont_seg++; //Incremento el registro que cuenta las veces que interrumpe el timer
	cont_zumbador++; //Incremento el registro utilizado para emitir los pitidos por el zumbador
	
	//Cada 200[mseg] (aprox) verifico si hay alguna tecla apretada
	
	if(cont_tecla == 35)
		{
		cont_tecla = 0;
		cheq_tecla();
		}
	
	//Cada 500[mseg] aproximadamente enciendo y apago el zumbador si es que se terminó el tiempo
			
	if(cont_zumbador == 65 && zumbador == 1)
		{
		if(aux_zum <= 3)
			{
			aux_zum++;
			cont_zumbador = 0;
			output_toggle(pin_a3);
			}
		else
			{
			aux_zum = 0;
			zumbador = 0; //Variable que indica que el zumbador debe quedar apaagdo.
			output_bit(pin_a3, 1); //Me aseguro de que el zumbador quede apagado
			}
		}
		
	//Cada 1 segundo incremento el registro asociado a los segundos y se regresca el display
	
	if (cont_seg == 125)
		{
		cont_seg = 0;
		tiempo();
		ref_display();
		}
	}

void main()
{
	//Configuración de los puertos
	
	set_tris_a(0b00000111);	//Los 3 pines más bajos se usan como entradas para los pulsadores y el cuarto es por donde entra
													//la corriente del led y del zumbador
		
	lcd_init();
	
	//Configuración e inicialización del timer. Se configura el timer para que comience a contar de 6 en adelante y el prescales
	//para que divida por 16. Esto logra que el timer interrumpa cada 4[mseg] y entonces si se cuentan 250 interrupciones
	//se habrá contado 1[seg] exactamente
	
	setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32);
	set_timer0(tc);
	
	//Habilitación de interrupcionesc. De momento solo se usa el timer0
	
	enable_interrupts(INT_TIMER0);
	enable_interrupts(GLOBAL);
	
	output_bit(pin_a3, 1);
	
	//Programa principal

	while(TRUE)
		{
		
		}
}
