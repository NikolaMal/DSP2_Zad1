#include "processing.h"
#include "iir.h"
#define INT16_MAX_VALUE 32768

void calculateShelvingCoeff(float c_alpha, Int16* output)
{
	/* Your code here */
    output[0] = c_alpha*INT16_MAX_VALUE;
    output[1] = -INT16_MAX_VALUE;
    output[2] = INT16_MAX_VALUE-1;
    output[3] = -c_alpha*INT16_MAX_VALUE;
}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output)
{
	/* Your code here */
    output[0] = c_alpha*INT16_MAX_VALUE;
    output[1] = ((Int32)(-c_beta*(1+c_alpha)*INT16_MAX_VALUE)) >> 1;
    output[2] = INT16_MAX_VALUE-1;
    output[3] = INT16_MAX_VALUE-1;
    output[4] = ((Int32)(-c_beta*(1+c_alpha)*INT16_MAX_VALUE)) >> 1;
    output[5] = c_alpha*INT16_MAX_VALUE;

}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 temp = first_order_IIR(input, coeff, x_history, y_history) >> 1;
	input = input >> 1;
	return  input - temp + _smpy(k, (input+temp));
}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 temp = first_order_IIR(input, coeff, x_history, y_history) >> 1;
	input = input >>1;

	return input + temp + _smpy(k, (input-temp));
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */

	Int16 temp = second_order_IIR(input, coeff, x_history, y_history);
	input = input >> 1;

	return input + temp + _smpy(k, (input-temp));

}
