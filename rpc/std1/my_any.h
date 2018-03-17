#pragma once
#include <typeinfo>
#include <assert.h>
#include <iostream>

namespace std1 {
class Any {
    protected:
        struct PlaceHolder{
            virtual ~PlaceHolder() {}
            virtual const std::type_info& GetType() = 0;
            virtual PlaceHolder *clone() = 0;
        };
        template<typename value_type>
        struct Holder : public PlaceHolder {
            Holder(value_type &&value) : _hold(value) {}
            Holder(const value_type &value) : _hold(value) {}

            virtual const std::type_info& GetType() { 
                return typeid(_hold);
            }
            virtual PlaceHolder *clone() {
                return new Holder(_hold);
            }
            value_type _hold;
        };

    public:
        template<typename value_type>
        Any(value_type &&value):
            _content(new Holder<value_type>(value)) {}
        template<typename value_type>
        Any(const value_type &value):
            _content(new Holder<value_type>(value)) {}

        Any(const Any& other):
            _content(other._content ? other._content->clone() : nullptr) {}
        
        ~Any() { delete _content; }

        Any& swap(Any& rhs) {
            std::swap(_content, rhs._content); // 单纯地交换指针
            return *this;
        }

        template<typename value_type>
        Any& operator=(const value_type& rhs) {
            Any(rhs).swap(*this);
            return *this;
        }

        const std::type_info& GetType() { 
            return _content ? _content->GetType() : typeid(void);
        }

        bool IsEmpty() const {
            return !_content;
        }


    private:
        PlaceHolder *_content;

    template<typename value_type>
    friend value_type* any_cast(Any* any);
};

template<typename value_type>
value_type* any_cast(Any* any) {
    if (!any) {
        std::cerr << "any is null." << std::endl;
    }
    if (any->GetType() == typeid(value_type)) {
        return &(static_cast<Any::Holder<value_type>*>(any->_content)->_hold);
    }
    std::cerr << "Can't cast cast " << any->GetType().name() << " to " << typeid(value_type).name() << std::endl;
    return nullptr;
}

template<typename value_type>
const value_type* any_cast(const Any* any) {
    return any_cast<value_type>(const_cast<Any*>(any));
}

template<typename value_type>
value_type any_cast(const Any& any) {
    const value_type* result = any_cast<value_type>(&any);
    assert(result);
    return *result;
}
} // std1
