#include <stdint.h>

// =====================================================================
// Definición manual de los registros en memoria para el STM32F103
// =====================================================================

// Registros de control del sistema (Relojes)
#define RCC_CR        (*((volatile uint32_t *)0x40021000)) // Control de reloj
#define RCC_CFGR      (*((volatile uint32_t *)0x40021004)) // Configuración de reloj
#define RCC_APB2ENR   (*((volatile uint32_t *)0x40021018)) // Reloj de periféricos APB2

// Registros del Puerto A
#define GPIOA_CRL     (*((volatile uint32_t *)0x40010800)) // Configuración baja (Pines 0-7)
#define GPIOA_IDR     (*((volatile uint32_t *)0x40010808)) // Datos de entrada
#define GPIOA_ODR     (*((volatile uint32_t *)0x4001080C)) // Datos de salida

// Función para generar un retardo simple (antirrebote)
void delay_ms(uint32_t ms) {
    // Al correr a 16 MHz, se calibra empíricamente el ciclo for
    for(volatile uint32_t i = 0; i < ms * 1600; i++) {
        // No hacer nada, solo gastar ciclos
    }
}

// Función para configurar el reloj a 16 MHz (Lo más cercano a 18 MHz)
void conf_clk(void) {
    RCC_CR |= 0x00000001; // Encender el reloj interno HSI (8 MHz)
    while(!(RCC_CR & 0x00000002)); // Esperar a que el HSI esté listo

    // Configurar PLL: Fuente HSI/2 (4 MHz), Multiplicador x4 (Bits 21:18 = 0010)
    RCC_CFGR &= ~(0x003F0000); // Limpiar configuración del PLL
    RCC_CFGR |=  (0x00080000); // Multiplicador x4 -> 4MHz * 4 = 16 MHz

    RCC_CR |= 0x01000000; // Encender el PLL
    while(!(RCC_CR & 0x02000000)); // Esperar a que el PLL esté listo

    // Seleccionar el PLL como la fuente principal del sistema
    RCC_CFGR &= ~(0x00000003); // Limpiar bits de selección
    RCC_CFGR |= 0x00000002;    // Seleccionar PLL
    while((RCC_CFGR & 0x0000000C) != 0x00000008); // Confirmar que se aplica el PLL
}

int main(void) {
    conf_clk(); // Inicializar reloj a 16 MHz

    // 1. Habilitar el reloj para el Puerto A (Bit 2)
    RCC_APB2ENR |= 0x00000004;
    
    // 2. Configurar pines del Puerto A (PA3, PA4, PA6)
    // Limpiamos los bits de configuración para los pines 3, 4 y 6
    GPIOA_CRL &= ~(0x0F0FF000); 
    
    // PA4 y PA6 como salidas Push-Pull a 2MHz (CNF=00, MODE=10 -> 0x2)
    // PA3 como entrada con Pull-up/Pull-down (CNF=10, MODE=00 -> 0x8)
    GPIOA_CRL |= 0x02028000; 
    
    // Activar resistencia Pull-up para PA3
    GPIOA_ODR |= (1 << 3); 

    // Estado inicial: Complementario (PA4 Encendido, PA6 Apagado)
    GPIOA_ODR |=  (1 << 4); 
    GPIOA_ODR &= ~(1 << 6); 

    uint8_t conteo_pulsos = 0;

    while(1) {
        // Leer estado de PA3. Con pull-up, al apretar lee 0.
        if((GPIOA_IDR & (1 << 3)) == 0) { 
            delay_ms(30); // Antirrebote al apretar
            
            // Confirmar que sigue presionado
            if((GPIOA_IDR & (1 << 3)) == 0) { 
                
                // Esperar a que el interruptor se suelte
                while((GPIOA_IDR & (1 << 3)) == 0); 
                delay_ms(30); // Antirrebote al soltar
                
                conteo_pulsos++; // Se completó un ciclo (apretar y soltar)

                // Al llegar a 2 pulsos, conmutar los LEDs
                if(conteo_pulsos >= 2) {
                    conteo_pulsos = 0; // Reiniciar contador
                    
                    // Invertir el estado de los LEDs para mantenerlos complementarios
                    // Operador XOR (^) invierte el bit actual
                    GPIOA_ODR ^= (1 << 4); 
                    GPIOA_ODR ^= (1 << 6); 
                }
            }
        }
    }
    
    return 0;
}
