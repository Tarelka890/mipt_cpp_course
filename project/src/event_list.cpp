#include <event.h>
#include <event_list.h>

namespace nano_edr {
    struct EventNode {
        explicit EventNode(const Event& value) : event(value) {}

        Event event;
        EventNode* next = nullptr;
    };

    class EventList {
        public:
            // Вместимость: 0 означает «без ограничения». Так один и тот же список
            // годится и как окно, и как хранилище.
            explicit EventList(std::size_t capacity = 0) : capacity_(capacity) {}

            ~EventList(){
                Clear();
            }

            EventList(const EventList&) = delete;
            EventList& operator=(const EventList&) = delete;

            // Дописывает копию события в конец. Если вместимость задана и список
            // полон, самое старое событие уходит: окно, а не архив. После вызова
            // size() <= capacity() всегда.
            void PushBack(const Event& event){
                if ((!empty()) && (size() == capacity())){
                    PopFront();
                }
                // if (empty()){
                //     // крайний случай: список после этого пуст
                //     head_ = new EventNode(event);
                //     tail_ = head_;
                //     size_++;
                //     return;
                // }
                // tail_->next = new EventNode(event);
                // tail_ = tail_->next;

                EventNode* new_node = new EventNode(event);

                if (empty()) {
                    head_ = new_node;
                    tail_ = new_node;
                } else {
                    tail_->next = new_node;
                    tail_ = new_node;
                }

                size_++;
                // return;
            }

            // Убирает самое старое. На пустом списке ничего не делает и ошибкой
            // не считается: «убрать из пустого» — обычный ход событий.
            void PopFront(){
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

            void Clear(){
                size_ = 0;
                while (head_ != nullptr){
                    EventNode* next = head_->next; 
                    delete head_;
                    head_ = next;
                }
                head_ = nullptr;
            }

            std::size_t size() const { return size_; }
            std::size_t capacity() const { return capacity_; }
            bool empty() const { return size_ == 0; }

            // Голова цепочки для обхода. Указатель на чужой узел: владеет им список,
            // а вызывающий только смотрит. nullptr на пустом списке. Итераторы —
            // лекция 13; до тех пор обход руками.
            const EventNode* head() const { return head_; }

            // Последнее добавленное событие или nullptr. Список односвязный, назад
            // по нему не пройти, поэтому хвост хранится отдельно — иначе за ним
            // пришлось бы каждый раз идти от головы.
            const EventNode* back() const { return tail_; }

        private:
            EventNode* head_ = nullptr;
            EventNode* tail_ = nullptr;
            std::size_t size_ = 0;
            std::size_t capacity_ = 0;
    };
}
