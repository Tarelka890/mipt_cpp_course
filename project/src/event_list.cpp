#include <event.h>
#include <event_list.h>

namespace nano_edr {
    void ListPushBack(EventList* list, const Event* event){
        if ((list->capacity) && (list->size == list->capacity)){
            ListPopFront(list);
        }
        if (!list->size){
            // крайний случай: список после этого пуст
            list->head = new EventNode(*event, nullptr);
            list->tail = list->head;
            list->size++;
            return;
        }
        list->tail->next = new EventNode(*event, nullptr);
        list->tail = list->tail->next;
        list->size++;
        return;
    };

    void ListPopFront(EventList* list){
        if (list->size){
            EventNode* temp = list->head;
            list->head = list->head->next;
            delete temp;
            list->size--;

            if (!list->size) {
                // зануляем указатель, если был 1 элемент
                list->tail = nullptr;
            }
        }
    };

    void ListClear(EventList* list){
        list->size = 0;
        while (list->head != nullptr){
            EventNode* next = list->head->next; 
            delete list->head;
            list->head = next;
        }
        list->tail = nullptr;
    };

    EventList::~EventList(){
        ListClear(this);
    }
}