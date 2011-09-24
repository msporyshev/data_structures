
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

static int isContainerFull(ArrayDataT * handle)
{
	return (isHandleInvalid(handle)) ? -1 : handle->logical_size == handle->physical_size;	
}

static int isContainerEmpty(ArrayDataT * handle)
{
	return (isHandleInvalid(handle)) ? -1 : handle->logical_size == 0;	
}

static void setContainerSize(ArrayDataT * handle, int size)
{
	if (isHandleInvalid(handle)) 
		return;
	handle->physical_size = size;
	/*if (size <= 0) 
	{
		free(handle->data_ptr);
		return;
	}*/
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

static void insertElementByIndex(ArrayDataT * handle, 
								 LSQ_BaseTypeT element, 
								 LSQ_IntegerIndexT index)
{
	LSQ_IntegerIndexT tmp_index;
	LSQ_BaseTypeT * element_ptr = NULL;

	if isHandleInvalid(handle) 
		return;

	if (isContainerFull(handle)) 
	{
		int tmp_size;
		tmp_size = handle->physical_size + LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(handle, tmp_size);
	}

	handle->logical_size++;
	memmove(handle->data_ptr + index + 1, 
			handle->data_ptr + index, 
			sizeof(LSQ_BaseTypeT) * (handle->logical_size - index - 1));
	element_ptr = handle->data_ptr + index; 
	*(element_ptr) = element;
}

static void deleteElementByIndex(ArrayDataT * handle, LSQ_IntegerIndexT index)
{
	int tmp_size;
	LSQ_IntegerIndexT tmp_index;

	if (isHandleInvalid(handle))
		return;

	tmp_size = LSQ_BASE_ARRAY_PHYS_SIZE;
	handle->logical_size--;
	memmove(handle->data_ptr + index, 
			handle->data_ptr + index + 1, 
			sizeof(LSQ_BaseTypeT) * (handle->logical_size - index));

	if ((handle->physical_size - handle->logical_size) >= tmp_size)
	{
		tmp_size = handle->physical_size - LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(handle, tmp_size);
	}
}

/* Функция, создающая пустой контейнер. Возвращает назначенный ему дескриптор */
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
//	if (!isHandleInvalid(((ArrayDataT *)handle)->data_ptr))
	free(tmp_array->data_ptr);
	free(handle);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	return (isHandleInvalid(handle)) ? -1 : ((ArrayDataT *)handle)->logical_size;
}

/* Функция, определяющая, может ли данный итератор быть разыменован */
extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_DEREFERENCABLE));
}
/* Функция, определяющая, указывает ли данный итератор на элемент, следующий за последним в контейнере */
extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_PASTREAR));
}

/* Функция, определяющая, указывает ли данный итератор на элемент, предшествующий первому в контейнере */
extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	return (!isHandleInvalid(iterator) && (((IteratorT *)iterator)->position_kind == pk_BEFOREFIRST));
}

/* Функция разыменовывающая итератор. Возвращает указатель на элемент, на который ссылается данный итератор */
extern LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = NULL;
	if (!LSQ_IsIteratorDereferencable(iterator)) 
		return LSQ_HandleInvalid;
	tmp_iterator = (IteratorT *)iterator;
	return (tmp_iterator)->array_data->data_ptr + tmp_iterator->index;
}


/* Следующие три функции создают итератор в памяти и возвращают его дескриптор */
/* Функция, возвращающая итератор, ссылающийся на элемент с указанным индексом */
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
/* Функция, возвращающая итератор, ссылающийся на первый элемент контейнера */
extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	return LSQ_GetElementByIndex(handle, 0);
}
/* Функция, возвращающая итератор, ссылающийся на последний элемент контейнера */
extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	return isHandleInvalid(handle) ? LSQ_HandleInvalid : LSQ_GetElementByIndex(handle, ((ArrayDataT *)handle)->logical_size);
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
	LSQ_ShiftPosition(iterator, 1);
}
/* Функция, перемещающая итератор на один элемент назад */
extern void LSQ_RewindOneElement(LSQ_IteratorT iterator)
{
	LSQ_ShiftPosition(iterator, -1);
}
/* Функция, перемещающая итератор на заданное смещение со знаком */
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
/* Функция, устанавливающая итератор на элемент с указанным номером */
extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	if isHandleInvalid(iterator)
		return;
	LSQ_ShiftPosition(iterator, pos - ((IteratorT *)iterator)->index);
}

/* Функция, добавляющая элемент в начало контейнера */
extern void LSQ_InsertFrontElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
    LSQ_IteratorT iterator = LSQ_GetElementByIndex(handle, 0);
    LSQ_InsertElementBeforeGiven(iterator, element);
    LSQ_DestroyIterator(iterator);
}
/* Функция, добавляющая элемент в конец контейнера */
extern void LSQ_InsertRearElement(LSQ_HandleT handle, LSQ_BaseTypeT element)
{
    LSQ_IteratorT iterator = LSQ_GetPastRearElement(handle);
    LSQ_InsertElementBeforeGiven(iterator, element);
    LSQ_DestroyIterator(iterator);
}
/* Функция, добавляющая элемент в контейнер на позицию, указываемую в данный момент итератором. Элемент, на который  *
 * указывает итератор, а также все последующие, сдвигается на одну позицию в конец.                                  */
extern void LSQ_InsertElementBeforeGiven(LSQ_IteratorT iterator, LSQ_BaseTypeT newElement)
{
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	if (isHandleInvalid(iterator))
		return;
	insertElementByIndex(tmp_iterator->array_data, newElement, tmp_iterator->index);
}

/* Функция, удаляющая первый элемент контейнера */
extern void LSQ_DeleteFrontElement(LSQ_HandleT handle)
{
	LSQ_IteratorT iterator = LSQ_GetFrontElement(handle);
	LSQ_DeleteGivenElement(iterator);
	LSQ_DestroyIterator(iterator);
}
/* Функция, удаляющая последний элемент контейнера */
extern void LSQ_DeleteRearElement(LSQ_HandleT handle)
{
	LSQ_IteratorT iterator = LSQ_GetPastRearElement(handle);
	LSQ_RewindOneElement(iterator);
	LSQ_DeleteGivenElement(iterator);
	LSQ_DestroyIterator(iterator);
}
/* Функция, удаляющая элемент контейнера, указываемый заданным итератором. Все последующие элементы смещаются на     *
 * одну позицию в сторону начала.                                                                                    */
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
	if ((tmp_array->physical_size - tmp_array->logical_size) > tmp_size)
	{
		tmp_size = tmp_array->physical_size - LSQ_BASE_ARRAY_PHYS_SIZE;
		setContainerSize(tmp_array, tmp_size);
	}
}