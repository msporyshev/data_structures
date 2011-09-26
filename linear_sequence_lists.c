
#include <string.h>
#include <stdlib.h>
#include "linear_sequence.h"

#define isHandleInvalid(handle)(handle == LSQ_HandleInvalid)

typedef struct ListNodeStruct
{
	LSQ_BaseTypeT value;
	struct ListNodeStruct * prev;
	struct ListNodeStruct * next;
} ListNodeT, * ListNodePtrT;

typedef struct 
{
	ListNodePtrT before_first;
	ListNodePtrT past_rear;
	int size;
} ListDataT, * ListDataPtrT;

typedef struct 
{
	ListDataPtrT list_data;
	ListNodePtrT node;
} IteratorT;

static LSQ_IteratorT createIterator(LSQ_HandleT handle, ListNodePtrT node)
{
	IteratorT * iterator = (IteratorT *)malloc(sizeof(IteratorT));
	if(isHandleInvalid(handle)) 
		return LSQ_HandleInvalid;
	if isHandleInvalid(iterator) 
		return LSQ_HandleInvalid;
	iterator->list_data = (ListDataPtrT)handle;
	iterator->node = node;
	return iterator;
}

extern LSQ_HandleT LSQ_CreateSequence(void) 
{
	ListDataPtrT list_data = (ListDataPtrT)malloc(sizeof(ListDataT));
	if isHandleInvalid(list_data)
		return LSQ_HandleInvalid;
	list_data->size = 0;
	list_data->before_first = (ListNodePtrT)malloc(sizeof(ListNodeT));
	if isHandleInvalid(list_data->before_first)
		return LSQ_HandleInvalid;
	list_data->past_rear = (ListNodePtrT)malloc(sizeof(ListNodeT));
	if isHandleInvalid(list_data->before_first)
		return LSQ_HandleInvalid;
	list_data->before_first->next = list_data->past_rear;
	list_data->before_first->prev = NULL;
	list_data->past_rear->prev = list_data->before_first;
	list_data->past_rear->next = NULL;
	return list_data;
}

extern void LSQ_DestroySequence(LSQ_HandleT handle) 
{
	ListDataPtrT list_data = (ListDataPtrT)handle;
	ListNodePtrT tmp_node = NULL;
	if isHandleInvalid(handle)
		return;
	tmp_node = list_data->before_first;
	while (tmp_node->next != NULL)
	{
		tmp_node = tmp_node->next;
		free(tmp_node->prev);
	}
	free(tmp_node);
	free(list_data);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	return (isHandleInvalid(handle)) ? -1 : ((ListDataPtrT)handle)->size;
}

extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	return (!isHandleInvalid(iterator) && 
		(tmp_iterator->node != tmp_iterator->list_data->before_first) &&
		(tmp_iterator->node != tmp_iterator->list_data->past_rear) &&
		(tmp_iterator->node != NULL));
}

extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	return (!isHandleInvalid(iterator) && (tmp_iterator->node == tmp_iterator->list_data->past_rear));
}

extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	return (!isHandleInvalid(iterator) && (tmp_iterator->node == tmp_iterator->list_data->before_first));
}

extern LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	return (!LSQ_IsIteratorDereferencable(iterator)) ? LSQ_HandleInvalid :
		&((tmp_iterator)->node->value);
}

extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	int i;
	ListDataPtrT list_data = (ListDataPtrT)handle;
	IteratorT * tmp_iterator = NULL;
	ListNodePtrT tmp_node = NULL;
	if isHandleInvalid(handle)
		return LSQ_HandleInvalid;
	tmp_node = list_data->before_first;
	if (index >= list_data->size)
		return LSQ_GetPastRearElement(handle);
	for (i = 0; i <= index; i++)
	{
		tmp_node = tmp_node->next;
	}
	tmp_iterator = (IteratorT *)createIterator(handle, tmp_node);
	return tmp_iterator;
}

extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	return LSQ_GetElementByIndex(handle, 0);
}

extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	return isHandleInvalid(handle) ? LSQ_HandleInvalid : 
		createIterator(handle,((ListDataPtrT)handle)->past_rear);
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
	int i;
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	if isHandleInvalid(iterator)
		return;
	i = shift;
	if (shift > 0) 
	{
		while((i != 0) && (!isHandleInvalid(tmp_iterator->node->next)))
		{
			tmp_iterator->node = tmp_iterator->node->next;
			i--;
		}
	}
	else 
	{
		while((i != 0) && (!isHandleInvalid(tmp_iterator->node->prev)))
		{
			tmp_iterator->node = tmp_iterator->node->prev;
			i++;
		}
	}
}

extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	IteratorT * tmp_iterator = (IteratorT *)iterator;
	if isHandleInvalid(iterator)
		return;
	tmp_iterator->node = tmp_iterator->list_data->before_first;
	LSQ_ShiftPosition(iterator, pos + 1);
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
	ListDataPtrT list_data = NULL;
	ListNodePtrT node = NULL;
    if (isHandleInvalid(iterator))
        return;
	node = (ListNodePtrT)malloc(sizeof(ListNodeT));
	if isHandleInvalid(node)
		return;
	node->next = tmp_iterator->node;
	node->prev = tmp_iterator->node->prev;
	node->value = newElement;
	tmp_iterator->node->prev->next = node;
	tmp_iterator->node->prev = node;
	tmp_iterator->node = node;
	tmp_iterator->list_data->size++;
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
    ListNodePtrT cur_node = NULL;
    if (!LSQ_IsIteratorDereferencable(iterator))
        return;
    cur_node = tmp_iterator->node;
	tmp_iterator->list_data->size--;
    tmp_iterator->node->prev->next = tmp_iterator->node->next;
    tmp_iterator->node->next->prev = tmp_iterator->node->prev;
    tmp_iterator->node = cur_node->next;
    free(cur_node);
}