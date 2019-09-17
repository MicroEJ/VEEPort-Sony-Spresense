#include "sni.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/**
 * See description in sni.h.
 */
	int32_t SNI_getArrayElements(int8_t *java_array, int32_t java_start, int32_t java_length, int8_t *buffer, uint32_t buffer_length, int8_t **out_buffer, uint32_t *out_length, bool refresh_content)
	{

		// Check inputs: no null argument, array bounds
		if (java_array == NULL || buffer == NULL || out_buffer == NULL || out_length == NULL ||
			java_start < 0 || java_length < 0 || java_length > (SNI_getArrayLength(java_array) - java_start))
		{
			return -1;
		}

		if (SNI_isImmortalArray(java_array))
		{
			// Use the given java array
			*out_buffer = java_array + java_start;
			*out_length = java_length;
		}
		else
		{
			// Use the given buffer and copy content if needed
			*out_buffer = buffer;
			*out_length = java_length < buffer_length ? java_length : buffer_length;
			if (refresh_content == true)
			{
				memcpy(buffer, java_array + java_start, *out_length);
			}
		}

		return SNI_OK;
	}

	/**
 * See description in sni.h.
 */
	int32_t SNI_releaseArrayElements(int8_t *java_array, int32_t java_start, int32_t java_length, int8_t *buffer, uint32_t buffer_length)
	{

		// Check inputs: no null argument, array bounds
		if (java_array == NULL || buffer == NULL ||
			java_start < 0 || java_length < 0 || java_length > (SNI_getArrayLength(java_array) - java_start) ||
			buffer_length > java_length)
		{
			return -1;
		}

		if (!SNI_isImmortalArray(java_array))
		{
			memcpy(java_array + java_start, buffer, buffer_length);
		}
		// else: nothing to do

		return SNI_OK;
	}
#ifdef __cplusplus
}
#endif