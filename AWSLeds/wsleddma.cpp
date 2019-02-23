
#include "mbed.h"
#include "cy_pdl.h"
#include "cycfg_pins.h"
#include "cycfg_dmas.h"
#include "cycfg_peripherals.h"
#include "wsleddma.h"

#define WS_ZOFFSET (1)
#define WS_ONE3  (0b110<<24)
#define WS_ZERO3 (0b100<<24)
#define WS_SPI_BIT_PER_BIT (3)
#define WS_COLOR_PER_PIXEL (3)
#define WS_BYTES_PER_PIXEL (WS_SPI_BIT_PER_BIT * WS_COLOR_PER_PIXEL)

#define WSDRIVER_SLEEP_TIME 33          //time in ms between LED update triggers

//config for left DMA interrupt
 cy_stc_sysint_t leftWS_DMA_INT_cfg =
    {
            .intrSrc = cpuss_interrupts_dw0_24_IRQn, 
            .intrPriority = 7UL                     
    };

//config for right DMA interrupt
 cy_stc_sysint_t rightWS_DMA_INT_cfg =
    {
            .intrSrc = cpuss_interrupts_dw1_10_IRQn, 
            .intrPriority = 7UL                     
    };

static uint8_t leftWS_frameBuffer[WS_NUM_LEDS*WS_BYTES_PER_PIXEL+WS_ZOFFSET];
static uint8_t rightWS_frameBuffer[WS_NUM_LEDS*WS_BYTES_PER_PIXEL+WS_ZOFFSET];

cy_stc_scb_spi_context_t leftWS_SPI_context;
cy_stc_scb_spi_context_t rightWS_SPI_context;

#define WS_NUM_DESCRIPTORS (sizeof(leftWS_frameBuffer) / 256 + 1)
static cy_stc_dma_descriptor_t leftWSDescriptors[WS_NUM_DESCRIPTORS];
static cy_stc_dma_descriptor_t rightWSDescriptors[WS_NUM_DESCRIPTORS];




// Function: convert3Code
// This function takes an 8-bit value representing a color
// and turns it into a WS2812 bit code... where 1=110 and 0=011
// 1 input byte turns into three output bytes of a uint32_t
uint32_t WS_convert3Code(uint8_t input)
{
    uint32_t rval=0;
    for(int i=0;i<8;i++)
    {
        if(input%2)
        {
            rval |= WS_ONE3;
        }
        else
        {
            rval |= WS_ZERO3;
        }
        rval = rval >> 3;
        
        input = input >> 1;
    }
    return rval;
}

// Function: WS_setRGB
// Takes a position and a three byte rgb value and updates the WS_frameBuffer with the correct 9-bytes
void WS_setRGB(LED_SELECT_T ledSide, uint16_t led,uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t* frameBufferPtr;


    typedef union {
    uint8_t bytes[4];
    uint32_t word;
    } WS_colorUnion;
    
    switch(ledSide)
    {
        case LEFT_STRING:
            frameBufferPtr = leftWS_frameBuffer;
        break;

        case RIGHT_STRING:
            frameBufferPtr = rightWS_frameBuffer;
        break;

        default:
        break;
    }


    WS_colorUnion color;
    color.word = WS_convert3Code(green);
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+WS_ZOFFSET] = color.bytes[2];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+1+WS_ZOFFSET] = color.bytes[1];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+2+WS_ZOFFSET] = color.bytes[0];
    
    color.word = WS_convert3Code(red);
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+3+WS_ZOFFSET] = color.bytes[2];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+4+WS_ZOFFSET] = color.bytes[1];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+5+WS_ZOFFSET] = color.bytes[0];
 
    color.word = WS_convert3Code(blue);
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+6+WS_ZOFFSET] = color.bytes[2];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+7+WS_ZOFFSET] = color.bytes[1];
    frameBufferPtr[led*WS_BYTES_PER_PIXEL+8+WS_ZOFFSET] = color.bytes[0];
}

void leftWS_DMAComplete(void)
{
    Cy_DMA_Channel_ClearInterrupt(leftWS_DMA_HW, leftWS_DMA_CHANNEL);   
}

void rightWS_DMAComplete(void)
{
    Cy_DMA_Channel_ClearInterrupt(rightWS_DMA_HW, rightWS_DMA_CHANNEL);   
}



static void leftWS_DMAConfigure(void)
{
    // I copies this structure from the PSoC Creator Component configuration 
    // in generated source
    cy_stc_dma_descriptor_config_t WS_DMA_Descriptors_config =
    {
    .retrigger       = CY_DMA_RETRIG_IM,
    .interruptType   = CY_DMA_DESCR_CHAIN,
    .triggerOutType  = CY_DMA_1ELEMENT,
    .channelState    = CY_DMA_CHANNEL_ENABLED,
    .triggerInType   = CY_DMA_1ELEMENT,
    .dataSize        = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType  = CY_DMA_1D_TRANSFER,
    .srcAddress      = NULL,
    .dstAddress      = NULL,
    .srcXincrement   = 1L,
    .dstXincrement   = 0L,
    .xCount          = 256UL,
    .srcYincrement   = 0L,
    .dstYincrement   = 0L,
    .yCount          = 1UL,
    .nextDescriptor  = 0
    };
 
    for(unsigned int i=0;i<WS_NUM_DESCRIPTORS;i++)
    {
        Cy_DMA_Descriptor_Init(&leftWSDescriptors[i], &WS_DMA_Descriptors_config);
        Cy_DMA_Descriptor_SetSrcAddress(&leftWSDescriptors[i], (uint8_t *)&leftWS_frameBuffer[i*256]);
        Cy_DMA_Descriptor_SetDstAddress(&leftWSDescriptors[i], (uint32_t *)&leftWS_SPI_HW->TX_FIFO_WR);
        Cy_DMA_Descriptor_SetXloopDataCount(&leftWSDescriptors[i],256); // the last
        Cy_DMA_Descriptor_SetNextDescriptor(&leftWSDescriptors[i],&leftWSDescriptors[i+1]);
    }
    
    // The last one needs a bit of change
    Cy_DMA_Descriptor_SetXloopDataCount(&leftWSDescriptors[WS_NUM_DESCRIPTORS-1],sizeof(leftWS_frameBuffer)-256*(WS_NUM_DESCRIPTORS-1)); // the last
    Cy_DMA_Descriptor_SetNextDescriptor(&leftWSDescriptors[WS_NUM_DESCRIPTORS-1],0);
    Cy_DMA_Descriptor_SetChannelState(&leftWSDescriptors[WS_NUM_DESCRIPTORS-1],CY_DMA_CHANNEL_DISABLED);
 
    
     /* Initialize and enable the interrupt from WS_DMA */
    Cy_DMA_Channel_SetInterruptMask(leftWS_DMA_HW, leftWS_DMA_CHANNEL, CY_DMA_INTR_MASK);
    Cy_SysInt_Init(&leftWS_DMA_INT_cfg, &leftWS_DMAComplete);
    NVIC_EnableIRQ(leftWS_DMA_INT_cfg.intrSrc);
}


static void rightWS_DMAConfigure(void)
{
    // I copies this structure from the PSoC Creator Component configuration 
    // in generated source
    cy_stc_dma_descriptor_config_t WS_DMA_Descriptors_config =
    {
    .retrigger       = CY_DMA_RETRIG_IM,
    .interruptType   = CY_DMA_DESCR_CHAIN,
    .triggerOutType  = CY_DMA_1ELEMENT,
    .channelState    = CY_DMA_CHANNEL_ENABLED,
    .triggerInType   = CY_DMA_1ELEMENT,
    .dataSize        = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType  = CY_DMA_1D_TRANSFER,
    .srcAddress      = NULL,
    .dstAddress      = NULL,
    .srcXincrement   = 1L,
    .dstXincrement   = 0L,
    .xCount          = 256UL,
    .srcYincrement   = 0L,
    .dstYincrement   = 0L,
    .yCount          = 1UL,
    .nextDescriptor  = 0
    };
 
    for(unsigned int i=0;i<WS_NUM_DESCRIPTORS;i++)
    {
        Cy_DMA_Descriptor_Init(&rightWSDescriptors[i], &WS_DMA_Descriptors_config);
        Cy_DMA_Descriptor_SetSrcAddress(&rightWSDescriptors[i], (uint8_t *)&rightWS_frameBuffer[i*256]);
        Cy_DMA_Descriptor_SetDstAddress(&rightWSDescriptors[i], (uint32_t *)&rightWS_SPI_HW->TX_FIFO_WR);
        Cy_DMA_Descriptor_SetXloopDataCount(&rightWSDescriptors[i],256); // the last
        Cy_DMA_Descriptor_SetNextDescriptor(&rightWSDescriptors[i],&rightWSDescriptors[i+1]);
    }
    
    // The last one needs a bit of change
    Cy_DMA_Descriptor_SetXloopDataCount(&rightWSDescriptors[WS_NUM_DESCRIPTORS-1],sizeof(rightWS_frameBuffer)-256*(WS_NUM_DESCRIPTORS-1)); // the last
    Cy_DMA_Descriptor_SetNextDescriptor(&rightWSDescriptors[WS_NUM_DESCRIPTORS-1],0);
    Cy_DMA_Descriptor_SetChannelState(&rightWSDescriptors[WS_NUM_DESCRIPTORS-1],CY_DMA_CHANNEL_DISABLED);
 
    
     /* Initialize and enable the interrupt from WS_DMA */
    Cy_DMA_Channel_SetInterruptMask(rightWS_DMA_HW, rightWS_DMA_CHANNEL, CY_DMA_INTR_MASK);
    Cy_SysInt_Init(&rightWS_DMA_INT_cfg, &rightWS_DMAComplete);
    NVIC_EnableIRQ(rightWS_DMA_INT_cfg.intrSrc);
}


void leftWS_DMATrigger()
{
    leftWS_DMA_channelConfig.descriptor = &leftWSDescriptors[0];
    Cy_DMA_Channel_Init(leftWS_DMA_HW, leftWS_DMA_CHANNEL, &leftWS_DMA_channelConfig);
    Cy_DMA_Channel_SetDescriptor(leftWS_DMA_HW, leftWS_DMA_CHANNEL, &leftWSDescriptors[0]);
    Cy_DMA_Channel_Enable(leftWS_DMA_HW,leftWS_DMA_CHANNEL);
    Cy_DMA_Enable(leftWS_DMA_HW);    
}


void rightWS_DMATrigger()
{
    rightWS_DMA_channelConfig.descriptor = &rightWSDescriptors[0];
    Cy_DMA_Channel_Init(rightWS_DMA_HW, rightWS_DMA_CHANNEL, &rightWS_DMA_channelConfig);
    Cy_DMA_Channel_SetDescriptor(rightWS_DMA_HW, rightWS_DMA_CHANNEL, &rightWSDescriptors[0]);
    Cy_DMA_Channel_Enable(rightWS_DMA_HW,rightWS_DMA_CHANNEL);
    Cy_DMA_Enable(rightWS_DMA_HW);    
}


// Function: initWSspi
// Initializes and enables the SPI ports
//WS2812 driver uses only the SPI MOSI
static void initWSspi(void)
{
    Cy_SCB_SPI_Init(leftWS_SPI_HW, &leftWS_SPI_config, &leftWS_SPI_context);
    Cy_SCB_SPI_Init(rightWS_SPI_HW, &rightWS_SPI_config, &rightWS_SPI_context);
    Cy_SCB_SPI_Enable(leftWS_SPI_HW);
    Cy_SCB_SPI_Enable(rightWS_SPI_HW);
}



// Function: fill_side
// Fills LED buffer with color up to fillNum
void WS_fill_side(LED_SELECT_T ledSide, uint16_t fillNum, uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t index;
    if(fillNum > WS_NUM_LEDS) fillNum = WS_NUM_LEDS;

    for(index = 0; index < fillNum; index++)
    {
        WS_setRGB(ledSide, index, red, green, blue);
    }

    for(index = fillNum; index < WS_NUM_LEDS; index++)
    {
        WS_setRGB(ledSide, index, 0x00, 0x00, 0x00);
    }
}

// Function: clear_side
// Clears (turns off) all LEDs on a side
void WS_clear_side(LED_SELECT_T ledSide)
{
    uint16_t index;

    for(index = 0; index < WS_NUM_LEDS; index++)
    {
        WS_setRGB(ledSide, index, 0x00, 0x00, 0x00);
    }
}

// Function: fill_random
// Sets all LEDs to a random value
void WS_fill_random(void)
{
    uint16_t index;

    for(index = 0; index < WS_NUM_LEDS; index++)
    {
        if(rand() % 2)
        {
            WS_setRGB(LEFT_STRING, index, 0x00,rand() % 0xFF,rand() % 0xFF);
            WS_setRGB(RIGHT_STRING, index, rand() % 0xFF, 0x00, rand() % 0xFF);
        }
        else
        {
            WS_setRGB(LEFT_STRING, index, rand() % 0xFF, 0x00, rand() % 0xFF);
            WS_setRGB(RIGHT_STRING, index, 0x00,rand() % 0xFF,rand() % 0xFF);
        }
    }
}

// Function: percent_to_LEDs
// converts percent to LED string length
// based on number of LEDs in string
uint32_t WS_percent_toLEDs(uint8_t percent)
{
    uint32_t numLEDs = (percent * WS_NUM_LEDS) / 100;
    return numLEDs;
}

//this thread sets up the WS2812 LED driver hardware, and triggers an update every WSDRIVER_SLEEP_TIME ms
void wsThread(void)
{
    leftWS_DMAConfigure();
    rightWS_DMAConfigure();
    initWSspi();
    WS_clear_side(LEFT_STRING);
    WS_clear_side(RIGHT_STRING);

    while(1)
    {
        leftWS_DMATrigger();
        rightWS_DMATrigger();

        ThisThread::sleep_for(WSDRIVER_SLEEP_TIME);
    }
}



