// =====================================================================
// Definición manual de los registros en memoria para el STM32F103
// =====================================================================

// Registros de control del sistema (Relojes)
#define RCC_CR        (*((volatile unsigned int *)0x40021000))
#define RCC_CFGR      (*((volatile unsigned int *)0x40021004))
#define RCC_APB2ENR   (*((volatile unsigned int *)0x40021018))

// Registros del Puerto A
#define GPIOA_CRL     (*((volatile unsigned int *)0x40010800))
#define GPIOA_IDR     (*((volatile unsigned int *)0x40010808))
#define GPIOA_ODR     (*((volatile unsigned int *)0x4001080C))

// Macro para retardo antirrebote (evita usar funciones tipo void delay)
#define DELAY_ANTIRREBOTE for(i = 0; i < 48000; i++) {}

int main(void) {
    volatile unsigned int i; // Variable para el contador de la macro de retardo
    unsigned char conteo_pulsos = 0;

    // =================================================================
    // 1. CONFIGURACIÓN DEL RELOJ A 16 MHz (Aproximación a 18MHz)
    // =================================================================
    RCC_CR |= 0x00000001;          // Encender el reloj interno HSI (8 MHz)
    while(!(RCC_CR & 0x00000002)); // Esperar a que el HSI esté listo

    // Configurar PLL: Fuente HSI/2 (4 MHz), Multiplicador x4 
    RCC_CFGR &= ~(0x003F0000);     // Limpiar configuración del PLL
    RCC_CFGR |=  (0x00080000);     // Multiplicador x4 -> 4MHz * 4 = 16 MHz

    RCC_CR |= 0x01000000;          // Encender el PLL
    while(!(RCC_CR & 0x02000000)); // Esperar a que el PLL esté listo

    // Seleccionar el PLL como la fuente principal del sistema
    RCC_CFGR &= ~(0x00000003);     // Limpiar bits de selección
    RCC_CFGR |= 0x00000002;        // Seleccionar PLL
    while((RCC_CFGR & 0x0000000C) != 0x00000008); // Confirmar cambio

    // =================================================================
    // 2. CONFIGURACIÓN DE LOS PUERTOS
    // =================================================================
    RCC_APB2ENR |= 0x00000004;     // Habilitar reloj para el Puerto A

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

    // =================================================================
    // 3. BUCLE PRINCIPAL
    // =================================================================
    while(1) {
        // Leer estado de PA3. Con pull-up, al apretar lee 0.
        if((GPIOA_IDR & (1 << 3)) == 0) { 
            
            DELAY_ANTIRREBOTE; // Retardo al detectar pulsación
            
            // Confirmar que sigue presionado después del rebote
            if((GPIOA_IDR & (1 << 3)) == 0) { 
                
                // Quedarse aquí hasta que el interruptor se suelte
                while((GPIOA_IDR & (1 << 3)) == 0); 
                
                DELAY_ANTIRREBOTE; // Retardo al soltar
                
                conteo_pulsos++; // Se completó un ciclo (apretar y soltar)

                // Al llegar a 2 pulsos, conmutar los LEDs
                if(conteo_pulsos >= 2) {
                    conteo_pulsos = 0; // Reiniciar contador
                    
                    // Invertir el estado de los LEDs (XOR)
                    GPIOA_ODR ^= (1 << 4); 
                    GPIOA_ODR ^= (1 << 6); 
                }
            }
        }
    }
    
    return 0;
}
