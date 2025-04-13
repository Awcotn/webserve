#ifndef __AWCOTN_SINGLETON_H__
#define __AWCOTN_SINGLETON_H__

#include <memory>

namespace awcotn {

/**
 * @brief 单例模式模板类（值方式）
 * @tparam T 单例类型
 * @tparam X 用于创建多个实例类型的标签类型
 * @tparam N 同一类型T的多个实例的序号
 * 
 * 存在的问题：
 * 1. 未禁用构造函数，外部可以创建实例，破坏单例性质
 * 2. 未禁用拷贝/赋值，可能导致多个实例存在
 * 
 * 安全建议：用户应当将T的构造函数私有化，并声明Singleton为友元
 */
template<class T, class X = void, int N = 0>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
    
private:
    // 禁用构造函数和拷贝/赋值函数，防止外部创建Singleton实例
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

/**
 * @brief 单例模式模板类（指针方式）
 * @tparam T 单例类型
 * @tparam X 用于创建多个实例类型的标签类型
 * @tparam N 同一类型T的多个实例的序号
 * 
 * 存在的问题：
 * 1. 未禁用构造函数，外部可以创建实例，破坏单例性质
 * 2. 未禁用拷贝/赋值，可能导致多个实例存在
 * 
 * 安全建议：用户应当将T的构造函数私有化，并声明SingletonPtr为友元
 */
template<class T, class X = void, int N = 0>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
    
private:
    // 禁用构造函数和拷贝/赋值函数，防止外部创建SingletonPtr实例
    SingletonPtr() = default;
    ~SingletonPtr() = default;
    SingletonPtr(const SingletonPtr&) = delete;
    SingletonPtr& operator=(const SingletonPtr&) = delete;
};

/**
 * @brief 使用示例：
 * 
 * class MyClass {
 * private:
 *     // 构造函数私有化
 *     MyClass() {}
 *     // 禁用拷贝和赋值
 *     MyClass(const MyClass&) = delete;
 *     MyClass& operator=(const MyClass&) = delete;
 *     
 *     // 声明友元，使单例模板可以创建实例
 *     friend class Singleton<MyClass>;
 * };
 * 
 * // 使用
 * MyClass* instance = Singleton<MyClass>::GetInstance();
 */

}

#endif