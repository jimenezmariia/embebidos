// =====================================================================
// Definición manual de los registros en memoria para el STM32F103
// =====================================================================

// RCC_CR (Clock Control Register): Es el interruptor principal del oscilador. 
// Sirve para encender, estabilizar y apagar el reloj base interno.
#define RCC_CR        (*((volatile unsigned int *)0x40021000))

// RCC_CFGR (Clock Configuration Register): Es la caja de velocidades del reloj. 
// Aquí se ajusta qué tan rápido va a operar el sistema (multiplicador x4 para 16 MHz).
#define RCC_CFGR      (*((volatile unsigned int *)0x40021004))

// RCC_APB2ENR (APB2 Peripheral Clock Enable Register): Es el centro de carga eléctrica. 
// Se usa para habilitar la energía específicamente en el Puerto A.
#define RCC_APB2ENR   (*((volatile unsigned int *)0x40021018))

// GPIOA_CRL (GPIO Port A Configuration Register Low): Es el manual de instrucciones. 
// Le indica al chip si los pines (del 0 al 7) son entradas (botón) o salidas (LEDs).
#define GPIOA_CRL     (*((volatile unsigned int *)0x40010800))

// GPIOA_IDR (GPIO Port A Input Data Register): Son los sensores de lectura. 
// El micro revisa esta dirección para saber si el botón está apretado (0) o suelto (1).
#define GPIOA_IDR     (*((volatile unsigned int *)0x40010808))

// GPIOA_ODR (GPIO Port A Output Data Register): Es el interruptor de salida. 
// Escribir aquí hace que el micro envíe voltaje (1 enciende el LED, 0 lo apaga).
#define GPIOA_ODR     (*((volatile unsigned int *)0x4001080C))

// DELAY: Es una macro que pone al procesador a contar hasta 40,000 sin hacer nada. 
// Genera una pausa para darle tiempo al botón de dejar de vibrar (antirrebote).
#define DELAY for(i = 0; i < 40000; i++) {}

int main(void) {
    volatile unsigned int i; 
    unsigned char pulsos = 0;

    // =================================================================
    // 1. Configuración del Reloj a 16 MHz (HSI/2 * 4 multiplicador)
    // =================================================================
    RCC_CFGR = 0x00080000;         // Multiplicador x4
    RCC_CR |= 0x01000000;          // Encender PLL
    while(!(RCC_CR & 0x02000000)); // Esperar a que el PLL estabilice
    RCC_CFGR |= 0x00000002;        // Seleccionar PLL como reloj principal

    // =================================================================
    // 2. Configuración de Puertos
    // =================================================================
    RCC_APB2ENR |= 0x04;           // Habilitar reloj del Puerto A
    
    // Limpiar configuración de PA3, PA4 y PA6
    GPIOA_CRL &= 0xF0F00FFF;       
    
    // PA4, PA6 salidas Push-Pull (0x2). PA3 entrada con Pull (0x8)
    GPIOA_CRL |= 0x02028000;       
    
    GPIOA_ODR |= (1 << 3);         // Activar resistencia Pull-up en PA3
    GPIOA_ODR |= (1 << 4);         // Encender PA4 (Estado inicial)
    GPIOA_ODR &= ~(1 << 6);        // Apagar PA6 (Estado inicial complementario)

    // =================================================================
    // 3. Bucle Principal
    // =================================================================
    while(1) {
        if((GPIOA_IDR & (1 << 3)) == 0) { // Si detecta que se apretó el botón
            DELAY; // Antirrebote
            
            while((GPIOA_IDR & (1 << 3)) == 0); // Esperar a que se suelte
            DELAY; // Antirrebote
            
            pulsos++; // Cuenta 1 ciclo de apretar y soltar
            
            if(pulsos == 2) { // Si ya se completaron los 2 ciclos
                GPIOA_ODR ^= (1 << 4); // Invertir estado PA4
                GPIOA_ODR ^= (1 << 6); // Invertir estado PA6
                pulsos = 0;            // Reiniciar conteo
            }
        }
    }
}
