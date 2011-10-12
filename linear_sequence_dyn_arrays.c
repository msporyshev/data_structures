
#include <string.h>
#include <stdlib.h>
#include "linear_sequence.h"

#define PHYS_SIZE_CHANGE_FACTOR 10;
#define LSQ_ARRAY_BASE_PHYS_SIZE 1;
#define SIZE_RATIO_LOWER_THRESHOLD 0.25;
#define IS_HANDLE_INVALID(handle)(handle == LSQ_HandleInvalid)

typedef enum 
{
	BEFOREFIRST,
	DEREFERENCABLE,
	PASTREAR,
} IteratorStateT;

typedef struct 
{
	LSQ_BaseTypeT * data_ptr;
	int physical_size;
	int logical_size;
} ArrayDataT;

typedef struct 
{
	ArrayDataT * array_data;
	IteratorStateT state;
	LSQ_IntegerIndexT index;
} IteratorT;

static int isContainerFull(ArrayDataT * handle);

static LSQ_IteratorT createIterator(LSQ_HandleT handle);

static void setContainerSize(ArrayDataT * handle, int size);


static int isContainerFull(ArrayDataT * handle)
{
	return (IS_HANDLE_INVALID(handle)) ? -1 : handle->logical_size == handle->physical_size;	
}

static void setContainerSize(ArrayDataT * handle, int size)
{
	if (IS_HANDLE_INVALID(handle)) 
		return;
	handle->physical_size = size;
	handle->data_ptr = (LSQ_BaseTypeT *)realloc(handle->data_ptr, 
		size * sizeof(LSQ_BaseTypeT));
}

static LSQ_IteratorT createIterator(LSQ_HandleT handle)
{
	IteratorT * iterator = NULL;
	if(IS_HANDLE_INVALID(handle)) 
		return LSQ_HandleInvalid;
	iterator = (IteratorT *)malloc(sizeof(IteratorT));
	if (iterator == NULL) 
		return LSQ_HandleInvalid;
	iterator->array_data = (ArrayDataT *)handle;
	return iterator;
}

extern LSQ_HandleT LSQ_CreateSequence(void) 
{
	ArrayDataT * array_data = NULL;
	array_data = (ArrayDataT *)malloc(sizeof(ArrayDataT));
	if (array_data == NULL)
		return LSQ_HandleInvalid;
	array_data->data_ptr = NULL;
	array_data->physical_size = LSQ_ARRAY_BASE_PHYS_SIZE;
	array_data->logical_size = 0;
	return array_data;
}

extern void LSQ_DestroySequence(LSQ_HandleT handle) 
{
	ArrayDataT * array_data = (ArrayDataT *)handle;
	if (IS_HANDLE_INVALID(handle) || IS_HANDLE_INVALID(array_data->data_ptr))
		return;
	free(array_data->data_ptr);
	free(handle);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	return (IS_HANDLE_INVALID(handle)) ? -1 : ((ArrayDataT *)handle)->logical_size;
}

extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	return (!IS_HANDLE_INVALID(iterator) && (((IteratorT *)iterator)->state == DEREFERENCABLE));
}

extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	return (!IS_HANDLE_INVALID(iterator) && (((IteratorT *)iterator)->state == PASTREAR));
}

extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	return (!IS_HANDLE_INVALID(iterator) && (((IteratorT *)iterator)->state == BEFOREFIRST));
}

extern LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * iter = NULL;
	if (!LSQ_IsIteratorDereferencable(iterator)) 
		return LSQ_HandleInvalid;
	iter = (IteratorT *)iterator;
	return (iter)->array_data->data_ptr + iter->index;
}

extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	IteratorT * iter = NULL; 
	if IS_HANDLE_INVALID(handle)
		return LSQ_HandleInvalid;
	iter = (IteratorT *)createIterator(handle);
	iter->index = index;
	iter->state = DEREFERENCABLE;
	if (index >= iter->array_data->logical_size) 
	{
		iter->index = iter->array_data->logical_size;
		iter->state = PASTREAR;
	}
	if (index < 0) 
	{
		iter->index = -1;
		iter->state = BEFOREFIRST;
	}
	return iter;
}

extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	return LSQ_GetElementByIndex(handle, 0);
}

extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	return IS_HANDLE_INVALID(handle) ? LSQ_HandleInvalid : LSQ_GetElementByIndex(handle, ((ArrayDataT *)handle)->logical_size);
}

extern void LSQ_DestroyIterator(LSQ_IteratorT iterator)
{
	free(iterator);
}

extern void LSQ_AdvanceOneElement(LSQ_IteratorT iterator)
{
	LSQ_ShiftPosition(iterator, 1);
}

extern void LSQ_RewindOneElement(LSQ_IteratorT iterator)
{
	LSQ_ShiftPosition(iterator, -1);
}

extern void LSQ_ShiftPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT shift)
{
	IteratorT * iter = NULL;
	if IS_HANDLE_INVALID(iterator)
		return;
	iter = (IteratorT *)iterator;
	iter->index += shift;
	iter->state = DEREFERENCABLE;
	if (iter->index >= iter->array_data->logical_size)
	{
		iter->state = PASTREAR;
		iter->index = iter->array_data->logical_size;
	}
	if (iter->index < 0) 
	{
		iter->state = BEFOREFIRST;
		iter->index = -1;
	}
}

extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	if IS_HANDLE_INVALID(iterator)
		return;
	LSQ_ShiftPosition(iterator, pos - ((IteratorT *)iterator)->index);
}

extern void LSQ_InsertFrontElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
    LSQ_IteratorT iterator = LSQ_GetElementByIndex(handle, 0);
    LSQ_InsertElementBeforeGiven(iterator, element);
    LSQ_DestroyIterator(iterator);
}

extern void LSQ_InsertRearElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
    LSQ_IteratorT iterator = LSQ_GetPastRearElement(handle);
    LSQ_InsertElementBeforeGiven(iterator, element);
    LSQ_DestroyIterator(iterator);
}

extern void LSQ_InsertElementBeforeGiven(LSQ_IteratorT iterator, LSQ_BaseTypeT newElement)
{
	IteratorT * iter = (IteratorT *)iterator;
    ArrayDataT * array_data = NULL;
	int * element_ptr = NULL;

    if (IS_HANDLE_INVALID(iterator))
    {
        return;
    }
	array_data = iter->array_data;
	if (isContainerFull(array_data))
	{
		int new_size = LSQ_ARRAY_BASE_PHYS_SIZE;
		new_size = array_data->physical_size * PHYS_SIZE_CHANGE_FACTOR;
		setContainerSize(array_data, new_size);
	}
	array_data->logical_size++;
	memmove(array_data->data_ptr + iter->index + 1, 
			array_data->data_ptr + iter->index , 
			sizeof(LSQ_BaseTypeT) * (array_data->logical_size - iter->index - 1));
	element_ptr = array_data->data_ptr + iter->index; 
	*(element_ptr) = newElement;
}

extern void LSQ_DeleteFrontElement(LSQ_HandleT handle)
{
	LSQ_IteratorT iterator = LSQ_GetFrontElement(handle);
	LSQ_DeleteGivenElement(iterator);
	LSQ_DestroyIterator(iterator);
}

extern void LSQ_DeleteRearElement(LSQ_HandleT handle)
{
	LSQ_IteratorT iterator = LSQ_GetPastRearElement(handle);
	LSQ_RewindOneElement(iterator);
	LSQ_DeleteGivenElement(iterator);
	LSQ_DestroyIterator(iterator);
}

extern void LSQ_DeleteGivenElement(LSQ_IteratorT iterator) 
{
	IteratorT * iter = (IteratorT *)iterator;
    ArrayDataT * array_data = NULL;
	int is_container_empty_enough, new_size = LSQ_ARRAY_BASE_PHYS_SIZE;

    if (!LSQ_IsIteratorDereferencable(iterator))
    {
        return;
    }
	array_data = iter->array_data;
	array_data->logical_size--;
	memmove(array_data->data_ptr + iter->index, 
			array_data->data_ptr + iter->index + 1, 
			sizeof(LSQ_BaseTypeT) * (array_data->logical_size - iter->index));
	is_container_empty_enough = array_data->logical_size <= array_data->physical_size * SIZE_RATIO_LOWER_THRESHOLD;
	if (is_container_empty_enough)
	{
		new_size = array_data->physical_size / PHYS_SIZE_CHANGE_FACTOR;
		setContainerSize(array_data, new_size);
	}
}