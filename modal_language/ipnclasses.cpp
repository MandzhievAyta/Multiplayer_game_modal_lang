#include "lexemes.h"
#include "ipnclasses.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
ListOfVar::~ListOfVar()
{
  Node *tmp;
  while (first) {
    tmp = first->next;
    delete first;
    first = tmp;
  }
  last = NULL;
}

void ListOfVar::Change(const char *varname, int newvalue)
{
  Node **tmp = &first;
  while (*tmp) {
    if (!strcmp((**tmp).name, varname)) {
      (**tmp).value = newvalue;
      return;
    }
    tmp = &((**tmp).next);
  }
  *tmp = new Node;
  last = *tmp;
  last->name = new char[strlen(varname)+1];
  strcpy(last->name, varname);
  last->value = newvalue;
}


int *ListOfVar::Get(const char *varname)
{
  Node **tmp = &first;
  while (*tmp) {
    if (!strcmp((**tmp).name, varname)) {
      return &(**tmp).value;
    }
    tmp = &((**tmp).next);
  }
  return NULL;
}

ListOfIpnItem::~ListOfIpnItem()
{
  IpnItem *tmp;
  while (first) {
    tmp = first->next;
    if (first->p)
      delete first->p;
    delete first;
    first = tmp;
  }
  last = NULL;
}

void ListOfIpnItem::Append(IpnElem *object)
{
  if (!first) {
    first = new IpnItem;
    last = first;
  } else {
    last->next = new IpnItem;
    last = last->next;
  }
  last->p = object;
}

IpnElemStack::~IpnElemStack()
{
  IpnItem *tmp;
  while (vertex) {
    tmp = vertex->next;
    if (vertex->p)
      delete vertex->p;
    delete vertex;
    vertex = tmp;
  }
}

void IpnElemStack::Push(IpnElem *elem)
{
  IpnItem *tmp;
  if (!vertex) {
    vertex = new IpnItem;
  } else {
    tmp = new IpnItem;
    tmp->next = vertex;
    vertex = tmp;
  }
  vertex->p = elem;
}

IpnElem *IpnElemStack::Pop() {
  IpnElem *tmp;
  IpnItem *tmp_item;
  if (!vertex)
    return NULL;
  tmp = vertex->p;
  tmp_item = vertex;
  vertex = vertex->next;
  delete tmp_item;
  return tmp;
}


