/* Project includes. */
#include "main.h"


/* App includes. */
#include "app.h"
#include "logger.h"

/* Application includes. */


/********************** macros and definitions *******************************/

#define TEST_1 (1)
#define TEST_2 (2)
#define TEST_3 (3)

#define TEST_NUMBER TEST_1 //TEST_1, TEST_2, TEST_3

#define SAMPLES_COUNTER (100)
#define AVERAGER_SIZE (16)

/********************** external data declaration *****************************/

extern ADC_HandleTypeDef hadc1;

/********************** external functions definition ************************/



/********************** internal data declaration ****************************/
uint32_t tickstart;
uint16_t sample_idx;

#if TEST_3==TEST_NUMBER
uint16_t sample_array[SAMPLES_COUNTER];
bool b_trig_new_conversion;
#endif

/********************** internal data definition *****************************/

/********************** internal functions definitions ***********************/
bool test1_tick();
bool test2_tick();
bool test3_tick();
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef ADC_Poll_Read(uint16_t *value);

/********************** internal functions declaration ***********************/



void app_init(void)
{

#if TEST_3==TEST_NUMBER
	HAL_NVIC_SetPriority(ADC1_2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
#endif

	sample_idx = 0;
	LOGGER_LOG ("Test #%u starts\n", TEST_NUMBER);
	tickstart = HAL_GetTick();
}

void app_update(void)
{
	static bool b_test_done = false;

	if (!b_test_done) {
#if TEST_1==TEST_NUMBER
		b_test_done = test1_tick();
#endif
#if TEST_2==TEST_NUMBER
		b_test_done = test2_tick();
#endif
#if TEST_3==TEST_NUMBER
		b_test_done = test3_tick();
#endif

		if (b_test_done) {
			LOGGER_LOG("Test #%u ends. Ticks: %lu\n", TEST_NUMBER, HAL_GetTick()-tickstart);
		}
	}

}


//	Requests start of conversion, waits until conversion done
HAL_StatusTypeDef ADC_Poll_Read(uint16_t *value) {
	HAL_StatusTypeDef res;

	res=HAL_ADC_Start(&hadc1);
	if ( HAL_OK==res ) {
		res=HAL_ADC_PollForConversion(&hadc1, 0);
		if ( HAL_OK==res ) {
			*value = HAL_ADC_GetValue(&hadc1);
		}
	}
	return res;
}

#if TEST_3==TEST_NUMBER
/* ADC callback */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {

	sample_array[sample_idx++] = HAL_ADC_GetValue(&hadc1);
	if (sample_idx<SAMPLES_COUNTER) {
		b_trig_new_conversion = true;
	}
}
#endif


#if TEST_1==TEST_NUMBER
bool test1_tick() {

	uint16_t value;
	bool b_done = false;

	if (sample_idx>=SAMPLES_COUNTER) {
		b_done = true;
		goto test1_tick_end;
	}

	if (HAL_OK==ADC_Poll_Read(&value)) {
		LOGGER_LOG("%u\n", value);
	}
	else
	{
		LOGGER_LOG("error\n");
	}
	sample_idx++;
test1_tick_end:
	return b_done;
}
#endif

#if TEST_2==TEST_NUMBER
bool test2_tick() {

	uint32_t averaged = 0;
	uint16_t value;
	bool b_done = false;

	if (sample_idx>=SAMPLES_COUNTER) {
		b_done = true;
		goto test2_tick_end;
	}

	for (uint16_t averager=0 ; averager<AVERAGER_SIZE ; averager++) {
		//Activate the ADC peripheral and start conversions
		if (HAL_OK==ADC_Poll_Read(&value)) {
			averaged += value;
		}
	}
	averaged = averaged / AVERAGER_SIZE;
	LOGGER_LOG("%lu\n", averaged);

	sample_idx++;
test2_tick_end:
	return b_done;
}
#endif

#if TEST_3==TEST_NUMBER
bool test3_tick() {

	bool b_done = false;

	if (sample_idx>=SAMPLES_COUNTER) {
		b_done = true;
		goto test3_tick_end;
	}

	/* start of first conversion */
	if (0==sample_idx) {
		b_trig_new_conversion = true;
	}


	if (b_trig_new_conversion) {
		b_trig_new_conversion = false;
		HAL_ADC_Start_IT(&hadc1);
	}

test3_tick_end:
	if (b_done) {
		for (sample_idx=0;sample_idx<SAMPLES_COUNTER;sample_idx++) {
			LOGGER_LOG("%u\n",sample_array[sample_idx] );
		}
	}
	return b_done;
}
#endif

/********************** end of file ******************************************/
