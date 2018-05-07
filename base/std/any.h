#pragma once
#include <typeinfo>
#include <assert.h>
#include <iostream>
#include <type_traits>

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
            Holder(const value_type &value) : _held(value) {}
            Holder(value_type &&value) : _held(static_cast< value_type&& >(value)) {}

            virtual const std::type_info& GetType() { 
                return typeid(_held);
            }
            virtual PlaceHolder *clone() {
                return new Holder(_held);
            }
            value_type _held;
            private:
            Holder & operator=(const Holder &);
        };

    public:
        Any() noexcept : _content(0) {}

        template<typename value_type>
        Any(const value_type & value) : _content(new Holder<typename std::decay<const value_type>::type>(value))
        {
        }
        Any(const Any& other):_content(other._content ? other._content->clone() : nullptr) {}

        template<typename value_type, class = typename std::enable_if<!std::is_same<typename std::decay<value_type>::type, Any>::value, value_type>::type> 
        Any(value_type &&value) : _content(new Holder < typename std::decay<value_type>::type>(static_cast<value_type&&>(value)))
        {
        }

        Any(Any&& other) noexcept : _content(other._content) {
           other._content = 0;
        }

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

        template<typename value_type>
        Any& operator=(value_type&& rhs) {
            Any(static_cast<value_type&&>(rhs)).swap(*this);
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
        return &(static_cast<Any::Holder<value_type>*>(any->_content)->_held);
    }
    std::cerr << "Can't cast " << any->GetType().name() << " to " << typeid(value_type).name() << std::endl;
    //return &(static_cast<Any::Holder<value_type>*>(any->_content)->_held);
    return nullptr;
}

template<typename value_type>
const value_type* any_cast(const Any* any) {
    return any_cast<value_type>(const_cast<Any*>(any));
}

template<typename ValueType>
ValueType any_cast(Any & operand)
{
	typedef typename std::remove_reference<ValueType>::type nonref;
	nonref *result = any_cast<nonref>(&operand);
	assert(result);
	return static_cast<ValueType>(*result);
}

template<typename value_type>
value_type any_cast(const Any& any) {
    typedef typename std::remove_reference<value_type>::type nonref;
    return any_cast<const nonref &>(const_cast<Any &>(any));
}

template<typename ValueType>
inline ValueType&& any_cast(Any&& operand)
{
    static_assert(
            std::is_rvalue_reference<ValueType&&>::value 
            || std::is_const< typename std::remove_reference<ValueType>::type >::value,
            "boost::any_cast shall not be used for getting nonconst references to temporary objects" 
            );
	return any_cast<ValueType&&>(operand);
}
} // std1
