/*
 * fifo.c
 *
 *  Created on: 5 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include "fifo.h"

s8 fifo_put(fifo_t *fifo, void *data, int offset, int count)
{
  if ((FIFO_LENGTH - fifo->count) < count)                      //Проверяем, достаточно ли места
    return -1;                                                  //Нет? Возвращаем "-1" и уходим
  for (int i = offset; i < (offset + count); i++)                //Цикл заполнения
  {
    fifo->data[fifo->tail++] =((u8*) data)[i];                  //Кладем сами данные и сразу tail++
    fifo->count++;                                              //Увеличиваем переменную-счетчик
    if (fifo->tail == FIFO_LENGTH)                              //Если уперлись в границу длины
    {
      fifo->tail=0;                                             //Обнуляем хвост
    }                                                           //Т.е. "сворачиваем" буфер в кольцо
  }
  return 0;                                                     //Возвращаем "ОК"
}

s8 fifo_get(fifo_t *fifo, void *data, int offset, int count)
{
  if (fifo->count < count)                                      //Проверяем, можем ли мы выдать столько,
    return -1;                                                  //сколько у нас просят
  for (int i = offset; i < (offset + count); i++)                //Цикл записи
  {
    ((u8*)data)[i] = fifo->data[fifo->head++];                  //Пишем байт по указанному адресу, head++
    fifo->count--;                                              //Уменьшаем счетчик байт
    if (fifo->head == FIFO_LENGTH)                              //Если уперлись в границу длины
    {
      fifo->head=0;                                             //Обнуляем голову
    }
  }
  return 0;
}

int fifo_count(fifo_t *fifo)
{
  return fifo->count;
}

void fifo_init(fifo_t *fifo)
{
  fifo->head = fifo->tail = fifo->count = 0;
}



