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

#define ALPHA 0.3
#define K 8192

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];
Int16 shelvingLPCoeff[4];
Int16 shelvingLPXHistory;
Int16 shelvingLPYHistory;

Int16 result[1000];


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

	aic3204_set_input_filename("../input1.pcm");
    aic3204_set_output_filename("../output1.pcm");

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();
	
	aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    while(1)
    {


    	aic3204_read_block(sampleBufferL, sampleBufferR);

    	/* Your code here */

    	calculateShelvingCoeff(ALPHA, shelvingLPCoeff);
    	int i;
    	for(i=0;i<1000;i++){
    	    if(i==0){
    	                result[i] = shelvingLP(32768, shelvingLPCoeff, &shelvingLPXHistory, &shelvingLPYHistory, K);
    	            }

    	            else result[i] = shelvingLP(0, shelvingLPCoeff, &shelvingLPXHistory, &shelvingLPYHistory, K);

    	}





		aic3204_write_block(sampleBufferR, sampleBufferR);
	}

    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}

