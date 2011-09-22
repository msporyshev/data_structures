
#include <string.h>
#include <stdlib.h>
#include "linear_sequence.h"

#define LSQ_BASE_ARRAY_PHYS_SIZE 10;
#define isHandleInvalid(handle)(handle == LSQ_HandleInvalid)

typedef enum 
{
	ip_BEFOREFIRST,
	ip_PASTREAR
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
	//IteratorPosKindT position_kind;
	LSQ_IntegerIndexT index;
} IteratorT;

static int isContainerFull(ArrayDataT * handle)
{
	if (isHandleInvalid(handle))
		return -1; //c каким значением вылетать если null?
	return handle->logical_size == handle->physical_size;	
}

static int isContainerEmpty(ArrayDataT * handle)
{
	if (isHandleInvalid(handle))
		return -1;
	return handle->logical_size == 0;	
}

static void setContainerSize(ArrayDataT * handle, int size)
{
	if (isHandleInvalid(handle)) 
		return;
	if (size <= 0) 
	{
		free(handle);
		return;
	}
	handle->physical_size = size;
	handle->data_ptr = (LSQ_BaseTypeT *)realloc(handle->data_ptr, 
												size * sizeof(LSQ_BaseTypeT));
}

static LSQ_IteratorT createIterator(LSQ_HandleT handle)
{
	IteratorT * iterator = NULL;
	if(handle == LSQ_HandleInvalid) 
		return LSQ_HandleInvalid;
	iterator = (IteratorT *)malloc(sizeof(IteratorT));
	iterator->array_data = (ArrayDataT *)handle;
	return iterator;
}

static void insertElementByIndex(ArrayDataT * handle, 
								 LSQ_BaseTypeT element, 
								 LSQ_IntegerIndexT index)
{
	LSQ_IntegerIndexT tmp_index;
	if isHandleInvalid(handle) 
		return;
	if (isContainerFull(handle)) 
	{
		int tmp_size;
		tmp_size = handle->physical_size + LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(handle, tmp_size);
	}
	if (index == handle->logical_size) 
		LSQ_InsertRearElement(handle, element);
	tmp_index = (index > handle->logical_size) ? handle->logical_size : index;
	tmp_index = (index < 0) ? 0 : index;
	handle->logical_size++;
	memmove(handle->data_ptr + index + 1, 
			handle->data_ptr + index, 
			sizeof(LSQ_BaseTypeT) * (handle->logical_size - index));
	*(handle->data_ptr + index) = element;//лучше под адрес завести переменную?
}

static void deleteElementByIndex(ArrayDataT * handle, LSQ_IntegerIndexT index)
{
	int tmp_size;
	LSQ_IntegerIndexT tmp_index;
	if (isHandleInvalid(handle) ||
		isHandleInvalid((handle)->data_ptr))
		return;
	if (index == handle->logical_size - 1)
		LSQ_DeleteRearElement(handle);
	tmp_size = LSQ_BASE_ARRAY_PHYS_SIZE;
	tmp_index = (index > handle->logical_size) ? handle->logical_size : index;
	tmp_index = (index < 0) ? 0 : index;
	memmove(handle->data_ptr + index, 
			handle->data_ptr + index + 1, 
			sizeof(LSQ_BaseTypeT) * (handle->logical_size - index - 1));
	handle->logical_size--;
	if ((handle->physical_size - handle->logical_size) >= tmp_size)
	{
		tmp_size = handle->physical_size - LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(handle, tmp_size);
	}
}

extern LSQ_HandleT LSQ_CreateSequence(void) 
{
	ArrayDataT * array_data = NULL;
	array_data = (ArrayDataT *)malloc(sizeof(ArrayDataT));
	array_data->data_ptr = NULL;
	array_data->physical_size = 0;
	array_data->logical_size = 0;
	return array_data;
}

extern void LSQ_DestroySequence(LSQ_HandleT handle) 
{
	if isHandleInvalid(handle)
		return;
	free(((ArrayDataT *)handle)->data_ptr);
	free(handle);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	if isHandleInvalid(handle) 
		return -1;	
	return ((ArrayDataT *)handle)->logical_size;
}

extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	return (iterator != NULL) && 
		(((IteratorT *)iterator)->index >= 0) &&
		(((IteratorT *)iterator)->index < ((IteratorT *)iterator)->array_data->logical_size);
}


extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	IteratorT * tmp_iterator = NULL;
	if isHandleInvalid(iterator) 
		return 0;
	tmp_iterator = (IteratorT *)iterator;
	return (tmp_iterator->array_data != NULL) && 
		(tmp_iterator->index == (tmp_iterator->array_data->logical_size)) && 
		(tmp_iterator->index > 0);
}

/* Функция, определяющая, указывает ли данный итератор на элемент, предшествующий первому в контейнере */
extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	return (iterator != NULL) && 
		(((IteratorT *)iterator)->array_data != NULL) && 
		(((IteratorT *)iterator)->index == -1);
}

/* Функция разыменовывающая итератор. Возвращает указатель на элемент, на который ссылается данный итератор */
extern LSQ_BaseTypeT * LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = NULL;
	if (!LSQ_IsIteratorDereferencable(iterator)) 
		return NULL;
	tmp_iterator = (IteratorT *)iterator;
	return (tmp_iterator)->array_data->data_ptr + tmp_iterator->index;
}

/* Следующие три функции создают итератор в памяти и возвращают его дескриптор */
/* Функция, возвращающая итератор, ссылающийся на элемент с указанным индексом */
extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	int tmp_index;
	IteratorT * iterator = NULL;
	if (isHandleInvalid(handle) || 
		(((ArrayDataT *)handle)->logical_size == 0))
		return NULL;
	tmp_index = (index > ((ArrayDataT *)handle)->logical_size) ? 
		((ArrayDataT *)handle)->logical_size : index;
	tmp_index = (index < 0) ? -1 : index;
	iterator = (IteratorT *)createIterator(handle);
	iterator->index = tmp_index;
	return iterator;
}
/* Функция, возвращающая итератор, ссылающийся на первый элемент контейнера */
extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	IteratorT * iterator = NULL;
	if (isHandleInvalid(handle)) 
		return LSQ_HandleInvalid;
	iterator = (IteratorT *)createIterator(handle);
	iterator->index = 0;
	return iterator;
	// return LSQ_GetElementByIndex(handle, 0);
}
/* Функция, возвращающая итератор, ссылающийся на последний элемент контейнера */
extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	return LSQ_GetElementByIndex(handle, ((ArrayDataT *)handle)->logical_size);
}

/* Функция, уничтожающая итератор с заданным дескриптором и освобождающая принадлежащую ему память */
extern void LSQ_DestroyIterator(LSQ_IteratorT iterator)
{
	if (isHandleInvalid(iterator))  
		return;
	free(iterator);
}

/* Функция, перемещающая итератор на один элемент вперед */
extern void LSQ_AdvanceOneElement(LSQ_IteratorT iterator)
{
	LSQ_ShiftPosition(iterator, 1); //лучше так или без испольщования этой функции?
}
/* Функция, перемещающая итератор на один элемент назад */
extern void LSQ_RewindOneElement(LSQ_IteratorT iterator)
{
	if (isHandleInvalid(iterator))
		return;
	LSQ_ShiftPosition(iterator, -1); //аналогично
}
/* Функция, перемещающая итератор на заданное смещение со знаком */
extern void LSQ_ShiftPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT shift)
{
	if (iterator == LSQ_HandleInvalid)
		return;
	((IteratorT *)iterator)->index += shift;
}
/* Функция, устанавливающая итератор на элемент с указанным номером */
extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	if (iterator == LSQ_HandleInvalid)
		return;
	LSQ_ShiftPosition(iterator, pos - ((IteratorT *)iterator)->index);
}

/* Функция, добавляющая элемент в начало контейнера */
extern void LSQ_InsertFrontElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
	insertElementByIndex((ArrayDataT *)handle, element, 0);
}
/* Функция, добавляющая элемент в конец контейнера */
extern void LSQ_InsertRearElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
	int past_rear_index;
	if isHandleInvalid(handle) 
		return;
	if (isContainerFull((ArrayDataT *)handle))
	{
		int tmp_size;
		tmp_size = ((ArrayDataT *)handle)->physical_size + LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize((ArrayDataT *)handle, tmp_size);
	}
	past_rear_index = ((ArrayDataT *)handle)->logical_size++;
	*(((ArrayDataT *)handle)->data_ptr + past_rear_index) = element;
}
/* Функция, добавляющая элемент в контейнер на позицию, указываемую в данный момент итератором. Элемент, на который  *
 * указывает итератор, а также все последующие, сдвигается на одну позицию в конец.                                  */
extern void LSQ_InsertElementBeforeGiven(LSQ_IteratorT iterator, LSQ_BaseTypeT newElement)
{	
	if isHandleInvalid(iterator)
		return;
	insertElementByIndex(((IteratorT *)iterator)->array_data, newElement, ((IteratorT *)iterator)->index);
}

/* Функция, удаляющая первый элемент контейнера */
extern void LSQ_DeleteFrontElement(LSQ_HandleT handle)
{
	deleteElementByIndex((ArrayDataT *)handle, 0);
}
/* Функция, удаляющая последний элемент контейнера */
extern void LSQ_DeleteRearElement(LSQ_HandleT handle)
{
	int tmp_size;
	if isHandleInvalid(handle)
		return;
	((ArrayDataT *)handle)->logical_size--;
	tmp_size = LSQ_BASE_ARRAY_PHYS_SIZE;
	if ((((ArrayDataT *)handle)->physical_size - ((ArrayDataT *)handle)->logical_size) > tmp_size)
	{
		tmp_size = ((ArrayDataT *)handle)->physical_size - LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(((ArrayDataT *)handle), tmp_size);
	}
}
/* Функция, удаляющая элемент контейнера, указываемый заданным итератором. Все последующие элементы смещаются на     *
 * одну позицию в сторону начала.                                                                                    */
extern void LSQ_DeleteGivenElement(LSQ_IteratorT iterator)
{
	deleteElementByIndex(((IteratorT *)iterator)->array_data, ((IteratorT *)iterator)->index);
}
