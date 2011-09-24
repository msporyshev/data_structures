
#include <string.h>
#include <stdlib.h>
#include "linear_sequence.h"

#define LSQ_BASE_ARRAY_PHYS_SIZE 10;
#define isHandleInvalid(handle)(handle == LSQ_HandleInvalid)

typedef enum 
{
	pk_BEFOREFIRST,
	pk_DEREFERENCABLE,
	pk_PASTREAR
} IteratorPosKindT;

typedef struct 
{
	LSQ_BaseTypeT * data_ptr;
	int physical_size;
	int logical_size;
} ArrayDataT;

typedef struct 
{
	ArrayDataT * array_data;
	IteratorPosKindT position_kind;
	LSQ_IntegerIndexT index;
} IteratorT;

static int isContainerFull(ArrayDataT * handle);

static int createIterator(ArrayDataT * handle);

static void setContainerSize(ArrayDataT * handle, int size);



static int isContainerFull(ArrayDataT * handle)
{
	return (isHandleInvalid(handle)) ? -1 : handle->logical_size == handle->physical_size;	
}

static void setContainerSize(ArrayDataT * handle, int size)
{
	if (isHandleInvalid(handle)) 
		return;
	handle->physical_size = size;
	handle->data_ptr = (LSQ_BaseTypeT *)realloc(handle->data_ptr, 
												size * sizeof(LSQ_BaseTypeT));
}

static LSQ_IteratorT createIterator(LSQ_HandleT handle)
{
	IteratorT * iterator = NULL;
	if(isHandleInvalid(handle)) 
		return LSQ_HandleInvalid;
	iterator = (IteratorT *)malloc(sizeof(IteratorT));
	if isHandleInvalid(iterator) 
		return LSQ_HandleInvalid;
	iterator->array_data = (ArrayDataT *)handle;
	return iterator;
}

extern LSQ_HandleT LSQ_CreateSequence(void) 
{
	ArrayDataT * array_data = NULL;
	array_data = (ArrayDataT *)malloc(sizeof(ArrayDataT));
	if isHandleInvalid(array_data)
		return LSQ_HandleInvalid;
	array_data->data_ptr = NULL;
	array_data->physical_size = 0;
	array_data->logical_size = 0;
	return array_data;
}

extern void LSQ_DestroySequence(LSQ_HandleT handle) 
{
	ArrayDataT * tmp_array = (ArrayDataT *)handle;
	if isHandleInvalid(handle)
		return;
	free(tmp_array->data_ptr);
	free(handle);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	return (isHandleInvalid(handle)) ? -1 : ((ArrayDataT *)handle)->logical_size;
}

extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_DEREFERENCABLE));
}

extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_PASTREAR));
}

extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_BEFOREFIRST));
}

extern LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = NULL;
	if (!LSQ_IsIteratorDereferencable(iterator)) 
		return LSQ_HandleInvalid;
	tmp_iterator = (IteratorT *)iterator;
	return (tmp_iterator)->array_data->data_ptr + tmp_iterator->index;
}

extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	IteratorT * tmp_iterator = NULL; 
	if isHandleInvalid(handle)
		return LSQ_HandleInvalid;
	tmp_iterator = (IteratorT *)createIterator(handle);
	tmp_iterator->index = index;
	tmp_iterator->position_kind = pk_DEREFERENCABLE;
	if (index >= tmp_iterator->array_data->logical_size) 
	{
		tmp_iterator->index = tmp_iterator->array_data->logical_size;
		tmp_iterator->position_kind = pk_PASTREAR;
	}
	if (index < 0) 
	{
		tmp_iterator->index = -1;
		tmp_iterator->position_kind = pk_BEFOREFIRST;
	}
	return tmp_iterator;
}

extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	return LSQ_GetElementByIndex(handle, 0);
}

extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	return isHandleInvalid(handle) ? LSQ_HandleInvalid : LSQ_GetElementByIndex(handle, ((ArrayDataT *)handle)->logical_size);
}

extern void LSQ_DestroyIterator(LSQ_IteratorT iterator)
{
	if (isHandleInvalid(iterator))  
		return;
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
	IteratorT * tmp_iterator = NULL;
	if isHandleInvalid(iterator)
		return;
	tmp_iterator = (IteratorT *)iterator;
	tmp_iterator->index += shift;
	tmp_iterator->position_kind = pk_DEREFERENCABLE;
	if (tmp_iterator->index >= tmp_iterator->array_data->logical_size)
	{
		tmp_iterator->position_kind = pk_PASTREAR;
		tmp_iterator->index = tmp_iterator->array_data->logical_size;
	}
	if (tmp_iterator->index < 0) 
	{
		tmp_iterator->position_kind = pk_BEFOREFIRST;
		tmp_iterator->index = -1;
	}
}

extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	if isHandleInvalid(iterator)
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
	IteratorT * tmp_iterator = (IteratorT *)iterator;
    ArrayDataT * tmp_array = NULL;
	int * element_ptr = NULL, tmp_size = LSQ_BASE_ARRAY_PHYS_SIZE;

    if (isHandleInvalid(iterator))
    {
        return;
    }
	tmp_array = tmp_iterator->array_data;
	if (isContainerFull(tmp_array))
	{
		tmp_size = tmp_array->physical_size + LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(tmp_array, tmp_size);
	}
	tmp_array->logical_size++;
	memmove(tmp_array->data_ptr + tmp_iterator->index + 1, 
			tmp_array->data_ptr + tmp_iterator->index , 
			sizeof(LSQ_BaseTypeT) * (tmp_array->logical_size - tmp_iterator->index - 1));
	element_ptr = tmp_array->data_ptr + tmp_iterator->index; 
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
	IteratorT * tmp_iterator = (IteratorT *)iterator;
    ArrayDataT * tmp_array = NULL;
	int tmp_size = LSQ_BASE_ARRAY_PHYS_SIZE;

    if (!LSQ_IsIteratorDereferencable(iterator))
    {
        return;
    }
	tmp_array = tmp_iterator->array_data;
	tmp_array->logical_size--;
	memmove(tmp_array->data_ptr + tmp_iterator->index, 
			tmp_array->data_ptr + tmp_iterator->index + 1, 
			sizeof(LSQ_BaseTypeT) * (tmp_array->logical_size - tmp_iterator->index));
	if ((tmp_array->physical_size - tmp_array->logical_size) >= tmp_size)
	{
		tmp_size = tmp_array->physical_size - LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(tmp_array, tmp_size);
	}
}