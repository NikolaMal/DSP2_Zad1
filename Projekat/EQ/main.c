//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"

#define ALPHA 0.7
#define BETA 0

#define ALPHALP 0.888622125173229
#define ALPHAPEEK1 0.711648515824048
#define ALPHAPEEK2 0.763175873391206
#define ALPHAHP 0
#define BETAPEEK1 0.944806046591841
#define BETAPEEK2 0.542441537620623

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 16000L

// 300 Hz 850 Hz 530 Hz 2540 Hz 680 Hz 4KHz
// 0.01875  0.053125    0.033125   0.15875    0.0425      0.25






#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];
Int16 shelvingLPCoeff[4];
Int16 shelvingHPCoeff[4];
Int16 shelvingPeekCoeff1[6];
Int16 shelvingPeekCoeff2[6];
Int16 shelvingLPXHistory;
Int16 shelvingLPYHistory;
Int16 shelvingPeek1XHistory[2];
Int16 shelvingPeek1YHistory[2];
Int16 shelvingPeek2XHistory[2];
Int16 shelvingPeek2YHistory[2];
Int16 shelvingHPXHistory;
Int16 shelvingHPYHistory;
Int16 dirak[AUDIO_IO_SIZE];
Int16 output[AUDIO_IO_SIZE];
static Int16 K[4] = {16000, 16000, 16000, 16000};
int ind=0;

Int16 result[AUDIO_IO_SIZE];

Uint16 getSwitch();


void main( void )
{
	/* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

	/* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

	//aic3204_set_input_filename("../input1.pcm");
    //aic3204_set_output_filename("../output1.pcm");

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

	aic3204_dma_init();

	set_sampling_frequency_and_gain(SAMPLE_RATE, 0);
	int i;
	for(i=0;i<2;i++){
		shelvingPeek1XHistory[i] = 0;
		shelvingPeek1YHistory[i] = 0;
		shelvingPeek2XHistory[i] = 0;
		shelvingPeek2YHistory[i] = 0;

	}

	shelvingLPXHistory = 0;
	shelvingLPYHistory = 0;
	shelvingHPXHistory = 0;
	shelvingHPYHistory = 0;

	dirak[0] = 16000;

	for(i=1;i<AUDIO_IO_SIZE;i++){
		dirak[i] = 0;
	}

	/*calculatePeekCoeff(0.7, 0,  shelvingPeekCoeff1);

	for(i=0;i<AUDIO_IO_SIZE;i++){

		result[i] = shelvingPeek(dirak[i], shelvingPeekCoeff1, shelvingPeekXHistory, shelvingPeekYHistory, 24576); // testiranje impulsnih odziva

	}*/

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */


    calculateShelvingCoeff(ALPHALP, shelvingLPCoeff);
    calculateShelvingCoeff(ALPHAHP, shelvingHPCoeff);
    calculatePeekCoeff(ALPHAPEEK1, BETAPEEK1, shelvingPeekCoeff1);
    calculatePeekCoeff(ALPHAPEEK2, BETAPEEK2, shelvingPeekCoeff2);




    clearLCD();

    while(1)
    {


    	aic3204_read_block(sampleBufferL, sampleBufferR);

    	Uint16 sw = getSwitch();

    	if(sw == SW1) {
    		ind = (ind+1)%4;
    		setWritePointerToFirstChar();
    		printChar(ind+'0');
    		printChar(' ');
    		printChar(K[ind]/3277 + '0');

    	}

    	else if(sw == SW2){
    		K[ind] = K[ind] - 3277;
    		if(K[ind] < 0){
    			K[ind] = 32767;
    		}

    		setWritePointerToFirstChar();
    		printChar(ind+'0');
    		printChar(' ');
    		printChar(K[ind]/3277 + '0');
    	}


    	int i, j;

    	for(i=0;i<AUDIO_IO_SIZE;i++){
    		output[i] = shelvingLP(sampleBufferL[i], shelvingLPCoeff, &shelvingLPXHistory, &shelvingLPYHistory, K[0]);
    		output[i] = shelvingPeek(output[i], shelvingPeekCoeff1, shelvingPeek1XHistory, shelvingPeek1YHistory, K[1]);
    		output[i] = shelvingPeek(output[i], shelvingPeekCoeff2, shelvingPeek2XHistory, shelvingPeek2YHistory, K[2]);
    		output[i] = shelvingHP(output[i], shelvingHPCoeff, &shelvingHPXHistory, &shelvingHPYHistory, K[3]);

    		/*sampleBufferR[i] = shelvingLP(sampleBufferR[i], shelvingLPCoeff, &shelvingLPXHistory, &shelvingLPYHistory, K[0]);
    		sampleBufferR[i] = shelvingPeek(sampleBufferR[i], shelvingPeekCoeff1, shelvingPeekXHistory, shelvingPeekYHistory, K[1]);
    		for(j=0;j<2;j++){
    		    	shelvingPeekXHistory[j] = 0;
    		    	shelvingPeekYHistory[j] = 0;

    		    }
    		sampleBufferR[i] = shelvingPeek(sampleBufferR[i], shelvingPeekCoeff2, shelvingPeekXHistory, shelvingPeekYHistory, K[2]);
    		for(j=0;j<2;j++){
    		    				shelvingPeekXHistory[j] = 0;
    		    				shelvingPeekYHistory[j] = 0;

    		    			}
    		sampleBufferR[i] = shelvingHP(sampleBufferR[i], shelvingHPCoeff, &shelvingHPXHistory, &shelvingHPYHistory, K[3]);*/
    	}





		aic3204_write_block(sampleBufferL, sampleBufferR);
	}

    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}

Uint16 getSwitch() {
        static Uint16 prev = NoKey;
        Uint16 sw = EZDSP5535_SAR_getKey();
        if (sw == prev) {
            return NoKey;
        } else {
            prev = sw;
            return sw;
        }
    }
