#include <event.h>
#include <event_list.h>

namespace nano_edr {

    EventList::~EventList(){
        Clear();
    }

    void EventList::PushBack(const Event& event){
        if ((!empty()) && (size() == capacity())){
            PopFront();
        }

        EventNode* new_node = new EventNode(event);

        if (empty()) {
            head_ = new_node;
            tail_ = new_node;
        } else {
            tail_->next = new_node;
            tail_ = new_node;
        }

        size_++;
    }
    
    void EventList::PopFront(){
        if (!empty()){
            EventNode* temp = head_;
            head_ = head_->next;
            delete temp;
            size_--;

            if (empty()) {
                // зануляем указатель, если был 1 элемент
                tail_ = nullptr;
            }
        }
    }

    void EventList::Clear(){
        while (head_ != nullptr){
            EventNode* next = head_->next; 
            delete head_;
            head_ = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

            
}
