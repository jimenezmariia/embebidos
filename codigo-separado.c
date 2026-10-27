// =====================================================================
// Definición manual de los registros en memoria para el STM32F103
// =====================================================================

// Registros de control de reloj
#define RCC_CR        (*((volatile unsigned int *)0x40021000))
#define RCC_CFGR      (*((volatile unsigned int *)0x40021004))
#define RCC_APB2ENR   (*((volatile unsigned int *)0x40021018))

// Registros del Puerto A
#define GPIOA_CRL     (*((volatile unsigned int *)0x40010800))
#define GPIOA_IDR     (*((volatile unsigned int *)0x40010808))
#define GPIOA_ODR     (*((volatile unsigned int *)0x4001080C))

// Macro antirrebote
#define DELAY for(i = 0; i < 40000; i++) {}

int main(void) {
    volatile unsigned int i; 
    unsigned char pulsos = 0;

    // =================================================================
    // 1. Configuración del Reloj a 16 MHz
    // =================================================================
    RCC_CFGR = 0x00080000;         // Multiplicador x4
    RCC_CR |= 0x01000000;          // Encender PLL
    while(!(RCC_CR & 0x02000000)); // Esperar a que el PLL estabilice
    RCC_CFGR |= 0x00000002;        // Seleccionar PLL como reloj principal

    // =================================================================
    // 2. Configuración de Puertos
    // =================================================================
    RCC_APB2ENR |= 0x04;           // Habilitar reloj del Puerto A
    
    // Limpiar la configuración de cada pin por separado
    GPIOA_CRL &= ~(0x0000F000); // Limpiar PA3 
    GPIOA_CRL &= ~(0x000F0000); // Limpiar PA4 
    GPIOA_CRL &= ~(0x0F000000); // Limpiar PA6 
    
    // Configurar cada pin por separado
    GPIOA_CRL |= 0x00008000;    // Configurar PA3 como entrada con Pull (valor 8)
    GPIOA_CRL |= 0x00020000;    // Configurar PA4 como salida Push-Pull (valor 2)
    GPIOA_CRL |= 0x02000000;    // Configurar PA6 como salida Push-Pull (valor 2)
    
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
